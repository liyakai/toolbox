

/*
* ������
*/
typedef enum _error_code
{
    ERR_SUCCESS = 0,
    ERR_CREATE_EPOLL_FAILED,    // ���� epoll ʧ��
    ERR_MALLOC_FAILED,          // �����ڴ�ʧ��
    ERR_PARAM_NULLPTR,          // ����Ϊ��ָ��
    ERR_EPOLL_WAIT_FAILED,      // epoll_wait ���ظ�ֵ.��Ҫ��� errno�������жϴ�������
    
} ErrCode;