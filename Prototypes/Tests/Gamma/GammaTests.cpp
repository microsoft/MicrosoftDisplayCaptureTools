/*
* Copyright (c) Microsoft Corporation. All rights reserved.
*
* This File was generated using the VisualTAEF C++ Project Wizard.
* Class Name: GammaTests
*/
#include "StdAfx.h"
#include <WexTestClass.h>
#include <winrt/TestFramework.h>

using namespace WEX::Common;

using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace GammaTests
{
    class GammaTests
    {
        std::shared_ptr<winrt::TestFramework::Framework> framework;

        TEST_CLASS(GammaTests)

        TEST_CLASS_SETUP(GammaTestsSetup)
        {
            String configurationPath;
            RuntimeParameters::TryGetValue(L"ConfigPath", configurationPath);
            
            framework = std::make_shared<winrt::TestFramework::Framework>();
            //VERIFY_IS_TRUE((bool)framework);

            return true;
        }

        TEST_METHOD(CSCMatrixTest)
        {
            BEGIN_TEST_METHOD_PROPERTIES()
                TEST_METHOD_PROPERTY(L"Data:AdvancedColorMode", L"{SDR, HDR}")
            END_TEST_METHOD_PROPERTIES()


            framework->Initialize(winrt::hstring(L"blah"));

            String advancedColorMode;
            VERIFY_SUCCEEDED(TestData::TryGetValue(L"AdvancedColorMode", advancedColorMode));
            bool useHDR = advancedColorMode == L"HDR";

            // auto reqs = Framework::TestRequirements(
            //        Framework::TestRequirements::SOFTWARE_COMPARE | Framework::TestRequirements::MATRIX_3X4,
            //        5.0f, // Comparison tolerance, in percentage different from software rendering
            //        useHDR
            //        );

            // HRESULT setupNewTest = framework->SetupNewTest(reqs);
            // if(setupNewTest == ERROR_DEVICE_BUSY)
            // {
            //    Log::Result(TestResults::Blocked, L"Device is busy, check to make sure tests are not running in parallel.\n");
            //    return;
            // }
            // else if (FAILED(setupNewTest))
            // {
            //    Log::Result(TestResults::Skipped, L"Device doesn't support the requirements for this test\n");
            //    return;
            // }

            // auto matrix = Framework::OSOverrides::Matrix(0, 1, 0,
            //                                              1, 0, 0,
            //                                              0, 0, 1);

            // 
            // VERIFY_SUCCEEDED(framework->GetOSOverrides()->SetMatrix(matrix));
            // VERIFY_SUCCEEDED(framework->CompareResultsAgainstSoftware());
        }
    };
} /* namespace GammaTests */ 