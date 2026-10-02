#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"
#include "FrameSinks/AudioFrameSink.h"
#include "Demuxing/StreamData.h"

#include <memory>

namespace ffmpegcpp
{
	class RawAudioDataSource
	{

	public:

		RawAudioDataSource(AVSampleFormat sampleFormat, int sampleRate, int channels, FrameSink* output);
		RawAudioDataSource(AVSampleFormat sampleFormat, int sampleRate, int channels, int64_t channelLayout, FrameSink* output);
		~RawAudioDataSource();

		void WriteData(void* data, int sampleCount);
		void Close();

		bool IsPrimed();

	private:

		void CleanUp();

		FrameSinkStream* output;

		FramePtr frame;

		std::unique_ptr<StreamData> metaData;
	};
}
