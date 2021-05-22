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
        int32_t* vtable = (int32_t*)(*(int32_t*)&object);
        Print("\n虚表地址:%p\n", vtable);
        if (nullptr != vtable)
        {
            int i = 0;
            while(nullptr != (FUNC)*vtable)
            {
                Print("第 %d 个虚函数地址: 0X%x ->", ++i, vtable);
                FUNC f = (FUNC)*vtable;
                //Print(" FUNC:%x f:%x ",*vtable, f);
                if(nullptr != f && true == GetDebugStatus())
                {
                    f();
                }
                vtable = vtable + 2;
                
            }
        } 
        else
        {
            Print("没有虚函数\n ");
            Print("%s 没有虚函数.",typeid(object).name());
        }
        Print("\n");

    }
private:
    ClassType& object;
};