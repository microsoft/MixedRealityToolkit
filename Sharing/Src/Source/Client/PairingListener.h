//////////////////////////////////////////////////////////////////////////
// PairingListener.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// This allows C++ calls to these functions to cause the Java or C# code overloading the function to execute
#if defined(SWIG)
%feature("director") PairingListener;
#endif

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

/// Base class for objects that wish to receive callbacks when pairing attempts succeed or fail.  
class PairingListener : public Listener
{
public:
	/// Called when a pairing connection attempt succeeds
	virtual void PairingConnectionSucceeded() XTABSTRACT {}

	/// Called when a pairing connection attempt fails
	/// \param reason The enum representing identifying why the pairing attempt failed
	virtual void PairingConnectionFailed(PairingResult reason) XTABSTRACT {}
};

#pragma warning( pop )

XTOOLS_NAMESPACE_END