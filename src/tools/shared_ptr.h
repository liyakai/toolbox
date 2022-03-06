

/*
* @file brief 实现智能指针
* 根据gcc中的智能指针实现进行提取和改编.
*/

namespace ToolBox{

  // Available locking policies:                                        // 可用的加锁策略:
  // _S_single    single-threaded code that doesn't need to be locked.  // 单线程,不需要被锁
  // _S_mutex     multi-threaded code that requires additional support  // 多线程,互斥锁
  //              from gthr.h or abstraction layers in concurrence.h.   // 多线程,原子锁
  // _S_atomic    multi-threaded code using atomic operations.
  enum _Lock_policy { _S_single, _S_mutex, _S_atomic }; 

  // Compile time constant that indicates prefered locking policy in    // 编译时间常数，指示当前配置中的首选锁定策略。
  // the current configuration.
  static const _Lock_policy __default_lock_policy = 
#ifndef __GTHREADS
  _S_single;
#elif defined _GLIBCXX_HAVE_ATOMIC_LOCK_POLICY
  _S_atomic;
#else
  _S_mutex;
#endif

/*
* Define operator[] for shared_ptr<T[]> and shared_ptr<T[N]>.
* 为 hared_ptr<T[]> 和 shared_ptr<T[N]> 定义 operator[]
*/
template<typename Tp>

};  // ToolBox

