#include "tools/backtrace.h"
#include "unit_test_frame/unittest.h"
#include <thread>
#ifdef __linux__

FIXTURE_BEGIN(Ftrace)

int TestFunction1()
{
    print_callstack();
    return 0;
}

int TestFunction2()
{
    print_callstack();
    return 0;
}

int TestFunction3()
{
    print_callstack();
    return 0;
}

CASE(TestFtraceCase1)
{
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1*1000; i++)
    {
        TestFunction1();
        TestFunction2();
        TestFunction3();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    fprintf(stderr, "TestFtraceCase1 cost: %lld ms\n", duration);
    
}

int TestBacktrace1()
{
    StackTracer::get_trace();
    return 0;
}

int TestBacktrace2()
{
    StackTracer::get_trace();
    return 0;
}

int TestBacktrace3()
{
    StackTracer::get_trace();
    return 0;
}


CASE(TestFtraceCase2)
{
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1*1000; i++)
    {
        TestBacktrace1();
        TestBacktrace2();
        TestBacktrace3();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    fprintf(stderr, "TestFtraceCase2 cost: %lld ms\n", duration);
}

FIXTURE_END(Ftrace)

#endif // __linux__