#pragma once

#include "ffmpeg.h"
#include "FfmpegDeleters.h"
#include "Demuxing/StreamData.h"

#include <memory>

namespace ffmpegcpp
{
	class VideoFilterInput
	{

	public:

		VideoFilterInput();
		~VideoFilterInput();

		void WriteFrame(AVFrame* frame);

		bool HasFrame();
		bool IsClosed();
		bool FetchFrame(AVFrame** frame);
		bool PeekFrame(AVFrame** frame);

		void SetMetaData(StreamData* metaData);
		StreamData* GetMetaData();

		void Close();

	private:

		FifoPtr frame_queue;
		StreamData* metaData = nullptr;

		bool frameReceived = false;
		bool closed = false;
	};
}
