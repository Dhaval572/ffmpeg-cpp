#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"
#include "FrameSinks/VideoFrameSink.h"
#include "Demuxing/StreamData.h"

#include <memory>

namespace ffmpegcpp
{
	class RawVideoDataSource
	{

	public:

		RawVideoDataSource(int width, int height, AVPixelFormat pixelFormat, int framesPerSecond, FrameSink* output);
		RawVideoDataSource(int width, int height, AVPixelFormat sourcePixelFormat, AVPixelFormat targetPixelFormat, int framesPerSecond, FrameSink* output);
		~RawVideoDataSource();

		void WriteFrame(void* data, int bytesPerRow);
		void Close();

		int GetWidth();
		int GetHeight();

		bool IsPrimed();

	private:

		void Init(int width, int height, AVPixelFormat sourcePixelFormat, AVPixelFormat targetPixelFormat, int framesPerSecond, FrameSink* output);

		AVPixelFormat sourcePixelFormat;

		FrameSinkStream* output;

		StreamData metaData;

		FramePtr frame;
		SwsContextPtr swsContext;
	};
}
