#pragma once
#include <string.h>
#include <stdio.h>
#include <typeinfo>
#include <stdint.h>
#include "debug_print.h"

typedef unsigned long DWORD;
struct TypeDescriptor
{
    DWORD ptrToVTable;
    DWORD spare;
    char name[8];
};
struct PMD
{

    int mdisp;  //member displacement

    int pdisp;  //vbtable displacement

    int vdisp;  //displacement inside vbtable

};
struct RTTIBaseClassDescriptor

{

    struct TypeDescriptor* pTypeDescriptor; //type descriptor of the class

    DWORD numContainedBases; //number of nested classes following in the Base Class Array

    struct PMD where;        //pointer-to-member displacement info

    DWORD attributes;        //flags, usually 0

};

struct RTTIClassHierarchyDescriptor
{

    DWORD signature;      //always zero?

    DWORD attributes;     //bit 0 set = multiple inheritance, bit 1 set = virtual inheritance

    DWORD numBaseClasses; //number of classes in pBaseClassArray

    struct RTTIBaseClassArray* pBaseClassArray;

};

struct RTTICompleteObjectLocator
{

    DWORD signature; //always zero ?

    DWORD offset;    //offset of this vtable in the complete class

    DWORD cdOffset;  //constructor displacement offset

    struct TypeDescriptor* pTypeDescriptor; //TypeDescriptor of the complete class

    struct RTTIClassHierarchyDescriptor* pClassDescriptor; //describes inheritance hierarchy


};
// RTTI的这5个struct之间的关系参考:toolboox/doc/picture/RTTI_info.jpg

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
    type_info 微软的版本:
    Type information is generated for polymorphic classes only if the /GR (Enable Run-Time Type Information) compiler option is specified.
    type_info 只有在/GR 开了之后才被加到虚函数表
    https://docs.microsoft.com/sr-latn-rs/cpp/cpp/type-info-class?view=msvc-160
    */
    /*
    * 打印虚函数
    */
    void PrintVirtFunc()
    {
#ifdef __linux__
        Print("函数指针长度%d,对象的大小:%d\n", sizeof(FUNC), sizeof(object));
        Print("对象的typeid(object).name():%s\n",typeid(object).name());
        // 取 vptr值,如果有的话
        int32_t* vptr = reinterpret_cast<int32_t*>(&object);
        Print("\nvptr值:%p\n", *vptr);
        int64_t* vtable = reinterpret_cast<int64_t*>(*vptr);
        Print("\n虚表地址:%p \n", vtable);
        int32_t* rtti_address = reinterpret_cast<int32_t*>(vtable)-1;   // RTTI 地址在vptr地址的前面4个字节.
        Print("\nRTTI地址:%p \n", rtti_address);
        // auto str= *((RTTICompleteObjectLocator*)(rtti_address)); // 转换会dump,待寻找合适的方法.
        if (nullptr != vtable)
        {
            int i = 0;
            while(nullptr != reinterpret_cast<FUNC>(*vtable))
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