toolbox
---------------------
游戏服务器相关的工具
# 1.目前实现的功能
## 1.1 网络库
    基于TCP+epoll的Linux网络库[reactor模式]
    基于UDP+epoll的Linux网络库[reactor模式]
    基于KCP+epoll的Linux网络库[reactor模式]
    基于TCP+iocp的windows网络库[proactor模式]
    基于TCP+kqueue的MacOS网络库[reactor模式]

[网络库README及测试数据](https://github.com/liyakai/toolbox/tree/main/src/network#readme)
    
## 1.2 工具
    1. 基于五层时间轮算法的定时器
    2. 跳表实现
    3. C++协程
    4. 环形缓冲区
    5. 内存池
    6. 对象池
    7. 打印虚函数工具
    8. 两种读写锁
    9. 线程池
# 2. 下一步开发计划
    扩充工具库
-------------------
路漫漫其修远兮,吾将上下而求索.
