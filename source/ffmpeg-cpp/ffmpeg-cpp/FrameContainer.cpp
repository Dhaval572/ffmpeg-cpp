#include "FrameContainer.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	FrameContainer::FrameContainer(AVFrame* frame, AVRational* timeBase)
	{
		AVFrame* tmp = av_frame_clone(frame);
		if (!tmp) throw FFmpegException("Failed to clone frame");
		av_frame_unref(frame);
		this->frame.reset(tmp);
		this->timeBase = timeBase;
	}

	FrameContainer::~FrameContainer()
	{
	}

	AVFrame* FrameContainer::GetFrame()
	{
		return frame.get();
	}

	AVRational* FrameContainer::GetTimeBase()
	{
		return timeBase;
	}
}
