//------------------------------------------------------------------------------
// <copyright file="Utilities.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

namespace MicrosoftTest
{
	void WaitForCompletion(std::function<bool()> hasCompleted, std::function<void()> update, DWORD timeout);

	void LogMessageFormat(const char *format, ...);
}