#pragma once

#include "ffmpeg.h"
#include "std.h"
#include "FfmpegDeleters.h"

#include "VideoFrameSink.h"
#include "Codecs/VideoCodec.h"
#include "VideoFormatConverter.h"
#include "Muxing/Muxer.h"
#include "OneInputFrameSink.h"

#include <memory>

namespace ffmpegcpp
{
	class VideoEncoder : public VideoFrameSink, public FrameWriter
	{
	public:
		VideoEncoder(VideoCodec* codec, Muxer* muxer);
		VideoEncoder(VideoCodec* codec, Muxer* muxer, AVPixelFormat format);
		VideoEncoder(VideoCodec* codec, Muxer* muxer, AVRational frameRate);
		VideoEncoder(VideoCodec* codec, Muxer* muxer, AVRational frameRate, AVPixelFormat format);
		~VideoEncoder() override;

		FrameSinkStream* CreateStream() override;

		void WriteFrame(int streamIndex, AVFrame* frame, StreamData* metaData) override;
		void Close(int streamIndex) override;

		bool IsPrimed() override;

	private:

		void OpenLazily(AVFrame* frame, StreamData* metaData);
		void PollCodecForPackets();

		VideoCodec* closedCodec;
		OutputStream* output;

		std::unique_ptr<VideoFormatConverter> formatConverter;
		std::unique_ptr<OpenCodec> codec;
		PacketPtr pkt;

		std::unique_ptr<OneInputFrameSink> oneInputFrameSink;

		int frameNumber = 0;

		AVPixelFormat finalPixelFormat = AV_PIX_FMT_NONE;

		AVRational finalFrameRate;
		bool finalFrameRateSet = false;
	};
}
