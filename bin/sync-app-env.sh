#!/bin/bash
#
# app/<name>/**/makepart.mk の OUTPUT_DIR を正本として、実行時のコマンド探索パスと
# ライブラリ探索パスを VS Code と CI の各設定ファイルへ同期する。
#
# app の追加・削除に伴う .vscode / .github / .jenkins の手作業をなくすことが目的で、
# app 側に追加のメタ ファイルは必要としない。
#
# see: app/c-modernization-kit/docs/vscode-variables.md

set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)

find_workspace_root() {
    local dir="$1"

    while [[ "$dir" != "/" ]]; do
        if [[ -f "$dir/.workspaceRoot" ]]; then
            printf '%s\n' "$dir"
            return 0
        fi
        dir=$(dirname -- "$dir")
    done

    return 1
}

usage() {
    cat <<'EOF'
Usage:
  sync-app-env.sh --check
  sync-app-env.sh --write
EOF
}

WORKSPACE_DIR=$(find_workspace_root "$SCRIPT_DIR") || {
    echo "workspace root not found" >&2
    exit 2
}
# Windows では make が cygpath -m 由来の Windows mixed 形式 (D:/...) を返す場合があるため、
# pwd -P 由来の MSYS POSIX 形式との両方で接頭辞判定できるよう mixed 形式も保持する
if command -v cygpath >/dev/null 2>&1; then
    WORKSPACE_DIR_M=$(cygpath -m "$WORKSPACE_DIR")
else
    WORKSPACE_DIR_M=""
fi

APP_DIR="$WORKSPACE_DIR/app"
WARN_FILE="$APP_DIR/app_env.warn"
APP_ORDER_RESOLVER="$WORKSPACE_DIR/framework/makefw/bin/resolve_app_deps.sh"
# --check では「設定差分あり」を warning として扱うため、内部エラーとは別の終了コードを使う
SYNC_WARN_EXIT=3

# pub_markdown.config.yaml の mergeSubfolderDocs に常に付与する app 以外のエントリ
FIXED_MERGE_DOCS=(
    'skills=.agents/skills'
    'docsfw=${DOCSFW_HOME}/docs'
    'doxyfw=${DOXYFW_HOME}/docs'
    'makefw=${MAKEFW_HOME}/docs'
    'testfw=${TESTFW_HOME}/docs'
)

MODE="${1:-}"
case "$MODE" in
    --check|--write)
        ;;
    *)
        usage >&2
        exit 2
        ;;
esac

#
# app/<name> 配下の makepart.mk を評価して OUTPUT_DIR を収集する
#

# app 配下の makepart.mk を、親から子の順 (実ビルドと同じ評価順) で列挙する
list_app_makeparts() {
    local app="$1"

    find "$APP_DIR/$app" -name makepart.mk -print | LC_ALL=C sort
}

