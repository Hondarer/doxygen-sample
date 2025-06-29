#!/bin/bash

# 拡張XML前処理スクリプト
# 用途: DoxygenのXMLファイル内のPlantUMLタグとパラメータdirection属性を変換
# 使用方法: ./preprocess.sh /path/to/xml/folder

set -e  # エラーで停止

# 使用方法表示関数
show_usage() {
    echo "拡張XML前処理スクリプト（PlantUML + Parameter Direction）"
    echo ""
    echo "使用方法:"
    echo "  $0 <XMLフォルダパス>"
    echo ""
    echo "説明:"
    echo "  指定されたフォルダ内のすべてのXMLファイルを対象に以下の変換を実行:"
    echo "  1. <plantuml>タグを```plantuml\\n@startumlに変換"
    echo "  2. </plantuml>タグを@enduml\\n```に変換"
    echo "  3. <parametername direction=\"...\">を[direction] prefixに変換"
    echo ""
    echo "パラメータdirection変換例:"
    echo "  <parametername direction=\"in\">param</parametername>"
    echo "  → <parametername>[in] param</parametername>"
    echo ""
    echo "例:"
    echo "  $0 ./doxygen/xml"
    echo "  $0 /home/user/project/xml"
    echo ""
    echo "注意:"
    echo "  - XMLファイルは直接編集されます（バックアップ推奨）"
    echo "  - .xmlまたは.XMLファイルのみが対象です"
}

# 引数チェック
if [ $# -ne 1 ]; then
    echo "エラー: 引数が正しくありません"
    echo ""
    show_usage
    exit 1
fi

XML_FOLDER="$1"

# フォルダ存在チェック
if [ ! -d "$XML_FOLDER" ]; then
    echo "エラー: 指定されたフォルダが見つかりません: $XML_FOLDER"
    exit 1
fi

# XMLファイル検索
XML_FILES=$(find "$XML_FOLDER" -type f \( -name "*.xml" -o -name "*.XML" \) 2>/dev/null)

if [ -z "$XML_FILES" ]; then
    echo "警告: 指定されたフォルダにXMLファイルが見つかりません: $XML_FOLDER"
    exit 0
fi

# XMLファイル数をカウント
XML_COUNT=$(echo "$XML_FILES" | wc -l)
PROCESSED_COUNT=0
PLANTUML_MODIFIED=0
DIRECTION_MODIFIED=0

#echo "=== XML前処理開始 ==="
#echo "対象フォルダ: $XML_FOLDER"
#echo "発見XMLファイル数: $XML_COUNT"
#echo ""

# 各XMLファイルを処理
while IFS= read -r xml_file; do
    PROCESSED_COUNT=$((PROCESSED_COUNT + 1))
    
    #echo -n "[$PROCESSED_COUNT/$XML_COUNT] $(basename "$xml_file")を処理中..."
    
    # 処理が必要かチェック
    PLANTUML_FOUND=$(grep -c "<plantuml>" "$xml_file" || true)
    DIRECTION_FOUND=$(grep -c 'direction=' "$xml_file" || true)
    
    if [ "$PLANTUML_FOUND" -gt 0 ] || [ "$DIRECTION_FOUND" -gt 0 ]; then
        # 一時ファイル作成
        TEMP_FILE=$(mktemp)
        
        # Step 1: PlantUML変換
        sed -e 's|\s*<plantuml>|**Figure**: \n\n```plantuml\n@startuml|g' \
            -e 's|</plantuml>|@enduml\n```|g' \
            "$xml_file" > "$TEMP_FILE"
        
        # Step 2: パラメータdirection変換
        # direction="in" の場合
        sed -i 's|<parametername direction="in">\([^<]*\)</parametername>|<parametername>[in] \1</parametername>|g' "$TEMP_FILE"
        
        # direction="out" の場合
        sed -i 's|<parametername direction="out">\([^<]*\)</parametername>|<parametername>[out] \1</parametername>|g' "$TEMP_FILE"
        
        # direction="in,out" や "in, out" の場合
        sed -i 's|<parametername direction="in,out">\([^<]*\)</parametername>|<parametername>[in,out] \1</parametername>|g' "$TEMP_FILE"
        sed -i 's|<parametername direction="in, out">\([^<]*\)</parametername>|<parametername>[in,out] \1</parametername>|g' "$TEMP_FILE"
        
        # direction="inout" の場合（一部のDoxygenバージョンで使用される）
        sed -i 's|<parametername direction="inout">\([^<]*\)</parametername>|<parametername>[inout] \1</parametername>|g' "$TEMP_FILE"
        
        # 元ファイルを置換
        mv "$TEMP_FILE" "$xml_file"
        
        if [ "$PLANTUML_FOUND" -gt 0 ]; then
            PLANTUML_MODIFIED=$((PLANTUML_MODIFIED + 1))
        fi
        if [ "$DIRECTION_FOUND" -gt 0 ]; then
            DIRECTION_MODIFIED=$((DIRECTION_MODIFIED + 1))
        fi
        
        #echo " 変換完了 (PlantUML:$PLANTUML_FOUND, Direction:$DIRECTION_FOUND)"
    else
        : #echo " 変換不要"
    fi
    
done <<< "$XML_FILES"

#echo ""
#echo "=== 変換結果 ==="
#echo "処理ファイル数: $PROCESSED_COUNT"
#echo "PlantUML変換ファイル数: $PLANTUML_MODIFIED"
#echo "Direction変換ファイル数: $DIRECTION_MODIFIED"
#echo "前処理完了"

# 変更されたファイルがある場合の確認メッセージ
#TOTAL_MODIFIED=$((PLANTUML_MODIFIED + DIRECTION_MODIFIED))
#if [ $TOTAL_MODIFIED -gt 0 ]; then
#    echo ""
#    echo "doxybook2で処理する準備が整いました"
#    echo "確認: grep -r \"\\[in\\]\\|\\[out\\]\" $XML_FOLDER/ | head -5"
#fi
