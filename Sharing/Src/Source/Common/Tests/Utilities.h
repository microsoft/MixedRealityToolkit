// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//------------------------------------------------------------------------------
// <copyright file="Utilities.h" company="Microsoft">
// </copyright>
//------------------------------------------------------------------------------

#pragma once

namespace MicrosoftTest
{
	void WaitForCompletion(std::function<bool()> hasCompleted, std::function<void()> update, DWORD timeout);

	void LogMessageFormat(const char *format, ...);
}
