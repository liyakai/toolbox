#include "iocp_ctrl.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "src/network/net_imp/net_imp_define.h"
#include <timer.h>

IocpCtrl::IocpCtrl()
{

}

bool IocpCtrl::CreateIocp()
{
    iocp_fd_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!iocp_fd_)
    {
        return false;
    }
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    return true;
}

void IocpCtrl::Destroy()
{
    CloseHandle(iocp_fd_);
    WSACleanup();
}

#endif // defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
