
/**
 *******************************************************************************
 *  @file           samplestruct.h
 *  @brief          サンプルの列挙体を定義します。
 *******************************************************************************
 */

 /**
 *  @ingroup        public_api
 *  @brief          サンプルの列挙体を定義します。
 */
enum SampleEnum
{
    one,  /*!< 1 つめの要素 */
    two,  /*!< 2 つめの要素 */
    three /*!< 3 つめの要素 */
};

/**
 *  @ingroup        public_api
 *  @brief          ユーザー情報を保持する構造体を定義します。
 */
typedef struct
{
    int id;               /*!< ユーザーID */
    const char *name;     /*!< ユーザー名 */
    SampleEnum enumValue; /*!< 列挙値 */
} UserInfo;
