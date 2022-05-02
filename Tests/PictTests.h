#pragma once

#include <WexTestClass.h>
#include "CaptureFrameworkTestBase.h"

class PictTests : public CaptureFrameworkTestBase
{
	TEST_CLASS(PictTests);

	TEST_CLASS_SETUP(Setup);
    TEST_CLASS_CLEANUP(Cleanup);

	BEGIN_TEST_METHOD(Test)
		TEST_METHOD_PROPERTY(L"DataSource", L"pict:SimplePICT.txt")
    END_TEST_METHOD();
};