/**
 *******************************************************************************
 *  @file           verbatim.h
 *  @brief          テキストのサンプルを示します。
 *******************************************************************************
 */

 /**
 *  @brief          テキストのサンプルを示します。
 *
 *  一般的なテキストの場合は、`@verbatim ~ @endverbatim` を優先して利用してください。
 *
    @verbatim
    # step

    - step 1

    this is test
        indentation is preserved
    @param is not treated as a command here
    @endverbatim
 */
void verbatim(void);
