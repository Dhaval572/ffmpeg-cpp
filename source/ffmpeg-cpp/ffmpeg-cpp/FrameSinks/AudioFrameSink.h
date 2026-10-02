#pragma once

#include "FrameSink.h"

namespace ffmpegcpp
{
	class AudioFrameSink : public FrameSink
	{
	public:

		AVMediaType GetMediaType() override
		{
			return AVMEDIA_TYPE_AUDIO;
		}

		~AudioFrameSink() override {}
	};
}
