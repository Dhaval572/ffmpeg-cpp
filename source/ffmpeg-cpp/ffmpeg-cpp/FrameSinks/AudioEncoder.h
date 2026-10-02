#pragma once

#include "ffmpeg.h"
#include "std.h"
#include "FfmpegDeleters.h"

#include "AudioFrameSink.h"
#include "Codecs/AudioCodec.h"
#include "ConvertedAudioProcessor.h"
#include "AudioFormatConverter.h"
#include "Muxing/Muxer.h"
#include "Muxing/OutputStream.h"
#include "OneInputFrameSink.h"

#include <memory>

namespace ffmpegcpp
{
	class AudioEncoder : public AudioFrameSink, public ConvertedAudioProcessor, public FrameWriter
	{
	public:
		AudioEncoder(AudioCodec* codec, Muxer* muxer);
		AudioEncoder(AudioCodec* codec, Muxer* muxer, int bitRate);
		~AudioEncoder() override;

		FrameSinkStream* CreateStream() override;
		void WriteFrame(int streamIndex, AVFrame* frame, StreamData* metaData) override;
		void Close(int streamIndex) override;

		void WriteConvertedFrame(AVFrame* frame) override;

		bool IsPrimed() override;

	private:

		void OpenLazily(AVFrame* frame, StreamData* metaData);

		void PollCodecForPackets();

		OutputStream* output;

		AudioCodec* closedCodec;

		std::unique_ptr<AudioFormatConverter> formatConverter;
		std::unique_ptr<OpenCodec> codec;
		PacketPtr pkt;

		std::unique_ptr<OneInputFrameSink> oneInputFrameSink;

		int frameNumber = 0;

		int finalBitRate = -1;
	};
}
