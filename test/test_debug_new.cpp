#include "src/tools/debug_new.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(TestDebugNew)

CASE(TestNewNormal){
    int* single = NEW int;
    delete single;
    CHECK_LEAKS
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
}

FIXTURE_END(TestDebugNew)



