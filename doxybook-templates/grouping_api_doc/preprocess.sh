#!/bin/bash

# PlantUML XML変換スクリプト
# 用途: DoxygenのXMLファイル内のPlantUMLタグをMarkdown形式に変換
# 使用方法: ./convert_plantuml_xml.sh /path/to/xml/folder

set -e  # エラーで停止

# 使用方法表示関数
show_usage() {
    echo "PlantUML XML変換スクリプト"
    echo ""
    echo "使用方法:"
    echo "  $0 <XMLフォルダパス>"
    echo ""
    echo "説明:"
    echo "  指定されたフォルダ内のすべてのXMLファイルを対象に"
    echo "  <plantuml>タグを```plantuml\\n@startumlに変換"
    echo "  </plantuml>タグを@enduml\\n```に変換"
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
MODIFIED_COUNT=0

#echo "=== PlantUML XML変換開始 ==="
#echo "対象フォルダ: $XML_FOLDER"
#echo "発見XMLファイル数: $XML_COUNT"
#echo ""

# 各XMLファイルを処理
while IFS= read -r xml_file; do
    PROCESSED_COUNT=$((PROCESSED_COUNT + 1))
    
    #echo -n "[$PROCESSED_COUNT/$XML_COUNT] $(basename "$xml_file")を処理中..."
    
    # PlantUMLタグの存在確認
    if grep -q "<plantuml>" "$xml_file"; then
        # 一時ファイル作成
        TEMP_FILE=$(mktemp)
        
        # 変換処理
        sed -e 's|\s*<plantuml>|**Figure**: \n\n```plantuml\n@startuml|g' \
            -e 's|</plantuml>|@enduml\n```|g' \
            "$xml_file" > "$TEMP_FILE"
        
        # 元ファイルを置換
        mv "$TEMP_FILE" "$xml_file"
        
        MODIFIED_COUNT=$((MODIFIED_COUNT + 1))
        #echo " 変換完了"
    else
        : #echo " PlantUMLタグなし"
    fi
    
done <<< "$XML_FILES"

#echo ""
#echo "=== 変換結果 ==="
#echo "処理ファイル数: $PROCESSED_COUNT"
#echo "変更ファイル数: $MODIFIED_COUNT"
#echo "変換完了"

# 変更されたファイルがある場合の確認メッセージ
#if [ $MODIFIED_COUNT -gt 0 ]; then
#    echo ""
#    echo "注意: $MODIFIED_COUNT 個のXMLファイルが変更されました"
#    echo "doxybook2で処理する前に結果を確認することを推奨します"
#fi