# 1 つの app について、その配下の makepart.mk が設定する OUTPUT_DIR をすべて出力する
eval_app_output_dirs() {
    local app="$1"
    local platform="$2"
    local platform_flag
    local target_arch
    local tmp_makefile
    local makepart_path
    local value

    if [[ "$platform" == "Linux" ]]; then
        platform_flag="PLATFORM_LINUX := 1"
        target_arch="linux-sync-x64"
    else
        platform_flag="PLATFORM_WINDOWS := 1"
        target_arch="windows-sync-x64"
    fi

    tmp_makefile=$(mktemp)
    {
        cat <<EOF
WORKSPACE_DIR := $WORKSPACE_DIR
APP_DIR := $APP_DIR
MYAPP_DIR := $APP_DIR/$app
PLATFORM := $platform
$platform_flag
TARGET_ARCH := $target_arch
MAKEFW_SYNC_EVAL := 1
INCDIR :=
LIBSDIR :=
DEFINES :=
MAKEFW_APP_OUTPUT_DIRS :=
EOF
        if [[ -f "$WORKSPACE_DIR/makepart.mk" ]]; then
            cat "$WORKSPACE_DIR/makepart.mk"
            printf '\n'
        fi
        if [[ -f "$APP_DIR/makepart.mk" ]]; then
            cat "$APP_DIR/makepart.mk"
            printf '\n'
        fi

        # makepart.mk ごとに OUTPUT_DIR を空へ戻してから取り込み、設定された値だけを蓄積する。
        # OUTPUT_DIR は := による上書き代入のため、蓄積しないと最後の 1 件しか観測できない。
        while IFS= read -r makepart_path; do
            printf 'OUTPUT_DIR :=\n'
            cat "$makepart_path"
            printf '\n'
            printf 'MAKEFW_APP_OUTPUT_DIRS += $(OUTPUT_DIR)\n'
        done < <(list_app_makeparts "$app")

        cat <<'EOF'
print:
	@printf '%s\n' "$(MAKEFW_APP_OUTPUT_DIRS)"
EOF
    } > "$tmp_makefile"

    if ! value=$(MAKEFLAGS= MFLAGS= make --no-print-directory -f "$tmp_makefile" print); then
        rm -f "$tmp_makefile"
        return 1
    fi

    rm -f "$tmp_makefile"

    printf '%s\n' "$value"
}

