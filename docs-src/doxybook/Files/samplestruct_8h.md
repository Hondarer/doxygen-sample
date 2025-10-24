---
summary: サンプルの列挙体を定義します。
author: doxygen and doxybook
toc: true
---

<!-- IMPORTANT: This is an AUTOMATICALLY GENERATED file by doxygen and doxybook. Manual edits are NOT allowed. -->

# src/samplestruct.h

サンプルの列挙体を定義します。

## 型

### SampleEnum

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| two | | 2 つめの要素   |
| three | | 3 つめの要素   |
| one | | 1 つめの要素   |

サンプルの列挙体を定義します。

## 構造体

### UserInfo

```cpp
struct UserInfo {
    const char * name;
    int id;
    SampleEnum enumValue;
}
```

ユーザー情報を保持する構造体を定義します。

#### name

```cpp
const char * name;
```

ユーザー名

#### id

```cpp
int id;
```

ユーザーID

#### enumValue

```cpp
SampleEnum enumValue;
```

列挙値
