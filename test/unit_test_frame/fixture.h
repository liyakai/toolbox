#pragma  once

#include <string>
#include <functional>
#include "test_case.h"

class TestCase;

using FixtureInitMethod = std::function<void()>;
using FixtureUninitMethod = std::function<void()>;

class Fixture
{
public:
	/*
	* 构造
	* @param name 名称
	*/
	Fixture(const std::string& name);
	/*
	* 析构
	*/
	virtual ~Fixture();
	/*
	* 初始化
	*/
	void Init();
	/*
	* 逆初始化
	*/
	void UnInit();
	/*
	* 设置初始化函数
	* @param method 初始化函数
	*/
	bool SetInitFunc(FixtureInitMethod method);
	/*
	* 设置逆初始化函数
	* @param method 逆初始化函数
	*/
	bool SetUninitFunc(FixtureUninitMethod method);
	/*
	* 获取名称
	*/
	const std::string& GetName()const;
	/*
	* 添加测试用例
	* @param testcase 测试用例指针 
	*/
	bool AddTestCase(TestCase* testcase);
	/*
	* 获取测试用例
	* @param name 测试用例名称
	*/
	TestCase* GetTestCase(const std::string& name);
	/*
	* 开启/禁用 用例
	* @param const std::string & name 
	* @param bool able 
	*/
	void SetTestCase(const std::string& name, bool able);
	/*
	* 运行所有用例
	*/
	void RunAllTestCase();
	/*
	* 是否允许运行
	*/
	bool IsEnable();
	/*
	* 设置是否允许运行此用例合集
	* @param bool able 
	*/
	void SetRunable(bool able);
private:
	using TestCaseList = std::list<TestCase*>;
	TestCaseList case_list_;			// 测试用例列表
	std::string name_;					// 测试用例名称
	bool run_;							// 是否运行

	FixtureInitMethod init_;			// 初始化函数
	FixtureUninitMethod uninit_;	// 清理函数
};

#define FIXTURE_BEGIN(name) \
	namespace namespace_##name {	\
		Fixture *fixture = new Fixture(std::string(#name));	\
		bool fixture_add = UnitTestMgr.AddFixture(fixture);

#define FIXTURE_END(name) }

#define INIT(method) bool fixture_init_add = fixture->SetInitFunc(method);
#define UNINIT(method) bool fixture_uninit_add = fixture->SetUninitFunc(method);

#define CASE(name) \
	class case_##name : public TestCase{	\
	public:	\
		case_##name(const std::string& name) : TestCase(name){}	\
		virtual ~case_##name() = default;	\
		virtual void Run() override;	\
	};	\
	bool test_case_##name##_add = fixture->AddTestCase(new case_##name(#name));	\
	void case_##name::Run()