# OUTPUT_DIR の絶対パスを app 相対の種別 (prod/cbin または prod/lib) へ写像する
classify_output_dir() {
    local app="$1"
    local path="$2"
    local app_root="$APP_DIR/$app"
    local rel=""

    if [[ "$path" == /* ]]; then
        path="$(realpath -m "$path")"
    fi

    if [[ "$path" == "$app_root"/* ]]; then
        rel="${path#"$app_root"/}"
    elif [[ -n "$WORKSPACE_DIR_M" && "$path" == "$WORKSPACE_DIR_M/app/$app"/* ]]; then
        rel="${path#"$WORKSPACE_DIR_M/app/$app/"}"
    else
        return 0
    fi

    case "$rel" in
        prod/cbin|prod/lib)
            printf '%s\n' "$rel"
            ;;
    esac
}

mapfile -t APPS < <(bash "$APP_ORDER_RESOLVER" --app-order | tr ' ' '\n' | LC_ALL=C sort | grep -v '^$')

CBIN_APPS=()
LIB_APPS=()
DOCS_APPS=()

for app in "${APPS[@]}"; do
    has_cbin=0
    has_lib=0

    for platform in Linux Windows; do
        if ! raw=$(eval_app_output_dirs "$app" "$platform"); then
            printf 'Error: failed to evaluate OUTPUT_DIR for app "%s" (platform: %s)\n' \
                "$app" "$platform" >&2
            exit 1
        fi

        for item in $raw; do
            kind=$(classify_output_dir "$app" "$item")
            case "$kind" in
                prod/cbin) has_cbin=1 ;;
                prod/lib)  has_lib=1 ;;
            esac
        done
    done

    if (( has_cbin )); then
        CBIN_APPS+=("$app")
    fi
    if (( has_lib )); then
        LIB_APPS+=("$app")
    fi
    if [[ -d "$APP_DIR/$app/docs" ]]; then
        DOCS_APPS+=("$app")
    fi
done

#
# 各設定ファイル向けの文字列を組み立てる
#

# 区切り文字で連結する
join_by() {
    local sep="$1"
    shift

    local result=""
    local item

    for item in "$@"; do
        if [[ -z "$result" ]]; then
            result="$item"
        else
            result="$result$sep$item"
        fi
    done

    printf '%s' "$result"
}

# app 名の配列を、指定の接頭辞・区切り・末尾要素からパス文字列へ変換する
build_path_value() {
    local prefix="$1"
    local suffix="$2"
    local sep="$3"
    local tail="$4"
    shift 4

    local -a entries=()
    local app

    for app in "$@"; do
        entries+=("$prefix$app$suffix")
    done
    if [[ -n "$tail" ]]; then
        entries+=("$tail")
    fi

    join_by "$sep" "${entries[@]}"
}

# Windows の PATH は app ごとに lib, cbin の順で並べる
build_windows_entries() {
    local prefix="$1"
    local sep_dir="$2"
    local app

    for app in "${APPS[@]}"; do
        if printf '%s\n' "${LIB_APPS[@]}" | grep -qx -- "$app"; then
            printf '%s%s%sprod%slib\n' "$prefix" "$app" "$sep_dir" "$sep_dir"
        fi
        if printf '%s\n' "${CBIN_APPS[@]}" | grep -qx -- "$app"; then
            printf '%s%s%sprod%scbin\n' "$prefix" "$app" "$sep_dir" "$sep_dir"
        fi
    done
}

VSCODE_LINUX_PATH=$(build_path_value '${workspaceFolder}/app/' '/prod/cbin' ':' '${env:PATH}' "${CBIN_APPS[@]}")
VSCODE_LINUX_LDPATH=$(build_path_value '${workspaceFolder}/app/' '/prod/lib' ':' '' "${LIB_APPS[@]}")

mapfile -t WIN_ENTRIES < <(build_windows_entries '${workspaceFolder}\app\' '\')
WIN_ENTRIES+=('${env:PATH}')
VSCODE_WINDOWS_PATH=$(join_by ';' "${WIN_ENTRIES[@]}")
# settings.json は JSON 文字列のため、バックスラッシュを二重化する
VSCODE_WINDOWS_PATH_JSON="${VSCODE_WINDOWS_PATH//\\/\\\\}"

MERGE_DOCS_ENTRIES=()
for app in "${DOCS_APPS[@]}"; do
    MERGE_DOCS_ENTRIES+=("$app=app/$app/docs")
done
MERGE_DOCS_ENTRIES+=("${FIXED_MERGE_DOCS[@]}")
MERGE_SUBFOLDER_DOCS=$(join_by ' ' "${MERGE_DOCS_ENTRIES[@]}")

CI_LINUX_LDPATH=$(build_path_value '$GITHUB_WORKSPACE/app/' '/prod/lib' ':' '$LD_LIBRARY_PATH' "${LIB_APPS[@]}")
JENKINS_LDPATH=$(build_path_value '/workspace/app/' '/prod/lib' ':' '${LD_LIBRARY_PATH:-}' "${LIB_APPS[@]}")
JENKINS_PATH=$(build_path_value '/workspace/app/' '/prod/cbin' ':' '${PATH}' "${CBIN_APPS[@]}")

#
# ファイル書き換えユーティリティ
#

# 生成済みファイルの一時出力先
TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

# BEGIN / END マーカーで囲まれた区間を、指定ファイルの内容で置き換える
replace_marker_region() {
    local src="$1"
    local dst="$2"
    local begin_marker="$3"
    local end_marker="$4"
    local body_file="$5"

    awk -v begin_marker="$begin_marker" -v end_marker="$end_marker" -v body_file="$body_file" '
        index($0, begin_marker) > 0 {
            print
            while ((getline line < body_file) > 0) {
                print line
            }
            close(body_file)
            in_region = 1
            found = 1
            next
        }
        index($0, end_marker) > 0 {
            in_region = 0
            print
            next
        }
        !in_region { print }
        END {
            if (!found) {
                printf "marker not found: %s\n", begin_marker > "/dev/stderr"
                exit 1
            }
        }
    ' "$src" > "$dst"
}

# 差分を判定し、--write のときだけ書き戻す
apply_file() {
    local target="$1"
    local generated="$2"

    if cmp -s "$target" "$generated"; then
        return 0
    fi

    CHANGED_FILES+=("${target#"$WORKSPACE_DIR"/}")

    if [[ "$MODE" == "--write" ]]; then
        cat "$generated" > "$target"
    fi

    return 0
}

CHANGED_FILES=()

#
# .vscode/.env.linux
#

sync_env_linux() {
    local target="$WORKSPACE_DIR/.vscode/.env.linux"
    local generated="$TMP_DIR/env.linux"

    awk -v path_value="$VSCODE_LINUX_PATH" -v ld_value="$VSCODE_LINUX_LDPATH" '
        /^PATH=/            { printf "PATH=%s\n", path_value; next }
        /^LD_LIBRARY_PATH=/ { printf "LD_LIBRARY_PATH=%s\n", ld_value; next }
        { print }
    ' "$target" > "$generated"

    apply_file "$target" "$generated"
}

#
# .vscode/.env.windows
#

sync_env_windows() {
    local target="$WORKSPACE_DIR/.vscode/.env.windows"
    local generated="$TMP_DIR/env.windows"

    # awk の -v はバックスラッシュをエスケープ シーケンスとして解釈するため、
    # Windows パスの受け渡しには環境変数 (ENVIRON) を使う
    SYNC_WINDOWS_PATH="$VSCODE_WINDOWS_PATH" awk '
        /^PATH=/ { printf "PATH=%s\n", ENVIRON["SYNC_WINDOWS_PATH"]; next }
        { print }
    ' "$target" > "$generated"

    apply_file "$target" "$generated"
}

#
# .vscode/settings.json
#

sync_settings_json() {
    local target="$WORKSPACE_DIR/.vscode/settings.json"
    local generated="$TMP_DIR/settings.json"

    # terminal.integrated.env.linux / .windows のどちらのブロックにいるかを追跡し、
    # 同名キー ("PATH") を取り違えないようにする
    SYNC_LINUX_PATH="$VSCODE_LINUX_PATH" \
    SYNC_LINUX_LDPATH="$VSCODE_LINUX_LDPATH" \
    SYNC_WINDOWS_PATH_JSON="$VSCODE_WINDOWS_PATH_JSON" \
    awk '
        /"terminal\.integrated\.env\.linux"[[:space:]]*:/   { ctx = "linux";   print; next }
        /"terminal\.integrated\.env\.windows"[[:space:]]*:/ { ctx = "windows"; print; next }
        ctx != "" && /^[[:space:]]*}/ { ctx = ""; print; next }
        ctx == "linux" && /^[[:space:]]*"LD_LIBRARY_PATH"[[:space:]]*:/ {
            printf "        \"LD_LIBRARY_PATH\": \"%s\",\n", ENVIRON["SYNC_LINUX_LDPATH"]
            next
        }
        ctx == "linux" && /^[[:space:]]*"PATH"[[:space:]]*:/ {
            printf "        \"PATH\": \"%s\"\n", ENVIRON["SYNC_LINUX_PATH"]
            next
        }
        ctx == "windows" && /^[[:space:]]*"PATH"[[:space:]]*:/ {
            printf "        \"PATH\": \"%s\"\n", ENVIRON["SYNC_WINDOWS_PATH_JSON"]
            next
        }
        { print }
    ' "$target" > "$generated"

    apply_file "$target" "$generated"
}

#
# .vscode/pub_markdown.config.yaml
#

sync_pub_markdown_config() {
    local target="$WORKSPACE_DIR/.vscode/pub_markdown.config.yaml"
    local generated="$TMP_DIR/pub_markdown.config.yaml"

    awk -v value="$MERGE_SUBFOLDER_DOCS" '
        /^mergeSubfolderDocs:/ { printf "mergeSubfolderDocs: %s\n", value; next }
        { print }
    ' "$target" > "$generated"

    apply_file "$target" "$generated"
}

#
# .github/workflows/ci.yml
#

sync_ci_yml() {
    local target="$WORKSPACE_DIR/.github/workflows/ci.yml"
    local generated="$TMP_DIR/ci.yml"
    local stage1="$TMP_DIR/ci.yml.stage1"
    local body_linux="$TMP_DIR/ci-linux.body"
    local body_windows="$TMP_DIR/ci-windows.body"
    local app
    local -a win_lines=()
    local i

    {
        printf '          echo "LD_LIBRARY_PATH=%s" >> $GITHUB_ENV\n' "$CI_LINUX_LDPATH"
        for app in "${CBIN_APPS[@]}"; do
            printf '          echo "$GITHUB_WORKSPACE/app/%s/prod/cbin" >> $GITHUB_PATH\n' "$app"
        done
    } > "$body_linux"

    mapfile -t win_lines < <(build_windows_entries '${{ github.workspace }}\app\' '\')
    {
        printf '          $paths = @(\n'
        for (( i = 0; i < ${#win_lines[@]}; i++ )); do
            if (( i + 1 < ${#win_lines[@]} )); then
                printf '            "%s",\n' "${win_lines[i]}"
            else
                printf '            "%s"\n' "${win_lines[i]}"
            fi
        done
        printf '          )\n'
    } > "$body_windows"

    replace_marker_region "$target" "$stage1" \
        '# BEGIN app-env-sync (linux)' '# END app-env-sync (linux)' "$body_linux"
    replace_marker_region "$stage1" "$generated" \
        '# BEGIN app-env-sync (windows)' '# END app-env-sync (windows)' "$body_windows"

    apply_file "$target" "$generated"
}

#
# .jenkins/inner-build.sh
#

sync_jenkins_inner_build() {
    local target="$WORKSPACE_DIR/.jenkins/inner-build.sh"
    local generated="$TMP_DIR/inner-build.sh"
    local body="$TMP_DIR/inner-build.body"

    {
        printf '# テスト実行時に必要な共有ライブラリ検索パスを設定 (.github/workflows/ci.yml に準拠)\n'
        printf 'export LD_LIBRARY_PATH="%s"\n' "$JENKINS_LDPATH"
        printf '\n'
        printf '# テスト実行時に必要なコマンド検索パスを設定 (.github/workflows/ci.yml に準拠)\n'
        printf 'export PATH="%s"\n' "$JENKINS_PATH"
    } > "$body"

    replace_marker_region "$target" "$generated" \
        '# BEGIN app-env-sync' '# END app-env-sync' "$body"

    apply_file "$target" "$generated"
}

#
# .jenkins/README.md
#

sync_jenkins_readme() {
    local target="$WORKSPACE_DIR/.jenkins/README.md"
    local generated="$TMP_DIR/jenkins-README.md"
    local body="$TMP_DIR/jenkins-README.body"

    {
        printf '```bash\n'
        printf 'export LD_LIBRARY_PATH="%s"\n' "$JENKINS_LDPATH"
        printf '\n'
        printf 'export PATH="%s"\n' "$JENKINS_PATH"
        printf '```\n'
    } > "$body"

    replace_marker_region "$target" "$generated" \
        '<!-- BEGIN app-env-sync -->' '<!-- END app-env-sync -->' "$body"

    apply_file "$target" "$generated"
}

sync_env_linux
sync_env_windows
sync_settings_json
sync_pub_markdown_config
sync_ci_yml
sync_jenkins_inner_build
sync_jenkins_readme

#
# 結果の報告
#

rm -f "$WARN_FILE"

if (( ${#CHANGED_FILES[@]} == 0 )); then
    if [[ "$MODE" == "--check" ]]; then
        printf 'INFO: app env settings are in sync.\n'
    fi
    exit 0
fi

if [[ "$MODE" == "--write" ]]; then
    printf 'INFO: updated app env settings:\n'
    printf '  %s\n' "${CHANGED_FILES[@]}"
    exit 0
fi

{
    printf 'app env settings are out of sync with sync sources.\n'
    printf '  SOURCE : app/*/**/makepart.mk (OUTPUT_DIR), app/*/docs\n'
    printf '  TARGET :\n'
    printf '    %s\n' "${CHANGED_FILES[@]}"
    printf 'Run from workspace root:\n'
    printf '  bash bin/sync-app-env.sh --write\n'
} > "$WARN_FILE"

cat "$WARN_FILE" >&2
exit "$SYNC_WARN_EXIT"
