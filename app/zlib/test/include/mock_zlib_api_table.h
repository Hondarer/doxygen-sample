/* zlib 1.3.2 の公開関数一覧。宣言・モック・実装・網羅テストで共有する。
 * include guard は意図的に置かない。gzprintf は専用ラッパーで gzvprintf へ渡す。
 */
#ifndef MOCK_ZLIB_RET
    #error MOCK_ZLIB_RET must be defined
#endif
#ifndef MOCK_ZLIB_VOID
    #error MOCK_ZLIB_VOID must be defined
#endif

MOCK_ZLIB_RET(const char *, zlibVersion, (), (), ())
MOCK_ZLIB_RET(int, deflate, (z_streamp strm, int flush), (strm, flush), (_, _))
MOCK_ZLIB_RET(int, deflateEnd, (z_streamp strm), (strm), (_))
MOCK_ZLIB_RET(int, inflate, (z_streamp strm, int flush), (strm, flush), (_, _))
MOCK_ZLIB_RET(int, inflateEnd, (z_streamp strm), (strm), (_))
MOCK_ZLIB_RET(int, deflateSetDictionary, (z_streamp strm, const Bytef *dictionary, uInt dictLength),
              (strm, dictionary, dictLength), (_, _, _))
MOCK_ZLIB_RET(int, deflateGetDictionary, (z_streamp strm, Bytef *dictionary, uInt *dictLength),
              (strm, dictionary, dictLength), (_, _, _))
MOCK_ZLIB_RET(int, deflateCopy, (z_streamp dest, z_streamp source), (dest, source), (_, _))
MOCK_ZLIB_RET(int, deflateReset, (z_streamp strm), (strm), (_))
MOCK_ZLIB_RET(int, deflateParams, (z_streamp strm, int level, int strategy), (strm, level, strategy), (_, _, _))
MOCK_ZLIB_RET(int, deflateTune, (z_streamp strm, int good_length, int max_lazy, int nice_length, int max_chain),
              (strm, good_length, max_lazy, nice_length, max_chain), (_, _, _, _, _))
MOCK_ZLIB_RET(uLong, deflateBound, (z_streamp strm, uLong sourceLen), (strm, sourceLen), (_, _))
MOCK_ZLIB_RET(z_size_t, deflateBound_z, (z_streamp strm, z_size_t sourceLen), (strm, sourceLen), (_, _))
MOCK_ZLIB_RET(int, deflatePending, (z_streamp strm, unsigned *pending, int *bits), (strm, pending, bits), (_, _, _))
MOCK_ZLIB_RET(int, deflateUsed, (z_streamp strm, int *bits), (strm, bits), (_, _))
MOCK_ZLIB_RET(int, deflatePrime, (z_streamp strm, int bits, int value), (strm, bits, value), (_, _, _))
MOCK_ZLIB_RET(int, deflateSetHeader, (z_streamp strm, gz_headerp head), (strm, head), (_, _))
MOCK_ZLIB_RET(int, inflateSetDictionary, (z_streamp strm, const Bytef *dictionary, uInt dictLength),
              (strm, dictionary, dictLength), (_, _, _))
MOCK_ZLIB_RET(int, inflateGetDictionary, (z_streamp strm, Bytef *dictionary, uInt *dictLength),
              (strm, dictionary, dictLength), (_, _, _))
MOCK_ZLIB_RET(int, inflateSync, (z_streamp strm), (strm), (_))
MOCK_ZLIB_RET(int, inflateCopy, (z_streamp dest, z_streamp source), (dest, source), (_, _))
MOCK_ZLIB_RET(int, inflateReset, (z_streamp strm), (strm), (_))
MOCK_ZLIB_RET(int, inflateReset2, (z_streamp strm, int windowBits), (strm, windowBits), (_, _))
MOCK_ZLIB_RET(int, inflatePrime, (z_streamp strm, int bits, int value), (strm, bits, value), (_, _, _))
MOCK_ZLIB_RET(long, inflateMark, (z_streamp strm), (strm), (_))
MOCK_ZLIB_RET(int, inflateGetHeader, (z_streamp strm, gz_headerp head), (strm, head), (_, _))
MOCK_ZLIB_RET(int, inflateBack, (z_streamp strm, in_func in, void *in_desc, out_func out, void *out_desc),
              (strm, in, in_desc, out, out_desc), (_, _, _, _, _))
MOCK_ZLIB_RET(int, inflateBackEnd, (z_streamp strm), (strm), (_))
MOCK_ZLIB_RET(uLong, zlibCompileFlags, (), (), ())
MOCK_ZLIB_RET(int, compress, (Bytef * dest, uLongf *destLen, const Bytef *source, uLong sourceLen),
              (dest, destLen, source, sourceLen), (_, _, _, _))
