#pragma once

#include "ffmpeg.h"
#include "InputStream.h"
#include "FrameSinks/AudioFrameSink.h"

namespace ffmpegcpp
{
	class AudioInputStream : public InputStream
	{

	public:

		AudioInputStream(AVFormatContext* format, AVStream* stream);
		~AudioInputStream() override;

		void AddStreamInfo(ContainerInfo* info) override;

	protected:

		void ConfigureCodecContext() override;
	};
}
