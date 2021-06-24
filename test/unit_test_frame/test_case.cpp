#include "test_case.h"
#include <string>

TestCase::TestCase(const std::string& name)
	:name_(name), run_(true)
{

}

TestCase::~TestCase()
{

}

const std::string& TestCase::GetName() const
{
	return name_;
}

void TestCase::SetNeedRun(bool need_run)
{
	run_ = need_run;
}

bool TestCase::IsEnable()
{
	return run_;
}

bool TestCase::IsSuccess()
{
	return error_list_.empty();
}

void TestCase::SetError(const std::string& error)
{
	error_list_.emplace_back(error);
}

const ErrorList& TestCase::GetErrorList() const
{
	return error_list_;
}

void TestCase::ClearError()
{
	error_list_.clear();
}