MOCK_ZLIB_RET(int, compress_z, (Bytef * dest, z_size_t *destLen, const Bytef *source, z_size_t sourceLen),
              (dest, destLen, source, sourceLen), (_, _, _, _))
MOCK_ZLIB_RET(int, compress2, (Bytef * dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level),
              (dest, destLen, source, sourceLen, level), (_, _, _, _, _))
MOCK_ZLIB_RET(int, compress2_z, (Bytef * dest, z_size_t *destLen, const Bytef *source, z_size_t sourceLen, int level),
              (dest, destLen, source, sourceLen, level), (_, _, _, _, _))
MOCK_ZLIB_RET(uLong, compressBound, (uLong sourceLen), (sourceLen), (_))
MOCK_ZLIB_RET(z_size_t, compressBound_z, (z_size_t sourceLen), (sourceLen), (_))
MOCK_ZLIB_RET(int, uncompress, (Bytef * dest, uLongf *destLen, const Bytef *source, uLong sourceLen),
              (dest, destLen, source, sourceLen), (_, _, _, _))
MOCK_ZLIB_RET(int, uncompress_z, (Bytef * dest, z_size_t *destLen, const Bytef *source, z_size_t sourceLen),
              (dest, destLen, source, sourceLen), (_, _, _, _))
MOCK_ZLIB_RET(int, uncompress2, (Bytef * dest, uLongf *destLen, const Bytef *source, uLong *sourceLen),
              (dest, destLen, source, sourceLen), (_, _, _, _))
MOCK_ZLIB_RET(int, uncompress2_z, (Bytef * dest, z_size_t *destLen, const Bytef *source, z_size_t *sourceLen),
              (dest, destLen, source, sourceLen), (_, _, _, _))
MOCK_ZLIB_RET(gzFile, gzdopen, (int fd, const char *mode), (fd, mode), (_, _))
MOCK_ZLIB_RET(int, gzbuffer, (gzFile file, unsigned size), (file, size), (_, _))
MOCK_ZLIB_RET(int, gzsetparams, (gzFile file, int level, int strategy), (file, level, strategy), (_, _, _))
MOCK_ZLIB_RET(int, gzread, (gzFile file, voidp buf, unsigned len), (file, buf, len), (_, _, _))
MOCK_ZLIB_RET(z_size_t, gzfread, (voidp buf, z_size_t size, z_size_t nitems, gzFile file), (buf, size, nitems, file),
              (_, _, _, _))
MOCK_ZLIB_RET(int, gzwrite, (gzFile file, voidpc buf, unsigned len), (file, buf, len), (_, _, _))
MOCK_ZLIB_RET(z_size_t, gzfwrite, (voidpc buf, z_size_t size, z_size_t nitems, gzFile file), (buf, size, nitems, file),
              (_, _, _, _))
MOCK_ZLIB_RET(int, gzputs, (gzFile file, const char *s), (file, s), (_, _))
MOCK_ZLIB_RET(char *, gzgets, (gzFile file, char *buf, int len), (file, buf, len), (_, _, _))
MOCK_ZLIB_RET(int, gzputc, (gzFile file, int c), (file, c), (_, _))
MOCK_ZLIB_RET(int, gzgetc, (gzFile file), (file), (_))
MOCK_ZLIB_RET(int, gzungetc, (int c, gzFile file), (c, file), (_, _))
MOCK_ZLIB_RET(int, gzflush, (gzFile file, int flush), (file, flush), (_, _))
MOCK_ZLIB_RET(int, gzrewind, (gzFile file), (file), (_))
MOCK_ZLIB_RET(int, gzeof, (gzFile file), (file), (_))
MOCK_ZLIB_RET(int, gzdirect, (gzFile file), (file), (_))
MOCK_ZLIB_RET(int, gzclose, (gzFile file), (file), (_))
MOCK_ZLIB_RET(int, gzclose_r, (gzFile file), (file), (_))
MOCK_ZLIB_RET(int, gzclose_w, (gzFile file), (file), (_))
MOCK_ZLIB_RET(const char *, gzerror, (gzFile file, int *errnum), (file, errnum), (_, _))
MOCK_ZLIB_VOID(void, gzclearerr, (gzFile file), (file), (_))
MOCK_ZLIB_RET(uLong, adler32, (uLong adler, const Bytef *buf, uInt len), (adler, buf, len), (_, _, _))
MOCK_ZLIB_RET(uLong, adler32_z, (uLong adler, const Bytef *buf, z_size_t len), (adler, buf, len), (_, _, _))
MOCK_ZLIB_RET(uLong, crc32, (uLong crc, const Bytef *buf, uInt len), (crc, buf, len), (_, _, _))
MOCK_ZLIB_RET(uLong, crc32_z, (uLong crc, const Bytef *buf, z_size_t len), (crc, buf, len), (_, _, _))
MOCK_ZLIB_RET(uLong, crc32_combine_op, (uLong crc1, uLong crc2, uLong op), (crc1, crc2, op), (_, _, _))
MOCK_ZLIB_RET(int, deflateInit_, (z_streamp strm, int level, const char *version, int stream_size),
              (strm, level, version, stream_size), (_, _, _, _))
