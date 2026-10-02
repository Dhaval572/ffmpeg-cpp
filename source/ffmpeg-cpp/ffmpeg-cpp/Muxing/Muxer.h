#pragma once

#include "ffmpeg.h"
#include "std.h"
#include "FfmpegDeleters.h"

namespace ffmpegcpp {

	class OutputStream;

	class Muxer
	{
	public:

		explicit Muxer(const char* fileName);
		~Muxer();

		Muxer(const Muxer&) = delete;
		Muxer& operator=(const Muxer&) = delete;

		void AddOutputStream(OutputStream* stream);

		void WritePacket(AVPacket* pkt);

		void Close();

		bool IsPrimed();

		const AVCodec* GetDefaultVideoFormat();
		const AVCodec* GetDefaultAudioFormat();

	private:

		void Open();

		std::vector<OutputStream*> outputStreams;

		const AVOutputFormat* containerFormat;

		FormatOutputContextPtr containerContext;

		std::string fileName;

		bool opened = false;
	};
}
