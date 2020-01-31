//----------------------------------------------------------------------------------------------------------------------
/// \file
/// <summary>WexCommon assert implementation.</summary>
// Copyright (c) Microsoft Corporation.  All Rights Reserved.
//----------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Wex.Common.h"
#include "WexDebug.h"

#ifndef WEX_ASSERT
#ifdef NDEBUG

#define WEX_ASSERT(___condition, ___message)

#else

#define WEX_ASSERT(___condition, ___message) (void)( \
        (!!(___condition)) || \
        (WEX::Common::Debug::Assert(false, (L#___condition), (___message), __WFILE__, __WFUNCTION__, __LINE__), 0) \
    )

#endif /* NDEBUG */
#endif /* WEX_ASSERT */

