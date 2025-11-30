# pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace ToolBox{

/*
* @brief 字符串分割函数
* @param str 待分割字符串
* @param tokens 已分割字符串
* @param dekunuters 分割字符
*/
inline void Split(const std::string str, std::vector<std::string>& tokens, const std::string& delimiters = " ")
{
	// 在字符串str中查找第一个与delimiters中的任意字符都不匹配的字符，返回它的位置。搜索从index=0开始。如果没找到就返回string::nops
	std::string::size_type last_pos = str.find_first_not_of(delimiters, 0);
	// 在字符串str中从last_pos位置开始查找第一个与delimiters匹配的字符,返回它的位置.如果没炸到就返回 string::nops
	std::string::size_type pos = str.find_first_of(delimiters, last_pos);
	while (std::string::npos != pos || std::string::npos != last_pos)
	{
		tokens.emplace_back(str.substr(last_pos, pos - last_pos));
		last_pos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, last_pos);
	}
}

/*
* @brief 以 16进制将内存转为字符串.
*/

inline std::string MemoryToStr(const char* str, uint32_t char_len)
{
    if (nullptr == str)
    {
        return "";
    }
    std::string result;
    result.reserve(2 * char_len);  // 预分配空间提高效率
    
    static const char hex_chars[] = "0123456789ABCDEF";
    for(uint32_t i = 0; i < char_len; ++i) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        result += hex_chars[c >> 4];    // 取高4位
        result += hex_chars[c & 0x0F];  // 取低4位
    }
    
    return result;
}

/*
* @brief 将 vector 中的内容转为字符串,默认以逗号为间隔
*/
template<typename T>
inline std::string VectorToStr(const std::vector<T>& vec, const std::string& delimiter = ",")
{
    std::string result;
    result.reserve(vec.size() * sizeof(T));
    for(const auto& item : vec)
    {
        result += std::to_string(item);
        if(&item != &vec.back())
        {
            result += delimiter;
        }
    }
    return result;
}   


}
