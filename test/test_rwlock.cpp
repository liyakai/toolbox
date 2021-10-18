#include "src/tools/rwlock.h"
#include "unit_test_frame/unittest.h"

FIXTURE_BEGIN(TestRWLock)

CASE(TestRWLockCase1){
    RWLock rw_lock;
    CRWLock crw_lock;
}



FIXTURE_END(TestRWLock)