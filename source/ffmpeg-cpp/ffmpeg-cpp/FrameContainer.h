#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"

namespace ffmpegcpp
{
	class FrameContainer
	{
	public:

		FrameContainer(AVFrame* frame, AVRational* timeBase);
		~FrameContainer();

		AVFrame* GetFrame();
		AVRational* GetTimeBase();

	private:

		FramePtr frame;
		AVRational* timeBase;
	};
}
