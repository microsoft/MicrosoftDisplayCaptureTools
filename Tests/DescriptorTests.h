#pragma once

#include "CaptureFrameworkTestBase.h"

/// <summary>
/// Validates basic EDID and DisplayID driver support.
/// </summary>
class DescriptorTests : CaptureFrameworkTestBase
{
    BEGIN_TEST_CLASS(DescriptorTests)
        TEST_CLASS_PROPERTY(L"", L"")
    END_TEST_CLASS()

    TEST_CLASS_SETUP(Setup);
    TEST_CLASS_CLEANUP(Cleanup);

public:
    BEGIN_TEST_METHOD(EdidManyBlocks)
        TEST_METHOD_PROPERTY(L"Descripton", L"Validates the driver's support for receiving many EDID blocks.")
    END_TEST_METHOD()

    BEGIN_TEST_METHOD(DisplayId2InEdid)
        TEST_METHOD_PROPERTY(L"Description", L"Validates the driver passes through DisplayID 2.0 blocks embedded in an EDID, correctly parses the native mode, and HDR support.")
    END_TEST_METHOD()
};
