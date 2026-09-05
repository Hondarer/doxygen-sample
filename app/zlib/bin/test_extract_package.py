#!/usr/bin/env python3
"""公式アーカイブの準備処理を、一時ディレクトリ内で検証する。"""
import concurrent.futures
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import tempfile
import unittest

APP = Path(__file__).resolve().parents[1]
MAKEFW = APP.parents[1] / "framework" / "makefw"


class ExtractPackageTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="zlib-extract-test-")
        self.addCleanup(self.temp.cleanup)
        self.app = Path(self.temp.name)
        shutil.copytree(APP / "packages", self.app / "packages")
        shutil.copytree(APP / "patches", self.app / "patches")

    def run_extract(self):
        return subprocess.run(
            [sys.executable, str(APP / "bin/extract_package.py"),
             "--app-dir", str(self.app), "--makefw-home", str(MAKEFW)],
            capture_output=True, text=True, encoding="utf-8", check=False)

    def assert_success(self):
        result = self.run_extract()
        self.assertEqual(0, result.returncode, result.stderr)
        return result

    def test_initial_extract_and_noop(self):
        self.assert_success()
        header = self.app / "prod/include/zconf.h"
        self.assertIn("ZLIB_API_VISIBILITY", header.read_text())
        license_file = self.app / "prod/libsrc/zlib/LICENSE"
        self.assertTrue(license_file.is_file())
        before = {p: p.stat().st_mtime_ns for p in self.app.rglob("*") if p.is_file()}
        result = self.assert_success()
        self.assertNotIn("展開しています", result.stderr)
        self.assertEqual(before, {p: p.stat().st_mtime_ns for p in before})

    def test_missing_output_is_restored(self):
        self.assert_success()
        source = self.app / "prod/libsrc/zlib/deflate.c"
        expected = source.read_bytes()
        source.unlink()
        self.assert_success()
        self.assertEqual(expected, source.read_bytes())

    def test_missing_package_fails(self):
        (self.app / "packages/zlib-1.3.2.tar.gz").unlink()
        result = self.run_extract()
        self.assertNotEqual(0, result.returncode)
        self.assertIn("curl", result.stderr)
        self.assertFalse((self.app / "make_extract.stamp").exists())

    def test_multiple_packages_choose_newest_version(self):
        shutil.copyfile(self.app / "packages/zlib-1.3.2.tar.gz",
                        self.app / "packages/zlib-1.3.1.tar.gz")
        result = self.assert_success()
        self.assertIn("WARNING", result.stderr)
        self.assertIn("package=zlib-1.3.2.tar.gz", (self.app / "make_extract.stamp").read_text())

    def test_package_content_change_with_preserved_metadata(self):
        self.assert_success()
        package = self.app / "packages/zlib-1.3.2.tar.gz"
        metadata = package.stat()
        data = bytearray(package.read_bytes())
        # gzip ヘッダーの OS フィールドだけを変更し、サイズと mtime を維持する。
        data[9] = (data[9] + 1) % 256
        package.write_bytes(data)
        os.utime(package, ns=(metadata.st_atime_ns, metadata.st_mtime_ns))
        result = self.assert_success()
        self.assertIn("展開しています", result.stderr)

    def test_patch_change_triggers_reapplication(self):
        self.assert_success()
        patch = self.app / "patches/0001-gcc-public-visibility.patch"
        patch.write_text(patch.read_text().replace("ZLIB_API_VISIBILITY", "ZLIB_TEST_VISIBILITY"))
        self.assert_success()
        self.assertIn("ZLIB_TEST_VISIBILITY", (self.app / "prod/include/zconf.h").read_text())

    def test_invalid_archive_fails_without_stamp(self):
        package = self.app / "packages/zlib-1.3.2.tar.gz"
        package.write_bytes(b"not a gzip archive")
        result = self.run_extract()
        self.assertNotEqual(0, result.returncode)
        self.assertIn("ERROR", result.stderr)
        self.assertFalse((self.app / "make_extract.stamp").exists())

    def test_non_regular_member_is_rejected(self):
        package = self.app / "packages/zlib-1.3.2.tar.gz"
        with tarfile.open(package, "w:gz") as archive:
            member = tarfile.TarInfo("zlib-1.3.2/zlib.h")
            member.type = tarfile.SYMTYPE
            member.linkname = "../../outside"
            archive.addfile(member)
        result = self.run_extract()
        self.assertNotEqual(0, result.returncode)
        self.assertFalse((self.app / "make_extract.stamp").exists())

    def test_concurrent_preparation(self):
        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
            results = list(pool.map(lambda _: self.run_extract(), range(4)))
        for result in results:
            self.assertEqual(0, result.returncode, result.stderr)
        self.assert_success()
        header = self.app / "prod/include/zconf.h"
        self.assertEqual(1, header.read_text().count("defined(ZLIB_API_VISIBILITY)"))


if __name__ == "__main__":
    unittest.main()
