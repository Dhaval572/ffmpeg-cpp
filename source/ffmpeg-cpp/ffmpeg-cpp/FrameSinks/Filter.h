#pragma once

#include "ffmpeg.h"
#include "std.h"
#include "FfmpegDeleters.h"

#include "VideoFrameSink.h"
#include "VideoFilterInput.h"

#include <memory>

namespace ffmpegcpp
{
	class Filter : public FrameSink, public FrameWriter
	{

	public:

		Filter(const char* filterString, FrameSink* target);
		~Filter() override;

		FrameSinkStream* CreateStream() override;

		void WriteFrame(int streamIndex, AVFrame* frame, StreamData* metaData) override;
		void Close(int streamIndex) override;

		bool IsPrimed() override;

		AVMediaType GetMediaType() override;

	private:

		void ConfigureFilterGraph();
		void DrainInputQueues();
		void PollFilterGraphForFrames();
		void FillArguments(char* args, int argsLength, AVFrame* frame, StreamData* metaData);

		const char* GetBufferName(AVMediaType mediaType);
		const char* GetBufferSinkName(AVMediaType mediaType);

		std::vector<std::unique_ptr<VideoFilterInput>> inputs;
		std::vector<std::unique_ptr<FrameSinkStream>> inputStreams;
		std::vector<AVFilterContext*> bufferSources;

		AVMediaType targetMediaType;
		FrameSinkStream* target;

		std::string filterString;

		FilterGraphPtr filter_graph;
		AVFilterContext* buffersink_ctx = nullptr;
		FramePtr filt_frame;

		bool initialized = false;

		StreamData outputMetaData;
	};
}
