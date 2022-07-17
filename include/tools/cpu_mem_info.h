#pragma once

#ifdef __linux__

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/sysinfo.h>
 
/*
* @file brief 获取CPU和内存占用情况
*/


#define VMRSS_LINE 17
#define VMSIZE_LINE 13
#define PROCESS_ITEM 14


typedef struct {
	unsigned long user;
	unsigned long nice;
	unsigned long system;
	unsigned long idle;
}Total_Cpu_Occupy_t;
 
 
typedef struct {
	unsigned int pid;
	unsigned long utime;  //user time
	unsigned long stime;  //kernel time
	unsigned long cutime; //all user time
    unsigned long cstime; //all dead time
}Proc_Cpu_Occupy_t;


/*
* @brief 定义获取CPU和内存的工具函数
*/
class CpuMemInfo
{
public:
    /*
    * 获取CPU占用率
    */
    float GetProcCpu(uint32_t pid){
        
        uint64_t totalcputime1, totalcputime2;
        uint64_t procputime1, procputime2;
        
        totalcputime1=last_total_cpu_time_;
        procputime1=last_pro_cpu_time_;

        int32_t cpu_num = get_nprocs();
    
        totalcputime2=GetCpuTotalOccupy() / cpu_num;
        procputime2=GetCpuProcOccupy(pid);
        
        last_total_cpu_time_ = totalcputime2;
        last_pro_cpu_time_ = procputime2;
        
        float pcpu = 0.0;
        if(0 != totalcputime2-totalcputime1){ 
            pcpu=100.0 * (procputime2 - procputime1) / (totalcputime2 - totalcputime1);
        }
        
        return pcpu;
    }
        
    /*
    * 获取进程占用内存
    */
    unsigned int GetProcMem(unsigned int pid){
        
        char file_name[64]={0};
        FILE *fd;
        char line_buff[512]={0};
        sprintf(file_name,"/proc/%d/status",pid);
        
        fd =fopen(file_name,"r");
        if(nullptr == fd){
            return 0;
        }
        
        char name[64];
        int vmrss;
        for (int i=0; i<VMRSS_LINE-1;i++){
            fgets(line_buff,sizeof(line_buff),fd);
        }
        
        fgets(line_buff,sizeof(line_buff),fd);
        sscanf(line_buff,"%s %d",name,&vmrss);
        fclose(fd);
    
        return vmrss;
    }
    
    
    /*
    * 获取进程占用虚拟内存
    */
    unsigned int GetProcVirtualMem(unsigned int pid){
        
        char file_name[64]={0};
        FILE *fd;
        char line_buff[512]={0};
        sprintf(file_name,"/proc/%d/status",pid);
        
        fd =fopen(file_name,"r");
        if(nullptr == fd){
            return 0;
        }
        
        char name[64];
        int vmsize;
        for (int i=0; i<VMSIZE_LINE-1;i++){
            fgets(line_buff,sizeof(line_buff),fd);
        }
        
        fgets(line_buff,sizeof(line_buff),fd);
        sscanf(line_buff,"%s %d",name,&vmsize);
        fclose(fd);
    
        return vmsize;
    }

        
    /*
    * 获取进程
    */
    int GetPid(const char* process_name, const char* user = nullptr)
    {
        if(user == nullptr){
            user = getlogin();	
        }
        
        char cmd[512];
        if (user){
            sprintf(cmd, "pgrep %s -u %s", process_name, user);	
        }
    
        FILE *pstr = popen(cmd,"r");	
        
        if(pstr == nullptr){
            return 0;	
        }
    
        char buff[512];
        ::memset(buff, 0, sizeof(buff));
        if(NULL == fgets(buff, 512, pstr)){
            return 0;
        }
    
        return atoi(buff);
    }

private:
    /*
    * @brief 获取第N项开始的指针
    */
    const char* GetItems(const char*buffer ,uint32_t item){
        
        const char *p =buffer;
    
        int len = strlen(buffer);
        uint32_t count = 0;
        
        for (int i=0; i<len;i++){
            if (' ' == *p){
                count ++;
                if(count == item -1){
                    p++;
                    break;
                }
            }
            p++;
        }
    
        return p;
    }

        
    /*
    * 获取总的CPU时间
    */
    uint64_t GetCpuTotalOccupy(){
        
        FILE *fd;
        char buff[1024]={0};
        Total_Cpu_Occupy_t t;
    
        fd =fopen("/proc/stat","r");
        if (nullptr == fd){
            return 0;
        }
            
        fgets(buff,sizeof(buff),fd);
        char name[64]={0};
        sscanf(buff,"%s %ld %ld %ld %ld",name,&t.user,&t.nice,&t.system,&t.idle);
        fclose(fd);
        
        return (t.user + t.nice + t.system + t.idle);
    }

        
    /*
    * 获取进程的CPU时间
    */
    uint64_t GetCpuProcOccupy(uint32_t pid){
        
        char file_name[64]={0};
        Proc_Cpu_Occupy_t t;
        FILE *fd;
        char line_buff[1024]={0};
        sprintf(file_name,"/proc/%d/stat",pid);
        
        fd = fopen(file_name,"r");
        if(nullptr == fd){
            return 0;
        }
        
        fgets(line_buff,sizeof(line_buff),fd);
        
        sscanf(line_buff,"%u",&t.pid);
        const char *q =GetItems(line_buff,PROCESS_ITEM);
        sscanf(q,"%ld %ld %ld %ld",&t.utime,&t.stime,&t.cutime,&t.cstime);
        fclose(fd);
        
        return (t.utime + t.stime + t.cutime + t.cstime);
    }

private:
    static uint64_t last_total_cpu_time_;
    static uint64_t last_pro_cpu_time_;
};

uint64_t CpuMemInfo::last_total_cpu_time_;
uint64_t CpuMemInfo::last_pro_cpu_time_;

#endif // __linux__