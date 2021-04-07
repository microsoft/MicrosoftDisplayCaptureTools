#include "pch.h"
#include "winrt\TestFramework.h"

using namespace winrt;
using namespace Windows::Foundation;

int main()
{
	TestFramework::Framework framework(L"");
	TestFramework::TestRequirements testRequirements{ true, 0.0f, TestFramework::RequiredDDIs::None };
	auto testEnvironment = framework.CreateTestEnvironment(testRequirements);

	//testEnvironment.Run();
}
