//----------------------------------------------------------------------------------------------------------------------
/// \file
/// <summary>API and data types for interacting with TAEF during Reboot and Disconnect scenarios.</summary>
// Copyright (c) Microsoft Corporation.  All Rights Reserved.
//----------------------------------------------------------------------------------------------------------------------
#pragma once
#include "TE.Common.h"

namespace WEX { namespace TestExecution
{
    namespace RebootOption
    {
        enum Option
        {
            Rerun = 0x0,
            Continue = 0x1,
            // When running under DES or a TestMD, the test infrastructure waits for the OS shell to fully initialize
            // before launching TE.exe to have it run the remaining tests. The AvoidWaitingForShellInitialization option
            // can be used to have TAEF tell that test infrastructure to avoid this wait. This can be useful for tests that
            // need to run during OOBE. This currently only applies when running within a DES test pass or within a TestMD.
            AvoidWaitingForShellInitialization = 0x2,
        };
        inline Option operator|(Option option1, Option option2)
        {
            return static_cast<Option>(static_cast<unsigned int>(option1) | static_cast<unsigned int>(option2));
        }
        inline Option& operator|=(Option& option1, Option option2)
        {
            return (option1 = option1 | option2);
        }
    }/* namespace RebootOption */

    namespace Interruption
    {
#if !defined(__cplusplus_cli)
        namespace Private
        {
            // This is for future usage.
            enum class DisconnectOption
            {
                Default = 0,
            };

            extern "C" TECOMMON_API void __stdcall Interruption_Disconnect(DisconnectOption);
        }/* namespace Private */
        
        inline void Disconnect() { Private::Interruption_Disconnect(Private::DisconnectOption::Default); }
#endif

        TECOMMON_API void __stdcall Reboot(RebootOption::Option option);
        TECOMMON_API void __stdcall RebootCustom(RebootOption::Option option);
    }/* namespace Interruption */
}/* namespace TestExecution */}/* namespace WEX */
