#include "tools/ringbuffer.h"
#include "tools/unit_test_frame/unittest.h"


FIXTURE_BEGIN(RingBuffer)

CASE(ringbuffer_write)
{
    /*
    * 测试写
    */
    const size_t config_buffer_size = 32;
    RingBuffer<char,config_buffer_size> ring_buffer;
    ring_buffer.SetDebugPrint(false);
    ring_buffer.DebugPrint();
    for(size_t i = 0; i < config_buffer_size * 2; i++)
    {
        bool result = ring_buffer.Write(char('a'+ (i%26)));
        if(false == result)
        {
            SetError("ringbuffer 写失败.");
        }
        ring_buffer.DebugPrint();
    }
}

CASE(ringbuffer_write_read)
{
    /*
    * 测试写与读 写比读快
    */
    const size_t config_buffer_size = 32;
    RingBuffer<char,config_buffer_size> ring_buffer;
    ring_buffer.SetDebugPrint(false);
    ring_buffer.DebugPrint();
    char c = '-';
    bool result = false;
    for(size_t i = 0; i < config_buffer_size * 2; i++)
    {
        result = ring_buffer.Write(char('a'+ (i%26)));
        if(false == result)
        {
            SetError("ringbuffer 写失败.");
        }
        if(0 == i%2)
        {
            result = ring_buffer.Read(c);
            if(false == result)
            {
                SetError("ringbuffer 读失败.");
            }else
            {
                // printf("读出的内容为%c.\n",c);
            }
        }
        ring_buffer.DebugPrint();
    }
}


CASE(ringbuffer_read_write)
{
    /*
    * 测试写与读 读比写快
    */
    const size_t config_buffer_size = 32;
    RingBuffer<char,config_buffer_size> ring_buffer;
    ring_buffer.SetDebugPrint(false);
    ring_buffer.DebugPrint();
    char c = '-';
    bool result = false;
    for(size_t i = 0; i < config_buffer_size * 2; i++)
    {
        for(size_t j = 0; j < 2; j++)
        {
            result = ring_buffer.Read(c);
            if(false == result)
            {
                // SetError("ringbuffer 读失败.");
                // printf("读取失败.");
            }else
            {
                // printf("读出的内容为%c.\n",c);
            }
        }
        
        result = ring_buffer.Write(char('a'+ (i%26)));
        if(false == result)
        {
            SetError("ringbuffer 写失败.");
        }
        ring_buffer.DebugPrint();
    }
}

CASE(ringbuffer_copy_write)
{
    /*
    * 测试写与拷贝 拷贝比写快
    */
    const size_t config_buffer_size = 32;
    RingBuffer<char,config_buffer_size> ring_buffer;
    ring_buffer.SetDebugPrint(false);
    ring_buffer.DebugPrint();
    char c = '-';
    bool result = false;
    for(size_t i = 0; i < config_buffer_size * 2; i++)
    {
        for(size_t j = 0; j < 2; j++)
        {
            result = ring_buffer.Copy(c);
            if(false == result)
            {
                // SetError("ringbuffer 读失败.");
                // printf("读取失败.");
            }else
            {
                // printf("读出的内容为%c.\n",c);
            }
        }
        
        result = ring_buffer.Write(char('a'+ (i%26)));
        if(false == result)
        {
            SetError("ringbuffer 写失败.");
        }
        ring_buffer.DebugPrint();
    }
}


FIXTURE_END(RingBuffer)