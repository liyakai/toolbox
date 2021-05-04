#include "tools/memory_pool.h"
#include "tools/unit_test_frame/unittest.h"

FIXTURE_BEGIN(MemPool)

CASE(TestMemPool1){
    //
    MemPoolMgr->DebugPrint();
    char* mem = MemPoolMgr->GetMemory(1);
    MemPoolMgr->DebugPrint();
    MemPoolMgr->GiveBack(mem);
    MemPoolMgr->DebugPrint();

    mem = MemPoolMgr->GetMemory(2*1000);
    MemPoolMgr->DebugPrint();
    MemPoolMgr->GiveBack(mem);
    MemPoolMgr->DebugPrint();

    mem = MemPoolMgr->GetMemory(2 * 1000*1000);
    MemPoolMgr->DebugPrint();
    MemPoolMgr->GiveBack(mem);
    MemPoolMgr->DebugPrint();

    //mem = MemPoolMgr->GetMemory(2*1000*1000*1000);    // Windows 上测试不通过
    //MemPoolMgr->DebugPrint();
    //MemPoolMgr->GiveBack(mem);
    //MemPoolMgr->DebugPrint();

    mem = MemPoolMgr->GetMemory(2 * 1000 * 1000);
    MemPoolMgr->DebugPrint();
    MemPoolMgr->GiveBack(mem);
    MemPoolMgr->DebugPrint();



}

FIXTURE_END(MemPool)