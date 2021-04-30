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
	* ����
	* @param name ����
	*/
	Fixture(const std::string& name);
	/*
	* ����
	*/
	virtual ~Fixture();
	/*
	* ��ʼ��
	*/
	void Init();
	/*
	* ���ʼ��
	*/
	void UnInit();
	/*
	* ���ó�ʼ������
	* @param method ��ʼ������
	*/
	bool SetInitFunc(FixtureInitMethod method);
	/*
	* �������ʼ������
	* @param method ���ʼ������
	*/
	bool SetUninitFunc(FixtureUninitMethod method);
	/*
	* ��ȡ����
	*/
	const std::string& GetName()const;
	/*
	* ��Ӳ�������
	* @param testcase ��������ָ�� 
	*/
	bool AddTestCase(TestCase* testcase);
	/*
	* ��ȡ��������
	* @param name ������������
	*/
	TestCase* GetTestCase(const std::string& name);
	/*
	* ����/���� ����
	* @param const std::string & name 
	* @param bool able 
	*/
	void SetTestCase(const std::string& name, bool able);
	/*
	* ������������
	*/
	void RunAllTestCase();
	/*
	* �Ƿ���������
	*/
	bool IsEnable();
	/*
	* �����Ƿ��������д������ϼ�
	* @param bool able 
	*/
	void SetRunable(bool able);
private:
	using TestCaseList = std::list<TestCase*>;
	TestCaseList case_list_;			// ���������б�
	std::string name_;					// ������������
	bool run_;							// �Ƿ�����

	FixtureInitMethod init_;			// ��ʼ������
	FixtureUninitMethod uninit_;	// ������
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
		bool std_except_;	\
		bool except_;	\
	public:	\
		case_##name(const std::string& name) : TestCase(name), std_except_(false), except_(false){}	\
		virtual ~case_##name() = default;	\
		virtual void Run() override;	\
	};	\
	bool test_case_##name##_add = fixture->AddTestCase(new case_##name(#name));	\
	void case_##name::Run()
