---
name: release-tag
description: Hondarer 配下の対象リポジトリ群へ日付ベースのタグと GitHub Release を一括作成するときに使用します。既定ブランチの HEAD、同名タグ、直近 Release を確認し、重複と不要な作成を防止します。
---

# リリース タグと GitHub Release の一括作成

## 目的

対象リポジトリ群の origin における既定ブランチの HEAD に対して、日付ベースのバージョン タグ  
(`vYYYYMMDD.<major>.<minor>` 形式、例: `v20260817.0.0`) と、同名の GitHub Release を  
一括で作成します。  
直近の GitHub Release がすでに HEAD と同じコミットを指している場合は、そのリポジトリのタグと Release の作成をスキップします。

## 対象リポジトリ

| リポジトリ (Hondarer 配下) | 種別 |
|---|---|
| c-modernization-kit | 統合ワークスペース (superproject) |
| make-framework | framework/makefw の submodule |
| googletest-c-framework | framework/testfw の submodule |
| doxygen-framework | framework/doxyfw の submodule |
| pub_markdown | framework/docsfw の submodule |
| app_c-platform | app/c-platform の submodule |
| app_cjson | app/cjson の submodule |
| app_porter | app/porter の submodule |
| app_sqlite | app/sqlite の submodule |
| app_lua | app/lua の submodule |
| devbin-win | このワークスペースに submodule としては存在しない独立リポジトリ |
| oracle-linux-container | このワークスペースに submodule としては存在しない独立リポジトリ |

対象はすべて GitHub 上のリポジトリ名で直接操作するため、ローカルにサブモジュールとして存在するかどうかは処理に影響しません。

## 前提条件

- `gh` (GitHub CLI) が対象 GitHub アカウントでログイン済みであることを `gh auth status` で確認してください。
- 各リポジトリの既定ブランチは `main` を想定します。  
  異なる場合は、リポジトリごとに `gh api repos/<owner>/<repo> --jq .default_branch` で取得した値を使用してください。

## タグ名の決定

- 通常は実行日から `v<実行日（YYYYMMDD）>.0.0` を組み立てます。
- 同日に再実行する必要が生じた場合は、末尾を `.0.1` のように 1 つずつ増やします。
- タグ名はユーザーに確認してから確定してください。

## 手順

1. 対象リポジトリごとに、既定ブランチの HEAD コミット SHA を取得します。
2. 対象リポジトリごとに、直近の GitHub Release が存在すれば、その Release が指すコミット SHA を `commits/<ref>` API で解決します。  
   Release が存在しない場合はスキップ判定を行わず、常に作成対象とします。
3. 直近 Release のコミット SHA が HEAD の SHA と一致する場合は、そのリポジトリの処理を「スキップ」として記録し、タグと Release を作成しません。
4. 一致しない場合、および直近 Release が存在しない場合は、`gh release create` で軽量タグと GitHub Release を同時に作成します。  
   本文はリポジトリの既存の慣習に合わせて `--generate-notes` による自動生成 (`Full Changelog` 比較リンク) とします。
5. 作成対象については、作成後にタグの参照先 SHA が HEAD の SHA と一致することを検証します。
6. リポジトリごとの判定結果 (作成／スキップ) と Release URL を一覧にして、日本語でユーザーへ報告します。

## 実行スクリプト例

以下は手順 1 から 5 を実施するシェル スクリプトの例です。  
`TAG` は事前にユーザーと確定した値を設定してください。

```bash
OWNER="Hondarer"
TAG="v20260817.0.0"   # 実行のたびに確定した値へ置き換える
REPOS=(
  c-modernization-kit
  make-framework
  googletest-c-framework
  doxygen-framework
  pub_markdown
  app_c-platform
  app_cjson
  app_porter
  app_sqlite
  app_lua
  devbin-win
  oracle-linux-container
)

for repo in "${REPOS[@]}"; do
  echo "=== ${repo} ==="

  # 既存タグの重複確認
  if gh api "repos/${OWNER}/${repo}/git/refs/tags/${TAG}" >/dev/null 2>&1; then
    echo "中断: ${repo} には ${TAG} が既に存在します。処理を止めて内容を確認してください。"
    continue
  fi

  default_branch=$(gh api "repos/${OWNER}/${repo}" --jq '.default_branch')
  head_sha=$(gh api "repos/${OWNER}/${repo}/git/refs/heads/${default_branch}" --jq '.object.sha')

  latest_tag=$(gh api "repos/${OWNER}/${repo}/releases/latest" --jq '.tag_name' 2>/dev/null || true)
  latest_sha=""
  if [ -n "${latest_tag}" ]; then
    latest_sha=$(gh api "repos/${OWNER}/${repo}/commits/${latest_tag}" --jq '.sha' 2>/dev/null || true)
  fi

  if [ -n "${latest_sha}" ] && [ "${latest_sha}" = "${head_sha}" ]; then
    echo "スキップ: ${repo} の直近 Release (${latest_tag}) は HEAD (${head_sha}) と同一コミットです。"
    continue
  fi

  gh release create "${TAG}" \
    --repo "${OWNER}/${repo}" \
    --target "${default_branch}" \
    --title "${TAG}" \
    --generate-notes

  actual_sha=$(gh api "repos/${OWNER}/${repo}/git/refs/tags/${TAG}" --jq '.object.sha')
  if [ "${actual_sha}" != "${head_sha}" ]; then
    echo "警告: ${repo} のタグ参照先 (${actual_sha}) が HEAD (${head_sha}) と一致しません。"
  fi
done
```

## 注意事項

- タグと Release の作成は GitHub 上の共有・公開状態を変更する操作です。  
  実行前に対象リポジトリの一覧、タグ名、スキップ判定の結果をユーザーへ提示し、確認を得てください。
- 対象リポジトリに同名タグがすでに存在する場合は上書きせず、処理を中断してユーザーへ報告してください。
- このワークスペースのローカルなサブモジュールのピン留めコミットは、origin の既定ブランチ HEAD と異なる場合があります。  
  本手順は常に origin の既定ブランチ HEAD を基準とし、ローカルの作業ツリーやサブモジュール状態には影響を与えません。
