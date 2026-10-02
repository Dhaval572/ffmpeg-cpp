#pragma once

#include "ffmpeg.h"

namespace ffmpegcpp
{
	struct VideoStreamInfo
	{
		int id = 0;
		AVRational frameRate = {0, 1};
		AVRational timeBase = {0, 1};
		const AVCodec* codec = nullptr;
		float bitRate = 0;

		AVPixelFormat format = AV_PIX_FMT_NONE;
		const char* formatName = nullptr;

		int width = 0, height = 0;
	};
}
