#pragma once
#include <stdio.h>
#include <algorithm>
#include <string.h>
/*
* 非线程安全的环形队列
* Type 数据类型
* Size 队列长度
* Ratio 警戒值,超过此容量,队列长度会自动增长一倍
*/
template <typename Type, size_t Size, size_t Ratio = 75>
class RingBuffer{
public:
    /*
    * 构造函数
    */
    RingBuffer()
        :buffer_size_(RebuildNum(Size)),ratio_((double)Ratio/100.0d)
    {
        buffer_ = new char[buffer_size_];
    }
    /*
    * 析构
    */
    ~RingBuffer()
    {
        delete[] buffer_;
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
    bool NeedEnlage()
    {
        auto ratio = (double)(buffer_size_ - WriteableSize()) / (double)buffer_size_;
        return ratio > ratio_;
    }
    /*
    * 可写数据长度[使用一个元素空间来判满]
    */
    size_t WriteableSize()
    {
        return (read_pos_ - write_pos_ - 1) % buffer_size_;
    }
    /*
    * 可读数据长度
    */
    size_t ReadableSize()
    {
        return (write_pos_ - read_pos_) % buffer_size_;
    }

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
    size_t Write(const char* buffer, size_t len)
    {
        if(NeedEnlage())
        {
            if(!Enlage())
            {
                return 0;
            }
        }
        len = std::min(len, WriteableSize());
        auto rbytes = std::min(len, buffer_size_ - write_pos_);
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
    size_t Read(char* buffer, size_t len)
    {
        len = std::min(len, ReadableSize());
        auto rbytes = std::min(len, buffer_size_ - read_pos_);
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
    size_t Copy(char* buffer, size_t len)
    {
        len = std::min(len, ReadableSize());
        auto rbytes = std::min(len, buffer_size_ - read_pos_);
        memmove(buffer, buffer_ + read_pos_, rbytes);
        memmove(buffer + rbytes, buffer_, len - rbytes);
        return len;
    }
    /*
    * 擦除
    * @param len 擦除的长度
    */
    size_t Remove(size_t len)
    {
        len = std::min(len, ReadableSize());
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
    void DebugPrint()
    {
        if(!debug_print_)
        {
            return;
        }
        printf("\n ========== RingBuffer ========== \n");
        printf("缓冲区大小:%zu,读位置:%zu,写位置:%zu,警戒值:%f\n", buffer_size_, read_pos_, write_pos_, ratio_);
        for(size_t i = 0; i < buffer_size_; i++)
        {
            // if(0 != i && 0 == (i % 10))
            // {
            //     printf("\n");
            // }
            printf("0x%02X ",buffer_[i]);
        }
        printf("\n ---------- RingBuffer ---------- \n");
    }
    /*
    * 开关 DebugPrint
    * @param enable 是否开启
    */
    void SetDebugPrint(bool enable)
    {
        debug_print_ = enable;
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
    size_t buffer_size_ = 0;    // 缓冲区最大长度
    size_t write_pos_ = 0;      // 可写入位置
    size_t read_pos_ = 0;       // 可读取位置
    char* buffer_ = nullptr;    // 缓冲区
    double ratio_ = 0;          // 警戒值
    bool debug_print_ = false;  // 是否打印调试信息
};