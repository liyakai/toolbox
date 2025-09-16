#pragma once

#if !defined(__linux__)
#error "This program can only run on Linux"
#endif

/*
 * 二进制热修补功能
 * 直接修改进程内存中的函数指令，用跳转指令指向新函数
 * 支持x86_64架构
 */

#include <dlfcn.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdint>
#include <cassert>
#include <sstream>

namespace hotpatch {

// x86_64架构的跳转指令结构
struct JumpInstruction {
    uint8_t opcode;           // 0xE9 (jmp rel32) 或 0xFF (jmp [rip+offset])
    int32_t offset;           // 相对偏移量
};

// 绝对跳转指令结构（用于大偏移量）
struct AbsoluteJumpInstruction {
    uint8_t opcode1;          // 0xFF
    uint8_t opcode2;          // 0x25
    int32_t offset;           // 相对偏移量到目标地址
    uint64_t target_addr;     // 目标地址
};

// 原始指令备份
struct OriginalInstruction {
    std::vector<uint8_t> data;  // 原始指令数据
    size_t size;                // 指令大小
};

class HotPatch {
public:
    HotPatch() : handle_(nullptr) {}

    ~HotPatch() {
        unload_patch();
    }

    // 加载补丁库
    bool load_patch(const std::string& so_path) {
        unload_patch();
        handle_ = dlopen(so_path.c_str(), RTLD_NOW);
        return handle_ != nullptr;
    }

    // 卸载补丁库
    void unload_patch() {
        if (handle_) {
            dlclose(handle_);
            handle_ = nullptr;
        }
        // 清理所有热修补
        for (auto& pair : original_instructions_) {
            restore_function(pair.first);
        }
        original_instructions_.clear();
        // 清理虚函数表条目（注意：这里无法自动恢复，需要手动处理）
        original_vtable_entries_.clear();
    }

    // 查找补丁中的函数
    template<typename Func>
    Func get_function(const std::string& func_name) {
        if (!handle_) return nullptr;
        return reinterpret_cast<Func>(dlsym(handle_, func_name.c_str()));
    }

    // 判断补丁是否已加载
    bool is_loaded() const { return handle_ != nullptr; }

    // 二进制热修补：直接修改函数指令跳转到补丁函数
    bool patch_function(const std::string& func_name, void* target_func) {
        if (!handle_ || !target_func) {
            return false;
        }

        // 获取补丁函数地址
        auto patch_func = get_function<void*>(func_name);
        if (!patch_func) {
            return false;
        }

        // 检查是否已经修补过
        if (original_instructions_.find(target_func) != original_instructions_.end()) {
            return false; // 已经修补过
        }

        // 备份原始指令
        if (!backup_original_instruction(target_func)) {
            return false;
        }

        // 生成跳转指令
        JumpInstruction jump = generate_jump_instruction(target_func, patch_func);
        
        // 检查跳转指令是否有效
        if (jump.offset == 0 && jump.opcode == 0xE9) {
            // 偏移量太大，使用绝对跳转
            return patch_function_absolute(target_func, patch_func);
        }
        
        // 修改内存权限
        if (!make_memory_writable(target_func, sizeof(JumpInstruction))) {
            return false;
        }

        // 写入跳转指令
        std::memcpy(target_func, &jump, sizeof(JumpInstruction));

        // 刷新指令缓存
        flush_instruction_cache(target_func, sizeof(JumpInstruction));

        // 恢复内存权限
        make_memory_executable(target_func, sizeof(JumpInstruction));

        return true;
    }

    // 恢复原始函数
    bool restore_function(void* target_func) {
        auto it = original_instructions_.find(target_func);
        if (it == original_instructions_.end()) {
            return false; // 没有找到备份
        }

        const auto& original = it->second;
        
        // 修改内存权限
        if (!make_memory_writable(target_func, original.size)) {
            return false;
        }

        // 恢复原始指令
        std::memcpy(target_func, original.data.data(), original.size);

        // 刷新指令缓存
        flush_instruction_cache(target_func, original.size);

        // 恢复内存权限
        make_memory_executable(target_func, original.size);

        // 删除备份
        original_instructions_.erase(it);
        return true;
    }

