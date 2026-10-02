#pragma once

#include "ffmpeg.h"
#include "InputStream.h"
#include "FrameSinks/VideoFrameSink.h"
#include "Info/VideoStreamInfo.h"

namespace ffmpegcpp
{
	class VideoInputStream : public InputStream
	{

	public:

		VideoInputStream(AVFormatContext* format, AVStream* stream);
		~VideoInputStream() override;

		void AddStreamInfo(ContainerInfo* info) override;

	protected:

		void ConfigureCodecContext() override;
	};
}
