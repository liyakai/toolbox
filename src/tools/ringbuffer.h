#pragma once
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <atomic>
#include <array>
#include "debug_print.h"
/*
* 非线程安全的环形队列
* Type 数据类型
* Size 队列长度
* Ratio 警戒值,超过此容量,队列长度会自动增长一倍
*/
template <typename Type, std::size_t Size, std::size_t Ratio = 75>
class RingBuffer : public DebugPrint
{
public:
    /*
    * 构造函数
    */
    RingBuffer()
        :buffer_size_(RebuildNum(Size)),ratio_((double)Ratio/double(100.0))
    {
        buffer_ = new char[buffer_size_];
    }
    /*
    * 析构
    */
    ~RingBuffer()
    {
        if(nullptr != buffer_)
        {
            delete[] buffer_;
            buffer_ = nullptr;
        }
        
    }

    /*
    * 清理缓冲区
    */
    void Clear()
    {
        write_pos_ = 0;
        read_pos_ = 0;
    }
    /*
    * 判断是否需要扩容
    */
    bool NeedEnlage(std::size_t new_len = 0)
    {
        auto ratio = (double)(buffer_size_ - WriteableSize()) / (double)buffer_size_;
        return ratio > ratio_ || WriteableSize() < new_len;
    }
    /*
    * 可写数据长度[使用一个元素空间来判满]
    */
    std::size_t WriteableSize()
    {
        return (read_pos_ - write_pos_ - 1) % buffer_size_;
    }
    /*
    * 连续可写数据长度
    */
    std::size_t ContinuouslyWriteableSize()
    {
        while(NeedEnlage())
        {
            if(!Enlage())
            {
                return 0;
            }
        }
        return read_pos_ >= write_pos_ + 1 ? read_pos_ - write_pos_ - 1 : (std::min)(WriteableSize(), buffer_size_ - write_pos_);
    }
    /*
    * 调整写位置
    */
    void AdjustWritePos(std::size_t size)
    {
        write_pos_ = (write_pos_ + size) % buffer_size_;
    }
    /*
    * 获取可写位置指针
    */
    char* GetWritePtr()
    {
        return buffer_ + write_pos_;
    }
    /*
    * 连续可读数据长度
    */
    std::size_t ContinuouslyReadableSize()
    {
        return write_pos_ >= read_pos_ ? write_pos_ - read_pos_ : (std::min)(ReadableSize(), buffer_size_ - read_pos_);
    }
    /*
    * 获取可读位置指针
    */
    char* GetReadPtr()
    {
        return buffer_ + read_pos_;
    }
    /*
    * 调整读位置
    */
    void AdjustReadPos(std::size_t size)
    {
        read_pos_ = (read_pos_ + size) % buffer_size_;
    }
    /*
    * 可读数据长度
    */
    std::size_t ReadableSize()
    {
        return (write_pos_ - read_pos_) % buffer_size_;
    }
    /*
    * 获取 buffer size
    */
    std::size_t GetBufferSize(){ return buffer_size_; }
    /*
    * 判空
    */
    bool Empty()
    {
        return ReadableSize() == 0;
    }

    /*
    * 判满
    */
    bool Full()
    {
        return WriteableSize() == 0;
    }