    // 检查函数是否已被修补
    bool is_patched(void* target_func) const {
        return original_instructions_.find(target_func) != original_instructions_.end();
    }

    // 获取已修补的函数列表
    std::vector<void*> get_patched_functions() const {
        std::vector<void*> result;
        for (const auto& pair : original_instructions_) {
            result.push_back(pair.first);
        }
        return result;
    }

    // 虚函数热修补：修改虚函数表中的函数指针
    template<typename T>
    bool patch_virtual_function(const std::string& func_name, T* object, int vtable_index) {
        if (!handle_ || !object) {
            return false;
        }

        // 获取补丁函数地址
        auto patch_func = get_function<void*>(func_name);
        if (!patch_func) {
            return false;
        }

        // 获取对象的虚函数表
        void** vtable = *reinterpret_cast<void***>(object);
        if (!vtable) {
            return false;
        }

        // 检查vtable索引是否有效
        if (vtable_index < 0) {
            return false;
        }

        // 获取原始虚函数地址
        void* original_vfunc = vtable[vtable_index];
        if (!original_vfunc) {
            return false;
        }

        // 检查是否已经修补过
        std::string vtable_key = create_vtable_key(object, vtable_index);
        if (original_vtable_entries_.find(vtable_key) != original_vtable_entries_.end()) {
            return false; // 已经修补过
        }

        // 备份原始虚函数地址
        original_vtable_entries_[vtable_key] = original_vfunc;

        // 修改内存权限
        if (!make_memory_writable(vtable, sizeof(void*))) {
            return false;
        }

        // 替换虚函数表中的函数指针
        vtable[vtable_index] = patch_func;

        // 刷新指令缓存
        flush_instruction_cache(vtable, sizeof(void*));

        // 恢复内存权限
        make_memory_executable(vtable, sizeof(void*));

        return true;
    }

    // 恢复虚函数
    template<typename T>
    bool restore_virtual_function(T* object, int vtable_index) {
        if (!object) {
            return false;
        }

        std::string vtable_key = create_vtable_key(object, vtable_index);
        auto it = original_vtable_entries_.find(vtable_key);
        if (it == original_vtable_entries_.end()) {
            return false; // 没有找到备份
        }

        void** vtable = *reinterpret_cast<void***>(object);
        if (!vtable) {
            return false;
        }

        // 修改内存权限
        if (!make_memory_writable(vtable, sizeof(void*))) {
            return false;
        }

        // 恢复原始虚函数地址
        vtable[vtable_index] = it->second;

        // 刷新指令缓存
        flush_instruction_cache(vtable, sizeof(void*));

        // 恢复内存权限
        make_memory_executable(vtable, sizeof(void*));

        // 删除备份
        original_vtable_entries_.erase(it);
        return true;
    }

    // 检查虚函数是否已被修补
    template<typename T>
    bool is_virtual_function_patched(T* object, int vtable_index) const {
        if (!object) return false;
        std::string vtable_key = create_vtable_key(object, vtable_index);
        return original_vtable_entries_.find(vtable_key) != original_vtable_entries_.end();
    }

private:
    void* handle_;
    std::unordered_map<void*, OriginalInstruction> original_instructions_;
    std::unordered_map<std::string, void*> original_vtable_entries_;  // 虚函数表条目备份

    // 备份原始指令
    bool backup_original_instruction(void* func_addr) {
        // 对于x86_64，我们至少需要5字节来放置跳转指令
        // 这里简化处理，备份前8字节（确保覆盖完整的指令）
        const size_t backup_size = 8;
        
        OriginalInstruction original;
        original.data.resize(backup_size);
        original.size = backup_size;
        
        // 复制原始指令
        std::memcpy(original.data.data(), func_addr, backup_size);
        
        original_instructions_[func_addr] = std::move(original);
        return true;
    }

