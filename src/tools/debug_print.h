#pragma once
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <string>
/*
* ���� debug ��ӡ����
*/
class DebugPrint
{
private:
    // �Ƿ��� debug ��ӡ
    bool debug_print_ = false;
public:
    DebugPrint(){};
    virtual ~DebugPrint(){};
    /*
    * �Զ��� ��ӡ����
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
    * ���� DebugPrint
    * @param enable �Ƿ���
    */
    void SetDebugPrint(bool enable)
    {
        debug_print_ = enable;
    }
    /*
    * ��ȡ debug print ״̬
    */
   bool GetDebugStatus()
   {
       return debug_print_;
   }
   /*
   * ��ӡ����������
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
