#pragma once

#include "FrameSink.h"

namespace ffmpegcpp
{
	class VideoFrameSink : public FrameSink
	{
	public:

		AVMediaType GetMediaType() override
		{
			return AVMEDIA_TYPE_VIDEO;
		}

		~VideoFrameSink() override {}
	};
}
