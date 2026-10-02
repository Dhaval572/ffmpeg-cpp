#pragma once

#include "Codec.h"
#include "OpenCodec.h"

namespace ffmpegcpp
{

	class VideoCodec : public Codec
	{
	public:

		VideoCodec(const char* codecName);
		VideoCodec(AVCodecID codecId);
		~VideoCodec() override;

		OpenCodec* Open(int width, int height, AVRational* frameRate, AVPixelFormat format);

		void SetQualityScale(int qscale);

		bool IsPixelFormatSupported(AVPixelFormat format);
		bool IsFrameRateSupported(AVRational* frameRate);

		AVPixelFormat GetDefaultPixelFormat();
		AVRational GetClosestSupportedFrameRate(AVRational frameRate);
	};
}
