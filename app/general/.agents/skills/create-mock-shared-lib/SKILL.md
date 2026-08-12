---
name: create-mock-shared-lib
description: |
  lua / sqlite / cjson のように、第三者の共有ライブラリを
  API 表からモックするときに使うスキルです。
  real delegate、Windows の dllimport 解除、実ライブラリとの同時リンク禁止をまとめます。
when_to_use: |
  - app/lua、app/sqlite、app/cjson の mock を追加・更新するとき
  - 同じ方針の第三者共有ライブラリ mock を新設するとき
  - これらの mock を利用する app 単体テストの makepart.mk を書くとき
---

# 第三者共有ライブラリ向け mock

このスキルは `app/lua`、`app/sqlite`、`app/cjson` の mock に適用します。  
一般の app mock (`create-mock` の失敗値既定) よりこちらを優先します。  
`mock_com_util` は関数単位の手書き実装なので `create-mock-com-mock` を使います。方針 (既定で実関数へ委譲) は同じです。

## 方針

- 公開関数は API 表 (`mock_<lib>_api_table.h`) から宣言、`MOCK_METHOD`、`delegate_real_`、ラッパー、`ON_CALL` を生成します。
- 未注入時と mock 注入時の既定動作は `Invoke(delegate_real_<func>)` です。
- 本物の呼び出しは `resolveSharedSymbolOrExit()` です。失敗時は `stderr` に理由を出し `exit(1)` します。
- `delegate_fake_` と real / fake 切替 API は作りません。
- `include_override` は使いません。利用側は本物のヘッダーを include し、リンク先を `mock_<lib>` に替えます。
- Linux は `MOCK_WEAK_IMPL`、Windows は `extern "C"` の強シンボルです。実ソースを `TEST_SRCS` に載せない前提です。

## 利用側の必須設定

`makepart.mk` では実ライブラリの代わりに mock だけをリンクします。両方をリンクすると、Linux では実 `.so` の強シンボルが弱定義のモックを上書きします。

| ライブラリ | Windows の define | `LIBS` |
|---|---|---|
| lua | `LUA_CORE` | `mock_lua` |
| sqlite | `SQLITE_API=` | `mock_sqlite3` |
| cjson | `CJSON_HIDE_SYMBOLS` | `mock_cjson` |

Windows の define を付けないとヘッダーが `dllimport` になり、モックへ届きません。

完全隔離では、SUT が呼ぶ関数をすべて `EXPECT_CALL` / `ON_CALL` でスタブします。  
スタブ漏れは実関数へ委譲されます。偽ハンドルを実関数へ渡すと失敗します。

出力引数を持つ生成系 API (`sqlite3_open` など) を成功させるときは `SetArgPointee` でハンドルを埋めます。

## API 表

- include guard は置きません。`MOCK_<LIB>_RET` / `MOCK_<LIB>_VOID` 未定義なら `#error` します。
- 可変長引数関数は表に入れず、専用 `.cc` で `va_list` 版の `MOCK_METHOD` にします。
- 公開関数の追加漏れは `exportTest` で検出します。接頭辞フィルターを使い、関数でない公開変数だけを除外します。

## 単体隔離の限界

次は関数モックを通りません。

- 公開構造体のフィールド直アクセス (`cJSON` の `valuestring` / `child` / `next`)
- フィールドを読むマクロ (`cJSON_ArrayForEach`、`cJSON_SetIntValue`、`luaL_addchar`、`lua_getextraspace`)
- ヘッダー マクロの展開先を知らずにマクロ名へ `EXPECT_CALL` すること (`lua_pop` は `lua_settop`)
- sqlite の公開変数 (`sqlite3_version` など)。値の差し替えはできません。`mock_sqlite3` が定義するのでリンクはできます。
- `sqlite3ext.h` の拡張 API 表

偽ポインターを返すときは、SUT がこれらの経路を使わないこと、またはテスト側で実レイアウトのオブジェクトを組むことを確認します。

## 実装箇所

- 宣言と Mock クラス: `app/<name>/test/include/mock_<lib>.h`
- API 表: `app/<name>/test/include/mock_<lib>_api_table.h`
- 生成ラッパーと `ON_CALL`: `app/<name>/test/libsrc/mock_<lib>/mock_<lib>.cc`
- 可変長引数: `mock_<lib>_variadic.cc`
- 利用手順: 各 app の `README.md`

## 確認項目

- API 表、`MOCK_METHOD`、`delegate_real_`、`ON_CALL` が同じ一覧から生成されていること
- Windows の define が mock 自身の `makepart.mk`、利用側の例、README で一致していること
- 利用側が実ライブラリを同時リンクしていないこと
- `exportTest` が接頭辞フィルターで余剰エクスポートを検出すること
- トレースが関数名を出し、`TRACE_DETAIL` で戻り値を出すこと
- 生成マクロ内の実関数ポインター名が `real_fn` であること
