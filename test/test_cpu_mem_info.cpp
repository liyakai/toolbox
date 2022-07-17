#include "tools/cpu_mem_info.h"
#include "tools/singleton.h"
#include "unit_test_frame/unittest.h"

#ifdef __linux__

FIXTURE_BEGIN(TestCpuMemInfo)


long long FibonacciSequence(long long N)
{
    if (N < 3)
    {
        return 1;
    }
    return (FibonacciSequence(N - 1) + FibonacciSequence(N - 2));
}


CASE(TestNewNormal)
{



    pid_t pid = getpid();
    printf("pid=%d\n", pid);
    printf("pcpu=%f\n", ToolBox::Singleton<CpuMemInfo>::Instance()->GetProcCpu(pid));

    std::thread thd([&]()
    {
        FibonacciSequence(40);
    });
    thd.join();
    printf("pcpu=%f\n", ToolBox::Singleton<CpuMemInfo>::Instance()->GetProcCpu(pid));
    printf("procmem=%d\n", ToolBox::Singleton<CpuMemInfo>::Instance()->GetProcMem(pid));
    printf("virtualmem=%d\n", ToolBox::Singleton<CpuMemInfo>::Instance()->GetProcVirtualMem(pid));
}

FIXTURE_END(TestCpuMemInfo)

#endif // __linux__