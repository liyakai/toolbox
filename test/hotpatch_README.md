# 二进制热修补功能

## 概述

这个热修补功能实现了直接修改进程内存中函数指令的能力，通过跳转指令将函数调用重定向到新的补丁函数。支持x86_64架构。

## 主要特性

1. **直接内存修改**: 直接修改进程内存中的函数指令
2. **跳转指令生成**: 自动生成x86_64架构的跳转指令
3. **内存权限管理**: 自动处理内存权限的修改和恢复
4. **原始指令备份**: 备份原始指令以便恢复
5. **状态管理**: 跟踪修补状态，防止重复修补

## 使用方法

### 1. 基本用法

```cpp
#include "include/tools/hotpatch.h"

// 创建热修补实例
hotpatch::HotPatch hotpatch;

// 加载补丁库
if (!hotpatch.load_patch("/path/to/patch.so")) {
    // 处理加载失败
}

// 执行热修补
if (hotpatch.patch_function("patched_function_name", target_function_ptr)) {
    // 修补成功
}

// 恢复原始函数
hotpatch.restore_function(target_function_ptr);
```

### 2. 补丁库创建

补丁库是一个动态链接库(.so文件)，包含要替换的函数：

```cpp
// patch.cpp
extern "C" {
    int patched_function(int x) {
        // 新的函数实现
        return x * 3;
    }
}
```

编译补丁库：
```bash
g++ -shared -fPIC -o patch.so patch.cpp
```

### 3. 完整示例

```cpp
#include <iostream>
#include "include/tools/hotpatch.h"

// 原始函数
int original_function(int x) {
    std::cout << "原始函数: " << x << std::endl;
    return x * 2;
}

int main() {
    hotpatch::HotPatch hotpatch;
    
    // 加载补丁库
    hotpatch.load_patch("patch.so");
    
    // 修补函数
    hotpatch.patch_function("patched_function", 
                           reinterpret_cast<void*>(original_function));
    
    // 现在调用original_function会执行patched_function
    int result = original_function(5);  // 输出: 15 (5*3)
    
    // 恢复原始函数
    hotpatch.restore_function(reinterpret_cast<void*>(original_function));
    
    // 现在调用original_function会执行原始实现
    result = original_function(5);  // 输出: 10 (5*2)
    
    return 0;
}
```

## API 参考

### HotPatch 类

#### 构造函数
- `HotPatch()`: 创建热修补实例

#### 公共方法

- `bool load_patch(const std::string& so_path)`: 加载补丁库
- `void unload_patch()`: 卸载补丁库
- `template<typename Func> Func get_function(const std::string& func_name)`: 获取补丁函数
- `bool is_loaded() const`: 检查补丁库是否已加载
- `bool patch_function(const std::string& func_name, void* target_func)`: 执行热修补
- `bool restore_function(void* target_func)`: 恢复原始函数
- `bool is_patched(void* target_func) const`: 检查函数是否已被修补
- `std::vector<void*> get_patched_functions() const`: 获取已修补的函数列表

## 技术实现

### 跳转指令

使用x86_64架构的`jmp rel32`指令（操作码0xE9）实现函数跳转：

```cpp
struct JumpInstruction {
    uint8_t opcode;    // 0xE9
    int32_t offset;    // 相对偏移量
};
```

### 内存权限管理

使用`mprotect`系统调用修改内存权限：
- 修补时：设置为`PROT_READ | PROT_WRITE | PROT_EXEC`
- 恢复时：设置为`PROT_READ | PROT_EXEC`

### 原始指令备份

备份函数开头的8字节指令，确保能够完整恢复原始函数。

## 注意事项

1. **架构限制**: 目前仅支持x86_64架构
2. **函数大小**: 目标函数开头至少需要5字节空间放置跳转指令
3. **内存对齐**: 自动处理页面对齐问题
4. **线程安全**: 当前实现不是线程安全的
5. **权限要求**: 需要修改内存权限，可能需要特殊权限

## 编译和运行

```bash
# 编译补丁库
g++ -shared -fPIC -o patch.so patch.cpp

# 编译测试程序
g++ -std=c++17 -I./include -o test_hotpatch test/simple_hotpatch_test.cpp -ldl

# 运行测试
./test_hotpatch
```

## 错误处理

- 补丁库加载失败：检查文件路径和权限
- 热修补失败：检查函数地址和补丁函数名称
- 内存权限修改失败：检查系统权限设置
- 恢复失败：检查是否已备份原始指令

