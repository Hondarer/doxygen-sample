#include <testfw.h>
#include <mock_calc.h>

MOCK_WEAK_IMPL(int, calc_handler, int kind, int a, int b, int *result)
{
    int mock_ret = 0;

    if (_mock_calc != nullptr)
    {
        mock_ret = _mock_calc->calc_handler(kind, a, b, result);
    }

    if (getTraceLevel() > TRACE_NONE)
    {
        printf("  > %s %d, %d, %d, 0x%p", __func__, kind, a, b, (void *)result);
        if (getTraceLevel() >= TRACE_DETAIL)
        {
            printf(" -> %d, %d\n", *result, mock_ret);
        }
        else
        {
            printf("\n");
        }
    }

    return mock_ret;
}
