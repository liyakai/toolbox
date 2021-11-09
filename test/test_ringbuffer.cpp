#include "src/tools/ringbuffer.h"
#include "unit_test_frame/unittest.h"
#include <vector>


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

std::vector<int32_t> product;
std::vector<int32_t> result;
RingBufferSPSC<int32_t, 17> ring_buffer;
bool stop = false;
void WriteThread()
{
    for(int32_t val : product)
    {
        while(true)
        {
            if(ring_buffer.Empty())
            {
                ring_buffer.Push(std::move(val));
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    stop = true;
}
void ReadThread()
{
    while (true)
    {
        if(!ring_buffer.Empty())
        {
            result.emplace_back(ring_buffer.Pop());
        } else if (stop)
        {
            break;
        }
    }
    
}


CASE(ringbuffer_spsc)
{
    /*
    * 测试单生产者单消费者队列
    */
    for(std::size_t i = 0; i < 1000; i++)
    {
        product.emplace_back(i);
    }
    std::thread write(WriteThread);
    std::thread read(ReadThread);
    write.join();
    read.join();
    if(result.size() != product.size())
    {
        SetError("ringbuffer_spsc 结果与原始数据大小不一致.");
    }
    for(std::size_t i = 0; i < result.size(); i++)
    {
        if(result[i] != product[i])
        {
            SetError("ringbuffer_spsc 结果与原始数据不一致.");
            break;
        }
    }
}


FIXTURE_END(RingBuffer)