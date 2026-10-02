#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"

#include "InputSource.h"
#include "Demuxer.h"

#include <memory>

namespace ffmpegcpp
{
	class RawVideoFileSource : public InputSource
	{
	public:

		RawVideoFileSource(const char* fileName, FrameSink* frameSink);
		~RawVideoFileSource() override;

		void PreparePipeline() override;
		bool IsDone() override;
		void Step() override;

	private:

		std::unique_ptr<Demuxer> demuxer;
	};
}
