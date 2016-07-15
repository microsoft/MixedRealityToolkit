// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ImageTagLocationImpl : public ImageTagLocation
{
public:
	ImageTagLocationImpl(apriltag_detection_t* det);
	virtual ~ImageTagLocationImpl();

	virtual int GetTagId() XTOVERRIDE;
	virtual float GetPixelX(ImageTagLocationType locationType) XTOVERRIDE;
	virtual float GetPixelY(ImageTagLocationType locationType) XTOVERRIDE;
	virtual float GetModelViewMatrix(int32 row, int32 col, float F, float G, float A, float B, float C, float D) XTOVERRIDE;

private:
	apriltag_detection_t* m_det;
	matd_t* m_homographToModelView;
};

XTOOLS_NAMESPACE_END
