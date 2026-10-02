#pragma once

#include "ffmpeg.h"

#include "FrameWriter.h"
#include "FrameSink.h"

#include <memory>

namespace ffmpegcpp
{
	class OneInputFrameSink : public FrameSink
	{
	public:

		OneInputFrameSink(FrameWriter* writer, AVMediaType mediaType);
		~OneInputFrameSink() override;

		AVMediaType GetMediaType() override;

		FrameSinkStream* CreateStream() override;

	private:

		int nStreamsGenerated = 0;

		FrameWriter* writer;

		std::unique_ptr<FrameSinkStream> stream;

		AVMediaType mediaType;
	};
}
