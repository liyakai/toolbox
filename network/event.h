#pragma once
#include <stdint.h>
#include <string>

/*
* �����¼�
*/
class Event
{
public:
    /*
    * ����
    * @param id �¼�ID
    */
    Event(uint32_t id);
    /*
    * ����
    */
    ~Event();
private:
    uint32_t id_ = 0;   // �¼�ID
};

enum NetEventWorkerType
{
    MainToWorkerNewAccepter = 1,
    MainToWorkerNewConnecter,
    MainToWorkerClose,
    MainToWorkerSend,

    MainToWorkerMax,
};

/*
* ���幤���߳��ڴ�����¼�
*/
class NetEventWorker
{
public:
    /*
    * ����
    * @param type �¼�����
    */
    NetEventWorker(NetEventWorkerType type);
    /*
    * ����
    */
    virtual ~NetEventWorker();
    /*
    * ��ȡ�¼�����
    */
    NetEventWorkerType GetType(){ return type_; };
    /*
    * ���� IP 
    */
    void SetIP(const std::string& ip);
    /*
    * ��ȡ IP
    */
    std::string GetIP() const;
    /*
    * ���� Port 
    */
    void SetPort(const uint16_t port);
    /*
    * ��ȡ Port
    */
    uint16_t GetPort() const;
    /*
    * ���� ����ID
    */
    void SetConnectID(const uint64_t conn_id);
    /*
    * ��ȡ ����ID
    */
    uint64_t GetConnectID() const;
    /*
    * ���� ����
    */
    void SetData(const char* data, uint32_t size);
    /*
    * ��ȡ ����
    */
    const char* GetData() const;
    /*
    * ��ȡ���ݴ�С
    */
    uint32_t GetDataSize() const;


private:
    union Detail
    {
        struct Stream
        {
            uint64_t connect_id_;
            char* data_;
            uint32_t size_;
        } stream_;
        struct Address
        {
            std::string ip_;
            uint16_t port_;
        } address_;
        Detail(){}
        ~Detail(){};
    } detail_;
    NetEventWorkerType type_;   // �¼�����
};

enum NetEvenMainType
{
    WorkerToMainBinded = static_cast<int>(NetEventWorkerType::MainToWorkerMax) + 1,
    WorkerToMainBindFailed,
    WorkerToMainConnected,
    WorkerToMainConnectFailed,
    WorkerToMainAccepted,
    WorkerToMainClose,
    WorkerToMainRecv,

    MainToMainMax,
};

/*
* �������߳��ڴ���������¼�
*/
class NetEventMain
{
public:
    /*
    * ����
    */
    NetEventMain(NetEvenMainType type);
    /*
    * ����
    */
    virtual ~NetEventMain();
    /*
    * ��ȡ�¼�����
    */
    NetEvenMainType GetType(){ return type_; };
    /*
    * ���� ����ID
    */
    void SetConnectID(const uint64_t conn_id);
    /*
    * ��ȡ ����ID
    */
    uint64_t GetConnectID() const;
    /*
    * ���� ����
    */
    void SetData(const char* data, uint32_t size);
    /*
    * ��ȡ ����
    */
    const char* GetData() const;
    /*
    * ��ȡ���ݴ�С
    */
    uint32_t GetDataSize() const;
private:
    NetEvenMainType type_;
    struct Stream
    {
        uint64_t connect_id_;
        char* data_;
        uint32_t size_;
    } stream_;
};