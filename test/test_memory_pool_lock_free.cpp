#include "tools/memory_pool_lock_free.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(MemPoolLockFree)

// TODO 增加多线程测试用例

CASE(TestMemPoolLockFree1)
{

    // MemPoolLockFreeMgr->SetDebugPrint(true);
    ToolBox::MemPoolLockFreeMgr->DebugPrint();
    char* mem = ToolBox::MemPoolLockFreeMgr->GetMemory(1);
    ToolBox::MemPoolLockFreeMgr->DebugPrint();
    ToolBox::MemPoolLockFreeMgr->GiveBack(mem);
    ToolBox::MemPoolLockFreeMgr->DebugPrint();

    mem = ToolBox::MemPoolLockFreeMgr->GetMemory(2 * 1000);
    ToolBox::MemPoolLockFreeMgr->DebugPrint();
    ToolBox::MemPoolLockFreeMgr->GiveBack(mem);
    ToolBox::MemPoolLockFreeMgr->DebugPrint();

    mem = ToolBox::MemPoolLockFreeMgr->GetMemory(2 * 1000 * 1000);
    ToolBox::MemPoolLockFreeMgr->DebugPrint();
    ToolBox::MemPoolLockFreeMgr->GiveBack(mem);
    ToolBox::MemPoolLockFreeMgr->DebugPrint();

    //mem = MemPoolLockFreeMgr->GetMemory(2*1000*1000*1000);    // Windows 上测试不通过
    //MemPoolLockFreeMgr->DebugPrint();
    //MemPoolLockFreeMgr->GiveBack(mem);
    //MemPoolLockFreeMgr->DebugPrint();

    mem = ToolBox::MemPoolLockFreeMgr->GetMemory(2 * 1000 * 1000);
    ToolBox::MemPoolLockFreeMgr->DebugPrint();
    ToolBox::MemPoolLockFreeMgr->GiveBack(mem);
    ToolBox::MemPoolLockFreeMgr->DebugPrint();



}

FIXTURE_END(MemPoolLockFree)