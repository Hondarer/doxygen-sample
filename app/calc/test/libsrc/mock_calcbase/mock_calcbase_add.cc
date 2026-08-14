#include <testfw.h>
#include <mock_calcbase.h>

MOCK_WEAK_IMPL(int, calcbase_add, int a, int b, int *result)
{
    int ret;

    ret = 0;
    if (_mock_calcbase != nullptr)
    {
        ret = _mock_calcbase->calcbase_add(a, b, result);
    }

    if (getTraceLevel() > TRACE_NONE)
    {
        printf("  > %s %d, %d, 0x%p", __func__, a, b, (void *)result);
        if (getTraceLevel() >= TRACE_DETAIL)
        {
            printf(" -> %d, %d\n", *result, ret);
        }
        else
        {
            printf("\n");
        }
    }

    return ret;
}