MOCK_ZLIB_RET(int, inflateInit_, (z_streamp strm, const char *version, int stream_size), (strm, version, stream_size),
              (_, _, _))
MOCK_ZLIB_RET(int, deflateInit2_,
              (z_streamp strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version,
               int stream_size),
              (strm, level, method, windowBits, memLevel, strategy, version, stream_size), (_, _, _, _, _, _, _, _))
MOCK_ZLIB_RET(int, inflateInit2_, (z_streamp strm, int windowBits, const char *version, int stream_size),
              (strm, windowBits, version, stream_size), (_, _, _, _))
MOCK_ZLIB_RET(int, inflateBackInit_,
              (z_streamp strm, int windowBits, unsigned char *window, const char *version, int stream_size),
              (strm, windowBits, window, version, stream_size), (_, _, _, _, _))
MOCK_ZLIB_RET(int, gzgetc_, (gzFile file), (file), (_))
MOCK_ZLIB_RET(gzFile, gzopen64, (const char *arg0, const char *arg1), (arg0, arg1), (_, _))
MOCK_ZLIB_RET(z_off64_t, gzseek64, (gzFile arg0, z_off64_t arg1, int arg2), (arg0, arg1, arg2), (_, _, _))
MOCK_ZLIB_RET(z_off64_t, gztell64, (gzFile arg0), (arg0), (_))
MOCK_ZLIB_RET(z_off64_t, gzoffset64, (gzFile arg0), (arg0), (_))
MOCK_ZLIB_RET(uLong, adler32_combine64, (uLong arg0, uLong arg1, z_off64_t arg2), (arg0, arg1, arg2), (_, _, _))
MOCK_ZLIB_RET(uLong, crc32_combine64, (uLong arg0, uLong arg1, z_off64_t arg2), (arg0, arg1, arg2), (_, _, _))
MOCK_ZLIB_RET(uLong, crc32_combine_gen64, (z_off64_t arg0), (arg0), (_))
MOCK_ZLIB_RET(gzFile, gzopen, (const char *arg0, const char *arg1), (arg0, arg1), (_, _))
MOCK_ZLIB_RET(z_off_t, gzseek, (gzFile arg0, z_off_t arg1, int arg2), (arg0, arg1, arg2), (_, _, _))
MOCK_ZLIB_RET(z_off_t, gztell, (gzFile arg0), (arg0), (_))
MOCK_ZLIB_RET(z_off_t, gzoffset, (gzFile arg0), (arg0), (_))
MOCK_ZLIB_RET(uLong, adler32_combine, (uLong arg0, uLong arg1, z_off_t arg2), (arg0, arg1, arg2), (_, _, _))
MOCK_ZLIB_RET(uLong, crc32_combine, (uLong arg0, uLong arg1, z_off_t arg2), (arg0, arg1, arg2), (_, _, _))
MOCK_ZLIB_RET(uLong, crc32_combine_gen, (z_off_t arg0), (arg0), (_))
MOCK_ZLIB_RET(const char *, zError, (int arg0), (arg0), (_))
MOCK_ZLIB_RET(int, inflateSyncPoint, (z_streamp arg0), (arg0), (_))
MOCK_ZLIB_RET(const z_crc_t *, get_crc_table, (), (), ())
MOCK_ZLIB_RET(int, inflateUndermine, (z_streamp arg0, int arg1), (arg0, arg1), (_, _))
MOCK_ZLIB_RET(int, inflateValidate, (z_streamp arg0, int arg1), (arg0, arg1), (_, _))
MOCK_ZLIB_RET(unsigned long, inflateCodesUsed, (z_streamp arg0), (arg0), (_))
MOCK_ZLIB_RET(int, inflateResetKeep, (z_streamp arg0), (arg0), (_))
MOCK_ZLIB_RET(int, deflateResetKeep, (z_streamp arg0), (arg0), (_))
#ifdef _WIN32
MOCK_ZLIB_RET(gzFile, gzopen_w, (const wchar_t *path, const char *mode), (path, mode), (_, _))
#endif
MOCK_ZLIB_RET(int, gzvprintf, (gzFile file, const char *format, va_list va), (file, format, va), (_, _, _))
