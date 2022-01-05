#pragma once
#include <string.h>
#include <stdio.h>
#include <typeinfo>
#include <stdint.h>
#include "debug_print.h"


typedef void(*FUNC)(void);

/*
* 定义一个能够打印虚函数的类
*/
template <typename ClassType>
class Virtual : public DebugPrint
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
#ifdef __linux__
        Print("函数指针长度%d,对象的大小:%d\n", sizeof(FUNC), sizeof(object));
        // 取 vptr值,如果有的话
        int32_t* vptr = reinterpret_cast<int32_t*>(&object);
        Print("\nvptr值:%p\n", *vptr);
        int64_t* vtable = reinterpret_cast<int64_t*>(*vptr);
        Print("\n虚表地址:%p \n", vtable);
        Print("\n虚表地址:%p %p\n", vtable, vtable+1);
        if (nullptr != vtable)
        {
            int i = 0;
            while(nullptr != reinterpret_cast<FUNC>(*vtable))
            // while(nullptr != static_cast<FUNC>(*vtable))
            {
                Print("第 %d 个虚函数地址: 0X%x ->", ++i, vtable);
                FUNC f = reinterpret_cast<FUNC>(*vtable);
                //Print(" FUNC:%x f:%x ",*vtable, f);
                if(nullptr != f && true == GetDebugStatus())
                {
                    f();
                }
                vtable = vtable + 1;
                
            }
        } 
        else
        {
            Print("没有虚函数\n ");
            Print("%s 没有虚函数.",typeid(object).name());
        }
        Print("\n");
#endif // __linux__
    }
private:
    ClassType& object;
};