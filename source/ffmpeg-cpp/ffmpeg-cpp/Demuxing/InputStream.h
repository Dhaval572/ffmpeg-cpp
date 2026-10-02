#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"
#include "FrameSinks/FrameSink.h"
#include "Info/ContainerInfo.h"
#include "Demuxing/StreamData.h"

#include <memory>

namespace ffmpegcpp
{
	class InputStream
	{

	public:

		InputStream(AVFormatContext* format, AVStream* stream);
		virtual ~InputStream();

		void Open(FrameSink* frameSink);

		virtual void DecodePacket(AVPacket* pkt);
		void Close();

		bool IsPrimed();
		int GetFramesProcessed();

		virtual void AddStreamInfo(ContainerInfo* info) = 0;

	protected:

		AVCodecContext* codecContext = nullptr;

		virtual void ConfigureCodecContext();

		AVFormatContext* format;
		AVStream* stream;

		float CalculateBitRate(AVCodecContext* ctx);

	private:

		FrameSinkStream* output = nullptr;

		FramePtr frame;

		std::unique_ptr<StreamData> metaData;

		StreamData* DiscoverMetaData();

		int nFramesProcessed = 0;
	};
}
