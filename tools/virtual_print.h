#pragma once
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <typeinfo>


typedef void(*FUNC)(void);

/*
* 定义一个能够打印虚函数的类
*/
template <typename ClassType>
class Virtual
{
public:
    /*
    * 构造
    */
    Virtual(ClassType& obj)
    : object(obj)
    {
    }
    /*
    * 打印虚函数
    */
    void PrintVirtFunc()
    {
        int* vtable = (int*)(*(int*)&object);
        Print("\n虚表地址:%p\n", vtable);
        if (nullptr != vtable)
        {
            int i = 0;
            while(nullptr != vtable)
            {
                Print("\n第 %d 个虚函数地址: 0X%x,->", i + 1, vtable);
                FUNC f = (FUNC)vtable[i];
                Print(" FUNC:%x f:%x ",*vtable, f);
                if(nullptr != f)
                {
                    f();
                }

                i++;
                Print(" ^^^\n ");
                // vtable = (int*)(*((int*)&object + i));
                vtable = vtable + 1;
                
            }
        } 
        else
        {
            Print("%s 没有虚函数.",typeid(object).name());
        }
        Print("End.\n");

    }
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
private:
    ClassType& object;
    bool debug_print_ = false;
};