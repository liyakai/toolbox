#pragma once

using ACCEPTEX = LPFN_ACCEPTEX;  // AcceptEx ����ָ��,���MSDN



enum EIOType
{
    IOT_ACCEPT = 1,    // ������
    IOT_CONNECT = 2,    // ���ӽ���
    IOT_RECV = 4,    // ����
    IOT_SEND = 8,    // ����
    IOT_CLOSE = 16,   // �ر�
};


