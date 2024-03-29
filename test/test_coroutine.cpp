#include "tools/coroutine.h"
#include "unit_test_frame/unittest.h"

#ifdef __linux__

FIXTURE_BEGIN(Coroutine)

struct Args
{
    int n;
};

static void foo(ToolBox::Schedule* sch, void* ud)
{
    if (nullptr == sch)
    {
        return;
    }
    auto* arg = (Args*)ud;
    int start = arg->n;
    for (size_t i = 0; i < 10; i++)
    {
        printf("coroutine %d : %zu\n", sch->CoroutineRunning(), start + i);
        sch->CoroutineYield();
    }
}


CASE(CoroutineCase1)
{
    ToolBox::Schedule sch;
    Args arg1 = { 0 };
    Args arg2 = { 100 };

    int co1 = sch.CoroutineNew(foo, &arg1);
    int co2 = sch.CoroutineNew(foo, &arg2);

    printf("main start\n");
    while (ToolBox::COROUTINE_STATUS::COROUTINE_DEAD != sch.CoroutineStatus(co1) && ToolBox::COROUTINE_STATUS::COROUTINE_DEAD != sch.CoroutineStatus(co2))
    {
        sch.CoroutineResume(co1);
        sch.CoroutineResume(co2);
    }


}

FIXTURE_END(Coroutine)

#endif // __linux__