// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _NON_COPYABLE_H
#define _NON_COPYABLE_H

/* Macro version for those afraid of deriving from yet another class... */
#define DECLARE_NON_COPYABLE(_CLASSNAME) private:                                                   \
                                             _CLASSNAME(const _CLASSNAME &);                   \
                                             const _CLASSNAME & operator=(const _CLASSNAME &);
class NonCopyable_Z
{
protected:
	NonCopyable_Z() {}
	~NonCopyable_Z() {}

private:
	NonCopyable_Z(const NonCopyable_Z &);
	const NonCopyable_Z & operator=(const NonCopyable_Z &);
};

#endif
