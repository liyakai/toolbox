

/*
* 错误码
*/
typedef enum _error_code
{
    ERR_SUCCESS = 0,
    ERR_CREATE_EPOLL_FAILED,    // 创建 epoll 失败
    ERR_MALLOC_FAILED,          // 分配内存失败
    ERR_PARAM_NULLPTR,          // 参数为空指针
    ERR_EPOLL_WAIT_FAILED,      // epoll_wait 返回负值.需要检查 errno错误码判断错误类型
    
} ErrCode;