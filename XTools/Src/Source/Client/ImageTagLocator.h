#pragma once

#include <Private/Utils/TypedLFQueue.h>

XTOOLS_NAMESPACE_BEGIN


class ImageTagLocator : public AtomicRefCounted
{
public:
	ImageTagLocator(const byte* data, int32 pixelWidth, int32 pixelHeight, int32 bytesPerPixel, ImageTagLocationListener* locationCallback);
	virtual ~ImageTagLocator();

	void Cancel();
	void Update();
	bool IsComplete() const { return m_isComplete; }
	void FindTags(apriltag_detector_t* td);

private:
	image_u8* CreateImage() const;
	byte GetMonochromePixel(int32 x, int32 y) const;

	typedef ListenerList<ImageTagLocationListener> ListenerList;
	DECLARE_PTR(ListenerList);
	
	std::vector<byte> m_imageData;
	int32 m_imageWidth;
	int32 m_imageHeight;
	int32 m_bytesPerPixel;
	ListenerListPtr m_callbackList;

	TypedLFQueue<ImageTagLocationPtr> m_outputQueue;
	bool m_isComplete;
	volatile int m_isStopping;
};

DECLARE_PTR(ImageTagLocator)

XTOOLS_NAMESPACE_END