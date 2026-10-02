#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"
#include "ConvertedAudioProcessor.h"

#include <memory>

namespace ffmpegcpp
{
	class AudioFormatConverter
	{
	public:

		AudioFormatConverter(ConvertedAudioProcessor* output, AVCodecContext* codecContext);
		~AudioFormatConverter();

		void ProcessFrame(AVFrame* frame);

	private:

		void InitDelayed(AVFrame* frame);

		void AddToFifo(AVFrame* frame);
		void PullConvertedFrameFromFifo();

		void WriteCompleteConvertedFrame();

		ConvertedAudioProcessor* output;

		AVCodecContext* codecContext;

		bool initialized = false;

		AudioFifoPtr fifo;
		FramePtr tmp_frame;
		FramePtr converted_frame;
		SwrContextPtr swr_ctx;

		int in_sample_rate = 0, out_sample_rate = 0;

		int samples_count = 0;
	};
}
