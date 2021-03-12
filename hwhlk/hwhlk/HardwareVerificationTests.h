#pragma once

#include <WexTestClass.h>

class BaseTest {
    TEST_CLASS(BaseTest);

    TEST_CLASS_SETUP(BaseTestSetup)
    TEST_CLASS_CLEANUP(BaseTestCleanup)

    BEGIN_TEST_METHOD(Test)
        TEST_METHOD_PROPERTY(L"DataSource", L"pict:SimplePICT.txt")
    END_TEST_METHOD()
};