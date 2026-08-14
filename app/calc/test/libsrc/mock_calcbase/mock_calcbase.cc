#include <com_util/base/compiler.h>
#include <testfw.h>
#include <mock_calcbase.h>

Mock_calcbase *_mock_calcbase = nullptr;

Mock_calcbase::Mock_calcbase()
{
    ON_CALL(*this, calcbase_add(_, _, _))
        .WillByDefault(Invoke(
            [](int a, int b, int *result)
            {
                *result = a + b;
                return CALC_OK;
            })); // モックの既定の挙動を定義する例
    ON_CALL(*this, calcbase_subtract(_, _, _))
        .WillByDefault(Return(
            CALC_OK)); // 一般的にはモックの既定の挙動は NOP にしておき、テスト プログラムで具体的な挙動を決める
    ON_CALL(*this, calcbase_multiply(_, _, _)).WillByDefault(Return(CALC_OK));
    ON_CALL(*this, calcbase_divide(_, _, _)).WillByDefault(Return(CALC_OK));

    _mock_calcbase = this;
}

Mock_calcbase::~Mock_calcbase()
{
    _mock_calcbase = nullptr;
}
