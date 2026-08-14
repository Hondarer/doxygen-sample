#include <com_util/base/compiler.h>
#include <testfw.h>
#include <mock_calc.h>

Mock_calc *_mock_calc = nullptr;

Mock_calc::Mock_calc()
{
    ON_CALL(*this, calc_handler(_, _, _, _)).WillByDefault(Return(CALC_OK));

    _mock_calc = this;
}

Mock_calc::~Mock_calc()
{
    _mock_calc = nullptr;
}
