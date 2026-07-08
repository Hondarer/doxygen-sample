/**
 *******************************************************************************
 *  @file           nested_group.h
 *  @brief          Doxygen でネストされたグループを記述する例を提供します。
 *
 *******************************************************************************
 */

/**
 *  @defgroup       NESTED_GROUP_SAMPLE_LEVEL1 LEVEL1
 *  @brief          ネストされたグループの LEVEL1 を表します。
 */

/**
 *  @addtogroup     NESTED_GROUP_SAMPLE_LEVEL1
 *  @{
 */

/**
 *  @brief          LEVEL1 に所属するサンプル定数を表します。
 */
#define NESTED_GROUP_SAMPLE_LEVEL1_CONSTANT (100)

/**
 *  @brief          LEVEL1 に所属するサンプル構造体を表します。
 */
typedef struct NESTED_GROUP_SAMPLE_LEVEL1_STRUCT
{
    int value; /**< LEVEL1 のサンプル値。 */
} NESTED_GROUP_SAMPLE_LEVEL1_STRUCT;

/**
 *  @brief          LEVEL1 に所属するサンプル関数です。
 *  @param[in]      sample LEVEL1 のサンプル構造体を指定します。
 *  @return         成功時は 0、失敗時は 0 以外を返します。
 */
int nested_group_sample_level1_func(const NESTED_GROUP_SAMPLE_LEVEL1_STRUCT *sample);

/** @} */

/**
 *  @defgroup       NESTED_GROUP_SAMPLE_LEVEL2 LEVEL2
 *  @brief          ネストされたグループの LEVEL2 を表します。
 *  @ingroup        NESTED_GROUP_SAMPLE_LEVEL1
 */

/**
 *  @addtogroup     NESTED_GROUP_SAMPLE_LEVEL2
 *  @{
 */

/**
 *  @brief          LEVEL2 に所属するサンプル定数を表します。
 */
#define NESTED_GROUP_SAMPLE_LEVEL2_CONSTANT (200)

/**
 *  @brief          LEVEL2 に所属するサンプル構造体を表します。
 */
typedef struct NESTED_GROUP_SAMPLE_LEVEL2_STRUCT
{
    int value; /**< LEVEL2 のサンプル値。 */
} NESTED_GROUP_SAMPLE_LEVEL2_STRUCT;

/**
 *  @brief          LEVEL2 に所属するサンプル関数です。
 *  @param[in]      sample LEVEL2 のサンプル構造体を指定します。
 *  @return         成功時は 0、失敗時は 0 以外を返します。
 */
int nested_group_sample_level2_func(const NESTED_GROUP_SAMPLE_LEVEL2_STRUCT *sample);

/** @} */

/**
 *  @defgroup       NESTED_GROUP_SAMPLE_LEVEL3 LEVEL3
 *  @brief          ネストされたグループの LEVEL3 を表します。
 *  @ingroup        NESTED_GROUP_SAMPLE_LEVEL2
 *  @{
 */

/**
 *  @brief          ネストされたグループに所属するサンプル定数を表します。
 */
#define NESTED_GROUP_SAMPLE_LEVEL3_CONSTANT (300)

/**
 *  @brief          ネストされたグループに所属するサンプル構造体を表します。
 */
typedef struct NESTED_GROUP_SAMPLE_LEVEL3_STRUCT
{
    int value; /**< サンプル値。 */
} NESTED_GROUP_SAMPLE_LEVEL3_STRUCT;

/**
 *  @brief          ネストされたグループに所属するサンプル関数です。
 *  @param[in]      sample サンプル構造体を指定します。
 *  @return         成功時は 0、失敗時は 0 以外を返します。
 */
int nested_group_sample_level3_func(const NESTED_GROUP_SAMPLE_LEVEL3_STRUCT *sample);

/** @} */
