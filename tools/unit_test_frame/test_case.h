#pragma once
#include <list>
#include <string>

using ErrorList = std::list<std::string>;

/*
* 测试用例
*/
class TestCase
{
public:
	/*
	* 构造
	* @param name 测试用例名称
	*/
	TestCase(const std::string& name);
	/*
	* 析构
	*/
	virtual ~TestCase();
	/*
	* 运行测试用例
	*/
	virtual void Run() = 0;
	/*
	* 获取用例名称
	*/
	const std::string& GetName() const;
	/*
	* 禁用测试用例
	* @param need_run 是否需要运行
	*/
	void SetNeedRun(bool need_run);
	/*
	*  是否开启
	*/
	bool IsEnable();
	/*
	*  是否全部成功运行
	*/
	bool IsSuccess();
	/*
	*  设置错误信息
	*  @param error 错误信息
	*/
	void SetError(const std::string& error);
	/*
	* 取得错误列表
	*/
	const ErrorList& GetErrorList() const;
	/*
	* 清理错误
	*/
	void ClearError();

private:
	std::string name_;						// 测试用例名称
	bool run_;								// 是否需要运行
	ErrorList error_list_;		// 本次测试错误信息
};