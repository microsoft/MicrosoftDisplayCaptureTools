#pragma once

#include "CaptureFrameworkTestBase.h"

class SingleScreenTestMatrix : public CaptureFrameworkTestBase
{
    TEST_CLASS(SingleScreenTestMatrix);

    TEST_CLASS_SETUP(Setup);
    TEST_CLASS_CLEANUP(Cleanup);

    BEGIN_TEST_METHOD(Test)
        TEST_METHOD_PROPERTY(L"DataSource", L"Table:CaptureCardInputs.xml#Inputs")
        TEST_METHOD_PROPERTY(L"DataSource", L"pict:SimplePICT.txt")
    END_TEST_METHOD();

private:
    void SaveOutput(winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet data, winrt::hstring name);
    std::vector<winrt::Windows::Foundation::IAsyncAction> fileOperationsVector;
};