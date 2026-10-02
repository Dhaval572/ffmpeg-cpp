#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"
#include "FrameSinks/FrameSink.h"
#include "Demuxing/StreamData.h"
#include "InputSource.h"

#include <memory>

namespace ffmpegcpp
{
	class EncodedFileSource : public InputSource
	{

	public:
		EncodedFileSource(const char* inFileName, AVCodecID codecId, FrameSink* output);
		EncodedFileSource(const char* inFileName, const char* codecName, FrameSink* output);
		~EncodedFileSource() override;

		void PreparePipeline() override;
		bool IsDone() override;
		void Step() override;

	private:

		bool done = false;

		FrameSinkStream* output = nullptr;

		ParserContextPtr parser;

		const AVCodec* codec = nullptr;
		CodecContextPtr codecContext;

		int bufferSize = 0;

		FramePtr decoded_frame;
		PacketPtr pkt;
		std::unique_ptr<uint8_t[]> buffer;

		FilePtr file;

		void Init(const char* inFileName, const AVCodec* codec, FrameSink* output);

		void Decode(AVPacket* packet, AVFrame* targetFrame);

		std::unique_ptr<StreamData> metaData;
	};
}
