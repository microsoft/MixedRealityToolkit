//////////////////////////////////////////////////////////////////////////
// ImageTagLocationListener.h
//
// Callback to receive notifications when an AR tag is found within an
// image.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef SWIG
%feature("director") XTools::ImageTagLocationListener;
#endif

#include "ImageTagLocation.h"

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XTOOLS_NAMESPACE_BEGIN

class ImageTagLocationListener XTABSTRACT : public Listener
{
public:
	virtual ~ImageTagLocationListener() {}

	// Called when a tag is located within the image.  This callback occurs on the thread
	// that ImageTagManager.Update is invoked on.
	virtual void OnTagLocated(const ImageTagLocationPtr& location) {}

	// Called when a tag search has completed, and all found tags
	// have been processed.  This callback occurs on the thread
	// that ImageTagManager.Update is invoked on.
	virtual void OnTagLocatingCompleted() {}
};

XTOOLS_NAMESPACE_END

#pragma warning( pop )