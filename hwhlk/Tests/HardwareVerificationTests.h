#pragma once

#include <WexTestClass.h>


class PictTests
{
	TEST_CLASS(PictTests);

	TEST_CLASS_SETUP(Setup);

	BEGIN_TEST_METHOD(Test)
		TEST_METHOD_PROPERTY(L"DataSource", L"pict:SimplePICT.txt")
	END_TEST_METHOD()

	TEST_CLASS_CLEANUP(Cleanup);

private:
	winrt::MicrosoftDisplayCaptureTools::Framework::ICore framework{ nullptr };
};