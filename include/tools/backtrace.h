#pragma once

#if defined(__x86_64__) || defined(_M_X64)
    // x86_64 架构的代码


// 简单版本
// #include <stdio.h>
// #include <stdint.h>

// // 定义栈帧结构
// typedef struct {
//     void* rbp;  // 基址指针
//     void* rip;  // 返回地址
// } StackFrame;

// inline void print_trace() {
//     // 获取当前 rbp
//     void* rbp;
//     asm volatile("movq %%rbp, %0" : "=r"(rbp));
    
//     // 遍历栈帧
//     StackFrame* frame = (StackFrame*)rbp;
//     int depth = 0;
    
//     while(frame && depth < 10) { // 限制深度
//         printf("Frame %d: %p\n", depth++, frame->rip);
//         frame = (StackFrame*)frame->rbp;
//     }
// }

// 增加安全检查的优化版本
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define MAX_FRAMES 32
#define IS_ALIGNED(x) (((uintptr_t)(x) & (sizeof(void*)-1)) == 0)

typedef struct {
    void* rbp;  // 基址指针
    void* rip;  // 返回地址
} StackFrame;

typedef struct {
    void* frames[MAX_FRAMES];
    int count;
} CallStack;

static inline int is_valid_ptr(void* ptr) {
    // 简单的地址范围检查
    return ptr > (void*)4096 && ptr < (void*)-4096;
}

inline int get_callstack(CallStack* cs) {
    void* rbp;
    asm volatile("movq %%rbp, %0" : "=r"(rbp));
    
    StackFrame* frame = (StackFrame*)rbp;
    cs->count = 0;
    
    while(frame && cs->count < MAX_FRAMES) {
        // 安全性检查
        if(!IS_ALIGNED(frame) || !is_valid_ptr(frame->rip)) {
            break;
        }
        
        cs->frames[cs->count++] = frame->rip;
        
        // 检查下一个帧
        StackFrame* next = (StackFrame*)frame->rbp;
        if(next <= frame) break;
        frame = next;
    }
    
    return cs->count;
}

inline void print_callstack()
{
    CallStack cs;
    get_callstack(&cs);
    // for (int i = 0; i < cs.count; i++) {
    //     printf("Frame %d: %p\n", i, cs.frames[i]);
    // }
}

#else
    #warning "This code only supports x86_64 architecture"
#endif




// ==============================  backtrace 方式,不推荐! backtrace_symbols 单次调用的开销在毫秒级.  ==============================
// 带符号解析的堆栈跟踪

#include <execinfo.h>     // backtrace
#include <dlfcn.h>        // dladdr
#include <cxxabi.h>       // abi::__cxa_demangle
#include <string>

class StackTracer {
public:
    static std::string get_trace() {
        const int max_frames = 32;
        void* frames[max_frames];
        backtrace(frames, max_frames);
        int num_frames = backtrace(frames, max_frames);
        
        // // 使用 abi::__cxa_demangle 进行符号解析
        backtrace_symbols(frames, num_frames);
        // char** symbols = backtrace_symbols(frames, num_frames);
        std::string result;
        
        // for (int i = 0; i < num_frames; i++) {
        //     Dl_info info;
        //     if (dladdr(frames[i], &info)) {
        //         int status;
        //         char* demangled = abi::__cxa_demangle(
        //             info.dli_sname, nullptr, nullptr, &status);
                
        //         if (demangled) {
        //             result += demangled;
        //             result += "\n";
        //             free(demangled);
        //         } else {
        //             result += symbols[i];
        //             result += "\n";
        //         }
        //     }
        // }
        
        // free(symbols);
        return result;
    }
};