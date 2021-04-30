#pragma once
#include <list>
#include <string>

using ErrorList = std::list<std::string>;

/*
* ��������
*/
class TestCase
{
public:
	/*
	* ����
	* @param name ������������
	*/
	TestCase(const std::string& name);
	/*
	* ����
	*/
	virtual ~TestCase();
	/*
	* ���в�������
	*/
	virtual void Run() = 0;
	/*
	* ��ȡ��������
	*/
	const std::string& GetName() const;
	/*
	* ���ò�������
	* @param need_run �Ƿ���Ҫ����
	*/
	void SetNeedRun(bool need_run);
	/*
	*  �Ƿ���
	*/
	bool IsEnable();
	/*
	*  �Ƿ�ȫ���ɹ�����
	*/
	bool IsSuccess();
	/*
	*  ���ô�����Ϣ
	*  @param error ������Ϣ
	*/
	void SetError(const std::string& error);
	/*
	* ȡ�ô����б�
	*/
	const ErrorList& GetErrorList() const;
	/*
	* �������
	*/
	void ClearError();

private:
	std::string name_;						// ������������
	bool run_;								// �Ƿ���Ҫ����
	ErrorList error_list_;		// ���β��Դ�����Ϣ
};