#include "stdafx.h"
#include "ImageTagManagerImpl.h"
#include "TagImageImpl.h"

XTOOLS_NAMESPACE_BEGIN

ImageTagManagerPtr ImageTagManager::Create()
{
	return new ImageTagManagerImpl();
}

ImageTagManagerImpl::ImageTagManagerImpl()
	: m_tf(tag36h11_create())
	, m_event(false, false)
	, m_stopping(0)
{
}

ImageTagManagerImpl::~ImageTagManagerImpl()
{
	tag36h11_destroy(m_tf);

	if (m_workerThread)
	{
		if (m_locator)
		{
			m_locator->Cancel();
		}

		m_stopping = 1;
		m_event.Set();
		m_workerThread->WaitForThreadExit();
	}
}

void ImageTagManagerImpl::Update()
{
	if (m_locator != nullptr)
	{
		m_locator->Update();
		if (m_locator->IsComplete())
		{
			m_locator = nullptr;
		}
	}
}

bool ImageTagManagerImpl::FindTags(const byte* data, int32 pixelWidth, int32 pixelHeight, int32 bytesPerPixel, ImageTagLocationListener* locationCallback)
{
	if (m_locator != nullptr)
	{
		return false;
	}

	if (m_workerThread == nullptr)
	{
		m_workerThread = new MemberFuncThread(&ImageTagManagerImpl::FindTagsWorker, this);
	}

	m_locator = new ImageTagLocator(data, pixelWidth, pixelHeight, bytesPerPixel, locationCallback);
	m_event.Set();
	return true;
}

void ImageTagManagerImpl::FindTagsWorker()
{
	apriltag_family_t* tf(tag36h11_create());
	apriltag_detector_t* td(apriltag_detector_create());
	apriltag_detector_add_family(td, tf);

	td->quad_decimate = 1.0;
	td->quad_sigma = 0.0;
	td->nthreads = 1;
	td->debug = 0;
	td->refine_decode = 0;
	td->refine_pose = 0;

	while (!m_stopping)
	{
		if (m_event.Wait())
		{
			if (m_locator)
			{
				m_locator->FindTags(td);
			}
		}
	}

	apriltag_detector_destroy(td);
	tag36h11_destroy(tf);
}

TagImagePtr ImageTagManagerImpl::CreateTagImage(int32 tagId) const
{
	if (tagId < 0 || tagId >= (int32)m_tf->ncodes)
	{
		XTASSERT(false);
		return nullptr;
	}

	const uint64 code = m_tf->codes[tagId];

	const int32 imageCoreSize = m_tf->d;
	const int32 borderSize = m_tf->black_border;
	const int32 tagPixelSize = imageCoreSize + 2 * borderSize;
	const int32 imageEdge = imageCoreSize + borderSize;
	TagImageImplPtr tagImageImpl(new TagImageImpl(tagPixelSize, tagPixelSize));

	// Center 6x6 grid of the image is the real content of the image, the outer
	// border of the image is all black
	uint64 bitFlag = 1ULL << (imageCoreSize * imageCoreSize - 1);
	for (int32 y = borderSize; y < imageEdge; y++)
	{
		for (int32 x = borderSize; x < imageEdge; x++)
		{
			if (code & bitFlag)
			{
				tagImageImpl->SetPixel(x, y, 0xFF);
			}
			bitFlag >>= 1;
		}
	}

	return tagImageImpl;
}

XTOOLS_NAMESPACE_END