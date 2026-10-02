#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"

namespace ffmpegcpp
{
	class OpenCodec
	{
	public:

		explicit OpenCodec(CodecContextPtr openCodecContext);
		~OpenCodec();

		OpenCodec(const OpenCodec&) = delete;
		OpenCodec& operator=(const OpenCodec&) = delete;

		AVCodecContext* GetContext();

	private:

		CodecContextPtr context;
	};
}
