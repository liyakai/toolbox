

/*
* ������
*/
typedef enum _error_code
{
    ERR_SUCCESS = 0,
    ERR_MALLOC_FAILED,          // �����ڴ�ʧ��
    ERR_PARAM_NULLPTR,          // ����Ϊ��ָ��
    
} ErrCode;

#define MAX_SOCKET_COUNT			20000