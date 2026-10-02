#pragma once

#include "ffmpeg.h"
#include "std.h"
#include "FfmpegDeleters.h"

#include "Demuxing/AudioInputStream.h"
#include "Demuxing/VideoInputStream.h"
#include "Demuxing/InputStream.h"
#include "Sources/InputSource.h"
#include "Info/ContainerInfo.h"

#include <memory>
#include <vector>

namespace ffmpegcpp
{
	class Demuxer : public InputSource
	{
	public:

		explicit Demuxer(const char* fileName);
		Demuxer(const char* fileName, const AVInputFormat* inputFormat, AVDictionary* inputFormatOptions);
		~Demuxer() override;

		void DecodeBestAudioStream(FrameSink* frameSink);
		void DecodeBestVideoStream(FrameSink* frameSink);

		void DecodeAudioStream(int streamId, FrameSink* frameSink);
		void DecodeVideoStream(int streamId, FrameSink* frameSink);

		void PreparePipeline() override;
		bool IsDone() override;
		void Step() override;

		ContainerInfo GetInfo();
		int GetFrameCount(int streamId);

		const char* GetFileName();

	private:

		bool done = false;

		std::string fileName;

		InputStream* GetInputStream(int index);
		InputStream* GetInputStreamById(int streamId);

		std::vector<std::unique_ptr<InputStream>> inputStreams;

		FormatContextPtr containerContext;
		PacketPtr pkt;

		void DecodePacket();
	};
}
