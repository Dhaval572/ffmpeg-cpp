#pragma once

#include "ffmpeg.h"

namespace ffmpegcpp
{
	struct StreamData
	{
		AVMediaType type = AVMEDIA_TYPE_UNKNOWN;

		AVRational timeBase = {0, 1};
		AVRational frameRate = {0, 1};
	};
}
