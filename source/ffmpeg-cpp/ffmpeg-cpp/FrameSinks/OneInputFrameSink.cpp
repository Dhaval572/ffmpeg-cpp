#include "OneInputFrameSink.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	OneInputFrameSink::OneInputFrameSink(FrameWriter* writer, AVMediaType mediaType)
	{
		this->writer = writer;
		this->mediaType = mediaType;
	}

	FrameSinkStream* OneInputFrameSink::CreateStream()
	{
		++nStreamsGenerated;
		if (nStreamsGenerated > 1)
		{
			throw FFmpegException("This frame sink only supports one input");
		}
		stream = std::make_unique<FrameSinkStream>(writer, 0);
		return stream.get();
	}

	OneInputFrameSink::~OneInputFrameSink()
	{
	}

	AVMediaType OneInputFrameSink::GetMediaType()
	{
		return mediaType;
	}
}
