#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"

#include "InputSource.h"
#include "Demuxer.h"

#include <memory>

namespace ffmpegcpp
{
	class RawAudioFileSource : public InputSource
	{
	public:

		RawAudioFileSource(const char* fileName, const char* inputFormat, int sampleRate, int channels, FrameSink* frameSink);
		~RawAudioFileSource() override;

		void PreparePipeline() override;
		bool IsDone() override;
		void Step() override;

	private:

		std::unique_ptr<Demuxer> demuxer;
	};
}