    // 生成跳转指令
    JumpInstruction generate_jump_instruction(void* from, void* to) {
        JumpInstruction jump;
        jump.opcode = 0xE9; // jmp rel32
        
        // 计算相对偏移量
        // jmp rel32 指令长度为5字节，所以目标地址是 from + 5 + offset
        int64_t offset = reinterpret_cast<char*>(to) - (reinterpret_cast<char*>(from) + 5);
        
        // 检查偏移量是否在32位范围内
        if (offset < INT32_MIN || offset > INT32_MAX) {
            // 如果偏移量太大，我们需要使用不同的跳转方式
            // 这里先返回一个无效的跳转指令
            jump.offset = 0;
            return jump;
        }
        
        jump.offset = static_cast<int32_t>(offset);
        
        return jump;
    }

    // 刷新指令缓存
    void flush_instruction_cache(void* addr, size_t size) {
        // 在x86_64架构上，通常不需要显式刷新指令缓存
        // 但为了保险起见，我们可以使用内存屏障
        __asm__ __volatile__("" ::: "memory");
        
        // 如果系统支持，也可以使用更明确的指令缓存刷新
        // 但这通常不是必需的，因为现代x86_64处理器会自动处理
    }

    // 使用绝对跳转进行热修补
    bool patch_function_absolute(void* target_func, void* patch_func) {
        // 对于绝对跳转，我们需要更多的空间
        const size_t jump_size = 14; // 绝对跳转需要14字节
        
        // 备份原始指令（需要更多空间）
        OriginalInstruction original;
        original.data.resize(jump_size);
        original.size = jump_size;
        std::memcpy(original.data.data(), target_func, jump_size);
        original_instructions_[target_func] = std::move(original);
        
        // 修改内存权限
        if (!make_memory_writable(target_func, jump_size)) {
            return false;
        }
        
        // 生成绝对跳转指令
        // 使用 mov rax, target_addr; jmp rax 的方式
        uint8_t* code = static_cast<uint8_t*>(target_func);
        
        // mov rax, patch_func
        code[0] = 0x48;  // REX.W prefix
        code[1] = 0xB8;  // mov rax, imm64
        *reinterpret_cast<uint64_t*>(&code[2]) = reinterpret_cast<uint64_t>(patch_func);
        
        // jmp rax
        code[10] = 0xFF; // jmp r/m64
        code[11] = 0xE0; // jmp rax
        
        // 填充剩余字节为nop
        for (size_t i = 12; i < jump_size; ++i) {
            code[i] = 0x90; // nop
        }
        
        // 刷新指令缓存
        flush_instruction_cache(target_func, jump_size);
        
        // 恢复内存权限
        make_memory_executable(target_func, jump_size);
        
        return true;
    }

    // 修改内存权限为可写
    bool make_memory_writable(void* addr, size_t size) {
        // 按页对齐
        size_t page_size = getpagesize();
        void* aligned_addr = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(addr) & ~(page_size - 1)
        );
        
        size_t aligned_size = size + (reinterpret_cast<char*>(addr) - reinterpret_cast<char*>(aligned_addr));
        aligned_size = (aligned_size + page_size - 1) & ~(page_size - 1);
        
        return mprotect(aligned_addr, aligned_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
    }

    // 修改内存权限为可执行
    bool make_memory_executable(void* addr, size_t size) {
        // 按页对齐
        size_t page_size = getpagesize();
        void* aligned_addr = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(addr) & ~(page_size - 1)
        );
        
        size_t aligned_size = size + (reinterpret_cast<char*>(addr) - reinterpret_cast<char*>(aligned_addr));
        aligned_size = (aligned_size + page_size - 1) & ~(page_size - 1);
        
        return mprotect(aligned_addr, aligned_size, PROT_READ | PROT_EXEC) == 0;
    }

    // 创建虚函数表条目的唯一键
    template<typename T>
    std::string create_vtable_key(T* object, int vtable_index) const {
        void** vtable = *reinterpret_cast<void***>(object);
        std::stringstream ss;
        ss << "vtable_" << vtable << "_" << vtable_index;
        return ss.str();
    }
};

} // namespace hotpatch

