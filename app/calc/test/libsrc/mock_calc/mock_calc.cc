#include <cplat/base/compiler.h>
#include <testfw.h>
#include <mock_calc.h>

Mock_calc *_mock_calc = nullptr;

Mock_calc::Mock_calc()
{
    ON_CALL(*this, calc_handler(_, _, _, _)).WillByDefault(Return(CALC_OK));

    TESTFW_REGISTER_MOCK_INSTANCE(_mock_calc);
}

Mock_calc::~Mock_calc()
{
    TESTFW_UNREGISTER_MOCK_INSTANCE(_mock_calc);
}