    /*
    * 写入
    * @param buffer 读取数组的指针
    * @param len 读取数组的长度
    */
    std::size_t Write(const char* buffer, std::size_t len)
    {
        while(NeedEnlage(len))
        {
            if(!Enlage())
            {
                return 0;
            }
        }
        len = (std::min)(len, WriteableSize());
        auto rbytes = (std::min)(len, buffer_size_ - write_pos_);
        memmove(buffer_ + (write_pos_ & (buffer_size_ - 1)), buffer, rbytes);
        memmove(buffer_, buffer + rbytes, len - rbytes);
        write_pos_ = (write_pos_ + len) % buffer_size_;
        return len;
    }
    /*
    * 读取
    * @param buffer 读取数组的指针
    * @param len 读取数组的长度
    */
    std::size_t Read(char* buffer, std::size_t len)
    {
        len = (std::min)(len, ReadableSize());
        auto rbytes = (std::min)(len, buffer_size_ - read_pos_);
        memmove(buffer, buffer_ + read_pos_, rbytes);
        memmove(buffer + rbytes, buffer_, len - rbytes);
        read_pos_ = (read_pos_ + len) % buffer_size_;
        return len;
    }
    /*
    * 读取而不移动读游标
    * @param buffer 拷贝的内存指针
    * @param len  拷贝的长度
    */
    std::size_t Copy(char* buffer, std::size_t len)
    {
        len = (std::min)(len, ReadableSize());
        auto rbytes = (std::min)(len, buffer_size_ - read_pos_);
        memmove(buffer, buffer_ + read_pos_, rbytes);
        memmove(buffer + rbytes, buffer_, len - rbytes);
        return len;
    }
    /*
    * 擦除
    * @param len 擦除的长度
    */
    std::size_t Remove(std::size_t len)
    {
        len = (std::min)(len, ReadableSize());
        read_pos_ = (read_pos_ + len) % buffer_size_;
        return len;
    }
    /*
    * 缓冲区扩容一倍
    */
    bool Enlage()
    {
        if(buffer_size_ > SIZE_MAX/ 2)
        {
            return false;
        }
        auto size = ReadableSize();
        auto new_size = buffer_size_ * 2;
        auto buffer = new char[new_size];
        Read(buffer, size);
        buffer_size_ = new_size;
        read_pos_ = 0;
        write_pos_ = size;
        delete[] buffer_;
        buffer_ = buffer;
        return true;
    }
    /*
    * 按类型读取
    * @param 读取的类型
    */
    template<typename OType>
    bool Read(OType& type)
    {
        if(ReadableSize() < sizeof(OType))
        {
            return false;
        }
        Read((char*)(&type), sizeof(OType));
        return true;
    }
    /*
    * 按类型写
    * @param type 写入的类型
    */
    template<typename OType>
    bool Write(const OType& type)
    {
        if(WriteableSize() < sizeof(OType))
        {
            return false;
        }
        Write((const char*)(&type), sizeof(OType));
        return true;
    }
    /*
    * 按类型拷贝
    * @param type 拷贝的类型
    */
    template<typename OType>
    bool Copy(OType& type)
    {
        if(ReadableSize() < sizeof(OType))
        {
            return false;
        }
        Copy((char*)(&type), sizeof(OType));
        return true;
    }
    /*
    * 测试打印
    */
    void DebugPrint(bool force = false, bool detail = true)
    {
        bool old_debug_status = GetDebugStatus();
        if(false == force && false == old_debug_status)
        {
            return;
        }
        SetDebugPrint(true);
        Print("\n ========== RingBuffer ========== \n");
        Print("缓冲区大小:%zu,读位置:%zu,写位置:%zu,警戒值:%f\n", buffer_size_, read_pos_, write_pos_, ratio_);
        for(size_t i = 0; true == detail && i < buffer_size_; i++)
        {
            // if(0 != i && 0 == (i % 10))
            // {
            //     printf("\n");
            // }
            Print("0x%02X ",buffer_[i]);
        }
        Print("\n ---------- RingBuffer ---------- \n");
        SetDebugPrint(old_debug_status);
    }
private:
    /*
    * 求大于等于(小于等于)一个整数最小2次幂算法
    * [算法原理见] https://blog.csdn.net/Kakarotto_/article/details/108958843
    */
    size_t RebuildNum(size_t num)
    {
        size_t n = num - 1;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n < 0 ? 1 : n + 1;
    }
private:
    size_t buffer_size_ = 0;    // 缓冲区的长度
    size_t write_pos_ = 0;      // 可写入位置
    size_t read_pos_ = 0;       // 可读取位置
    char* buffer_ = nullptr;    // 缓冲区
    double ratio_ = 0;          // 警戒值
};

/*
* 无锁循环队列 [single producer-single consumer]
*/
template<typename Type, std::size_t Size>
class RingBufferSPSC 
{
public:
    /*
    * 构造
    */
    RingBufferSPSC()
        : read_pos_(0), write_pos_(0), count_(0)
    {}
    /*
    * 析构
    */
    ~RingBufferSPSC(){}
    /*
    * 判空,读线程调用
    */
    bool Empty() const
    {
        return (0 == count_);
    }
    /*
    * 判满,写线程调用
    */
    bool Full() const
    {
        return (Size == count_);
    }
    /*
    * 出队列,读线程调用
    */
    Type Pop()
    {
        auto type = array_[read_pos_];
        read_pos_ = (read_pos_ + 1) % Size;
        count_.fetch_sub(1);
        return type;
    }
    /*
    * 入队列,写线程调用
    */
    void Push(Type&& type)
    {
        array_[write_pos_] = type;
        write_pos_ = (write_pos_ + 1) % Size;
        count_.fetch_add(1);
    }
    /*
    * 清空
    */
    void Clear()
    {
        read_pos_ = 0;
        write_pos_ = 0;
        count_ = 0;
    }
private:
    std::array<Type, Size> array_;      // 数据存储
    std::size_t read_pos_;              // 读位置
    std::size_t write_pos_;             // 写位置
    std::atomic<std::size_t> count_;    // 元素数量
};

