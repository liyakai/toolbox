# pragma once
#include <vector>
#include <string>

namespace ToolBox{

/*
* @brief 字符串分割函数
* @param str 待分割字符串
* @param tokens 已分割字符串
* @param dekunuters 分割字符
*/
void Split(const std::string str, std::vector<std::string>& tokens, const std::string& delimiters = " ")
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


}
