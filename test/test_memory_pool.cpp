#include "tools/memory_pool.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(MemPool)

CASE(TestMemPool1)
{
    //
    ToolBox::MemPoolMgr->DebugPrint();
    char* mem = ToolBox::MemPoolMgr->GetMemory(1);
    ToolBox::MemPoolMgr->DebugPrint();
    ToolBox::MemPoolMgr->GiveBack(mem);
    ToolBox::MemPoolMgr->DebugPrint();

    mem = ToolBox::MemPoolMgr->GetMemory(2 * 1000);
    ToolBox::MemPoolMgr->DebugPrint();
    ToolBox::MemPoolMgr->GiveBack(mem);
    ToolBox::MemPoolMgr->DebugPrint();

    mem = ToolBox::MemPoolMgr->GetMemory(2 * 1000 * 1000);
    ToolBox::MemPoolMgr->DebugPrint();
    ToolBox::MemPoolMgr->GiveBack(mem);
    ToolBox::MemPoolMgr->DebugPrint();

    //mem = MemPoolMgr->GetMemory(2*1000*1000*1000);    // Windows 上测试不通过
    //MemPoolMgr->DebugPrint();
    //MemPoolMgr->GiveBack(mem);
    //MemPoolMgr->DebugPrint();

    mem = ToolBox::MemPoolMgr->GetMemory(2 * 1000 * 1000);
    ToolBox::MemPoolMgr->DebugPrint();
    ToolBox::MemPoolMgr->GiveBack(mem);
    ToolBox::MemPoolMgr->DebugPrint();



}

FIXTURE_END(MemPool)