#pragma once

using ACCEPTEX = LPFN_ACCEPTEX;  // AcceptEx 函数指针,详见MSDN



enum EIOType
{
    IOT_ACCEPT = 1,    // 新连接
    IOT_CONNECT = 2,    // 连接建立
    IOT_RECV = 4,    // 接收
    IOT_SEND = 8,    // 发送
    IOT_CLOSE = 16,   // 关闭
};


