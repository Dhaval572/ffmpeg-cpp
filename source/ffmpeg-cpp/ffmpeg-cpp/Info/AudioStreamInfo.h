#pragma once

#include "ffmpeg.h"

#include <cstring>

namespace ffmpegcpp
{
	struct AudioStreamInfo
	{
		int id = 0;
		AVRational timeBase = {0, 1};
		const AVCodec* codec = nullptr;
		float bitRate = 0;

		int sampleRate = 0;
		int channels = 0;

		uint64_t channelLayout = 0;
		char channelLayoutName[255] = {};
	};
}
