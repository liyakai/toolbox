#pragma once
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
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
};
