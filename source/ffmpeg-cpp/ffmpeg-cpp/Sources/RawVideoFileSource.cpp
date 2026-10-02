#include "RawVideoFileSource.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	RawVideoFileSource::RawVideoFileSource(const char* fileName, FrameSink* frameSink)
	{
			demuxer = std::make_unique<Demuxer>(fileName, nullptr, nullptr);
		demuxer->DecodeBestVideoStream(frameSink);
	}

	RawVideoFileSource::~RawVideoFileSource()
	{
	}

	void RawVideoFileSource::PreparePipeline()
	{
		demuxer->PreparePipeline();
	}

	bool RawVideoFileSource::IsDone()
	{
		return demuxer->IsDone();
	}

	void RawVideoFileSource::Step()
	{
		demuxer->Step();
	}
}
