// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "stdafx.h"
#include "ImageTagLocationImpl.h"

XTOOLS_NAMESPACE_BEGIN

ImageTagLocationImpl::ImageTagLocationImpl(apriltag_detection_t* det)
	: m_det(det)
{
}

ImageTagLocationImpl::~ImageTagLocationImpl()
{
	apriltag_detection_destroy(m_det);
	if (m_homographToModelView)
	{
		matd_destroy(m_homographToModelView);
	}
}

int ImageTagLocationImpl::GetTagId()
{
	return m_det->id;
}

float ImageTagLocationImpl::GetPixelX(ImageTagLocationType locationType)
{
	switch (locationType)
	{
	case ImageTagLocationType::Center:
		return (float)m_det->c[0];
	case ImageTagLocationType::TopLeft:
		return (float)m_det->p[0][0];
	case ImageTagLocationType::BottomLeft:
		return (float)m_det->p[1][0];
	case ImageTagLocationType::BottomRight:
		return (float)m_det->p[2][0];
	case ImageTagLocationType::TopRight:
		return (float)m_det->p[3][0];
	default:
		return 0;
	}
}

float ImageTagLocationImpl::GetPixelY(ImageTagLocationType locationType)
{
	switch (locationType)
	{
	case ImageTagLocationType::Center:
		return (float)m_det->c[1];
	case ImageTagLocationType::TopLeft:
		return (float)m_det->p[0][1];
	case ImageTagLocationType::BottomLeft:
		return (float)m_det->p[1][1];
	case ImageTagLocationType::BottomRight:
		return (float)m_det->p[2][1];
	case ImageTagLocationType::TopRight:
		return (float)m_det->p[3][1];
	default:
		return 0;
	}
}

float ImageTagLocationImpl::GetModelViewMatrix(int32 row, int32 col, float F, float G, float A, float B, float C, float D)
{
	XT_UNREFERENCED_PARAM(C);
	XT_UNREFERENCED_PARAM(D);
	if (m_homographToModelView == nullptr)
	{
		m_homographToModelView = homography_to_pose(m_det->H, F, G, A, B);
	}

	return (float)MATD_EL(m_homographToModelView, row, col);
}

XTOOLS_NAMESPACE_END
