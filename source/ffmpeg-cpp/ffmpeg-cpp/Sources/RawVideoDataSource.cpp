#include "RawVideoDataSource.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	RawVideoDataSource::RawVideoDataSource(int width, int height, AVPixelFormat pixelFormat, int framesPerSecond, FrameSink* output)
		: RawVideoDataSource(width, height, pixelFormat, pixelFormat, framesPerSecond, output)
	{
	}

	RawVideoDataSource::RawVideoDataSource(int width, int height, AVPixelFormat sourcePixelFormat, AVPixelFormat targetPixelFormat, int framesPerSecond, FrameSink* output)
	{
		Init(width, height, sourcePixelFormat, targetPixelFormat, framesPerSecond, output);
	}

	void RawVideoDataSource::Init(int width, int height, AVPixelFormat sourcePixelFormat, AVPixelFormat targetPixelFormat, int framesPerSecond, FrameSink* output)
	{
		this->output = output->CreateStream();
		this->sourcePixelFormat = sourcePixelFormat;

		metaData.timeBase.num = 1;
		metaData.timeBase.den = framesPerSecond;
		metaData.frameRate.num = framesPerSecond;
		metaData.frameRate.den = 1;
		metaData.type = AVMEDIA_TYPE_VIDEO;

		frame.reset(av_frame_alloc());
		if (!frame)
		{
			throw FFmpegException("Could not allocate video frame");
		}

		frame->format = targetPixelFormat;
		frame->width = width;
		frame->height = height;

		int ret = av_frame_get_buffer(frame.get(), 32);
		if (ret < 0)
		{
			throw FFmpegException("Could not allocate the video frame data", ret);
		}
	}

	RawVideoDataSource::~RawVideoDataSource()
	{
	}

	void RawVideoDataSource::WriteFrame(void* data, int bytesPerRow)
	{
		int ret = av_frame_make_writable(frame.get());
		if (ret < 0)
		{
			throw FFmpegException("Error making frame writable", ret);
		}

		const int in_linesize[1] = { bytesPerRow };

		swsContext.reset(sws_getCachedContext(swsContext.get(),
			frame->width, frame->height, sourcePixelFormat,
			frame->width, frame->height, (AVPixelFormat)frame->format,
			0, 0, 0, 0));
		sws_scale(swsContext.get(), (const uint8_t* const*)&data, in_linesize, 0,
			frame->height, frame->data, frame->linesize);

		output->WriteFrame(frame.get(), &metaData);
	}

	void RawVideoDataSource::Close()
	{
		output->Close();
	}

	int RawVideoDataSource::GetWidth()
	{
		return frame->width;
	}

	int RawVideoDataSource::GetHeight()
	{
		return frame->height;
	}

	bool RawVideoDataSource::IsPrimed()
	{
		return output->IsPrimed();
	}
}
