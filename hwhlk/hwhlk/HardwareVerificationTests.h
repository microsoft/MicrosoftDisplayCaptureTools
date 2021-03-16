#pragma once

#include <WexTestClass.h>
#include "winrt/Core.h"

class BaseTest {
    TEST_CLASS(BaseTest);

    TEST_CLASS_SETUP(BaseTestSetup)

    BEGIN_TEST_METHOD(Test)
        TEST_METHOD_PROPERTY(L"DataSource", L"pict:SimplePICT.txt")
    END_TEST_METHOD()
};