#include "tools/backtrace.h"
#include "unit_test_frame/unittest.h"
#include <thread>
#ifdef __linux__

FIXTURE_BEGIN(Ftrace)

int TestFunction1(int i)
{
    print_callstack();
    return i;
}

int TestFunction2(int i)
{
    print_callstack();
    return i;
}

int TestFunction3(int i)
{
    print_callstack();
    return i;
}

CASE(TestBacktraceCase1)
{
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1*1; i++)
    {
        TestFunction1(i);
        TestFunction2(i);
        TestFunction3(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    fprintf(stderr, "TestBacktraceCase1 cost: %ld ms\n", duration);
    
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


CASE(TestBacktraceCase2)
{
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1*1; i++)
    {
        TestBacktrace1();
        TestBacktrace2();
        TestBacktrace3();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    fprintf(stderr, "TestBacktraceCase2 cost: %ld ms\n", duration);
}

FIXTURE_END(Ftrace)

#endif // __linux__