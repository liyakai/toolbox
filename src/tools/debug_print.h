#pragma once
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <string>
/*
* 定义 debug 打印基类
*/
class DebugPrint
{
private:
    // 是否开启 debug 打印
    bool debug_print_ = false;
public:
    DebugPrint(){};
    virtual ~DebugPrint(){};
    /*
    * 自定义 打印函数
    */
    void Print(const char* format, ...)
    {
        if(debug_print_)
        {
            va_list args;
            char str[1024];
            memset(str, 0 ,sizeof(char) *1024);
            va_start(args, format);
            vsprintf(str, format, args);
            va_end(args);
            printf("%s",str);
            fflush(stdout);
        }
    }
    /*
    * 开关 DebugPrint
    * @param enable 是否开启
    */
    void SetDebugPrint(bool enable)
    {
        debug_print_ = enable;
    }
    /*
    * 获取 debug print 状态
    */
   bool GetDebugStatus()
   {
       return debug_print_;
   }
   /*
   * 打印二进制内容
   */
   void PrintData(const char* data, size_t size, std::string msg = "")
   {
        if(!msg.empty())
        {
            Print("%s\n", msg.c_str());
        }
        
        for(size_t i = 0; i < size; i++)
        {
            if(0 != i && 0 == i % 16)
            {
                Print("\n");
            } 
            Print("0x%02x ", data[i]);     
        }
        Print("\n");
   }
};
