#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"

#include <memory>

namespace ffmpegcpp
{
	class VideoFormatConverter
	{
	public:

		explicit VideoFormatConverter(AVCodecContext* codecContext);
		~VideoFormatConverter();

		AVFrame* ConvertFrame(AVFrame* frame);

	private:

		void InitDelayed(AVFrame* frame);

		AVCodecContext* codecContext;

		bool initialized = false;

		FramePtr converted_frame;
		SwsContextPtr swsContext;
	};
}
