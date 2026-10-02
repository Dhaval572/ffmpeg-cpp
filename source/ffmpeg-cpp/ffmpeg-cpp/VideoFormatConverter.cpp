#include "VideoFormatConverter.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	VideoFormatConverter::VideoFormatConverter(AVCodecContext* codecContext)
	{
		this->codecContext = codecContext;

		converted_frame.reset(av_frame_alloc());
		if (!converted_frame)
		{
			throw FFmpegException("Error allocating a video frame");
		}

		converted_frame->format = codecContext->pix_fmt;
		converted_frame->width = codecContext->width;
		converted_frame->height = codecContext->height;

		int ret = av_frame_get_buffer(converted_frame.get(), 32);
		if (ret < 0)
		{
			throw FFmpegException("Failed to allocate buffer for frame", ret);
		}
	}

	VideoFormatConverter::~VideoFormatConverter()
	{
	}

	void VideoFormatConverter::InitDelayed(AVFrame* frame)
	{
		swsContext.reset(sws_getCachedContext(swsContext.get(),
			frame->width, frame->height, (AVPixelFormat)frame->format,
			converted_frame->width, converted_frame->height, (AVPixelFormat)converted_frame->format,
			0, 0, 0, 0));
	}

	AVFrame* VideoFormatConverter::ConvertFrame(AVFrame* frame)
	{
		if (!initialized)
		{
			InitDelayed(frame);
			initialized = true;
		}

		sws_scale(swsContext.get(), frame->data, frame->linesize, 0,
			frame->height, converted_frame->data, converted_frame->linesize);

		av_frame_copy_props(converted_frame.get(), frame);

		return converted_frame.get();
	}
}
