toolbox
---------------------
游戏服务器相关的工具
### 1. 编译
linux/macos 系统:  
    debug版本: `sh build_debug.sh`
    release版本: `sh build_release.sh`

windows 系统:
    在vs2019开始界面选择右侧下面的小字"继续但无需代码",然后选择文件-打开-cmake,选择 Cmakelists.txt
    vs会自动展开cmake项目,可以直接编译运行.

### 2.目前实现的功能
#### 2.1 网络库
[**网络库README及测试数据 -->跳转<--**](./src/network#readme)

    基于TCP+epoll的Linux网络库[reactor模式]
    基于UDP+epoll的Linux网络库[reactor模式]
    基于KCP+epoll的Linux网络库[reactor模式]
    基于TCP+io_uring的Linux网络库[proactor模式]
    基于TCP+iocp的windows网络库[proactor模式]
    基于TCP+kqueue的MacOS网络库[reactor模式]

|        | epoll | io_uring  |  iocp |  kqueue |
|  ----  | ----  |  ----     |  ---- |  ----   |
| TCP    |   √   |     √     |   √   |    √    |
| UDP    |   √   |           |       |         |
| KCP    |   √   |           |       |         |

#### 2.2 基于C++20协程的RPC
[**协程RPC文档 -->跳转<--**](./include/coro_rpc/README.md)
    
#### 2.3 工具
1. [基于五层时间轮算法的定时器](./include/tools/timer.h)
2. [跳表实现](./include/tools/skip_list.h)
3. [C++协程](./include/tools/coroutine.h)
4. [环形缓冲区](./include/tools/ringbuffer.h)
5. [内存池](./include/tools/memory_pool.h)
6. [对象池](./include/tools/object_pool.h)
7. [打印虚函数工具](https://github.com/liyakai/toolbox/blob/include/tools/virtual_print.h)
8. [两种读写锁](./include/tools/rwlock.h)
9. [线程池](./include/tools/thread_pool.h)
10. [LRU](./include/tools/lru_cache.h)
11. [内存泄漏检测函数](./include/tools/debug_new.h)
12. [统计CPU&&内存信息](./include/tools/cpu_mem_info.h)
13. [字符串处理工具](./include/tools/string_util.h)
14. [智能指针](./include/tools/smart_pointer.h)
15. [C++20协程](./include/tools/cpp20_coroutine.h)
16. [纳秒级获取堆栈](./include/tools/backtrace.h)
17. [获取函数名](./include/tools/function_name.h)
18. [获取函数参数](./include/tools/function_traits.h)
### 3. 下一步开发计划
1. ~~linux下的异步io机制:io_uring~~.
2. 基于协程的RPC实现.

   // TODO 扩充工具库
-------------------
路漫漫其修远兮,吾将上下而求索.
