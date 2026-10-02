#include "Filter.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	Filter::Filter(const char* filterString, FrameSink* target)
	{
		this->targetMediaType = target->GetMediaType();
		this->target = target->CreateStream();
		this->filterString = filterString;
	}

	AVMediaType Filter::GetMediaType()
	{
		return targetMediaType;
	}

	Filter::~Filter()
	{
	}

	void Filter::ConfigureFilterGraph()
	{
		int ret;
		char args[512];

		filt_frame.reset(av_frame_alloc());
		if (!filt_frame)
		{
			throw FFmpegException("Could not allocate intermediate video frame for filter");
		}

		filter_graph.reset(avfilter_graph_alloc());
		if (!filter_graph)
		{
			throw FFmpegException("Failed to allocate filter graph");
		}

		std::string fullFilterString = "";

		AVFrame* frame;
		for (int i = (int)inputs.size() - 1; i >= 0; --i)
		{
			if (!inputs[i]->PeekFrame(&frame))
			{
				throw FFmpegException(std::string("No frame found for input ") + std::to_string(i));
			}

			StreamData* metaData = inputs[i]->GetMetaData();

			FillArguments(args, sizeof(args), frame, metaData);

			char bufferString[1000];
			snprintf(bufferString, sizeof(bufferString), "%s=%s [in_%d]; ", GetBufferName(metaData->type), args, i + 1);
			fullFilterString = bufferString + fullFilterString;
		}

		for (size_t i = 0; i < inputs.size(); ++i)
		{
			fullFilterString += "[in_" + std::to_string(i + 1) + "] ";
		}
		fullFilterString += filterString;
		fullFilterString += " [result]; [result] ";
		fullFilterString += GetBufferSinkName(targetMediaType);

		AVFilterInOut* gis = NULL;
		AVFilterInOut* gos = NULL;
		ret = avfilter_graph_parse2(filter_graph.get(), fullFilterString.c_str(), &gis, &gos);
		if (ret < 0)
		{
			avfilter_inout_free(&gis);
			avfilter_inout_free(&gos);
			throw FFmpegException("Failed to parse and generate filters", ret);
		}

		avfilter_inout_free(&gis);
		avfilter_inout_free(&gos);

		bufferSources.clear();
		for (unsigned int i = 0; i < filter_graph->nb_filters; ++i)
		{
			AVFilterContext* ctx = filter_graph->filters[i];
			if (ctx->nb_inputs == 0)
			{
				bufferSources.push_back(ctx);
			}
			if (ctx->nb_outputs == 0)
			{
				buffersink_ctx = ctx;
			}
		}

		if ((ret = avfilter_graph_config(filter_graph.get(), NULL)) < 0)
		{
			throw FFmpegException("Failed to configure filter graph", ret);
		}

		outputMetaData.timeBase = av_buffersink_get_time_base(buffersink_ctx);
		outputMetaData.frameRate = av_buffersink_get_frame_rate(buffersink_ctx);
		outputMetaData.type = targetMediaType;
	}

	void Filter::FillArguments(char* args, int argsLength, AVFrame* frame, StreamData* metaData)
	{
		if (metaData->type == AVMEDIA_TYPE_VIDEO)
		{
			snprintf(args, argsLength,
				"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d:frame_rate=%d/%d:colorspace=%d:range=%d",
				frame->width, frame->height, frame->format,
				metaData->timeBase.num, metaData->timeBase.den,
				frame->sample_aspect_ratio.num, frame->sample_aspect_ratio.den,
				metaData->frameRate.num, metaData->frameRate.den,
				frame->colorspace, frame->color_range);
		}
		else if (metaData->type == AVMEDIA_TYPE_AUDIO)
		{
			char layoutDesc[128] = {};
			if (frame->ch_layout.nb_channels > 0)
			{
				av_channel_layout_describe(&frame->ch_layout, layoutDesc, sizeof(layoutDesc));
			}
			else
			{
				AVChannelLayout def{};
				av_channel_layout_default(&def, 2);
				av_channel_layout_describe(&def, layoutDesc, sizeof(layoutDesc));
				av_channel_layout_uninit(&def);
			}
			snprintf(args, argsLength,
				"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%s",
				metaData->timeBase.num, metaData->timeBase.den, frame->sample_rate,
				av_get_sample_fmt_name((AVSampleFormat)frame->format), layoutDesc);
		}
		else
		{
			throw FFmpegException(std::string("Media type ") + av_get_media_type_string(metaData->type) + " is not supported by filters.");
		}
	}

	const char* Filter::GetBufferName(AVMediaType mediaType)
	{
		if (mediaType == AVMEDIA_TYPE_VIDEO) return "buffer";
		else if (mediaType == AVMEDIA_TYPE_AUDIO) return "abuffer";
		else throw FFmpegException(std::string("Media type ") + av_get_media_type_string(mediaType) + " is not supported by filters.");
	}

	const char* Filter::GetBufferSinkName(AVMediaType mediaType)
	{
		if (mediaType == AVMEDIA_TYPE_VIDEO) return "buffersink";
		else if (mediaType == AVMEDIA_TYPE_AUDIO) return "abuffersink";
		else throw FFmpegException(std::string("Media type ") + av_get_media_type_string(mediaType) + " is not supported by filters.");
	}

	void Filter::DrainInputQueues()
	{
		int ret;
		AVFrame* frame;
		for (size_t i = 0; i < inputs.size(); ++i)
		{
			while (inputs[i]->FetchFrame(&frame))
			{
				if ((ret = av_buffersrc_add_frame(bufferSources[i], frame)) < 0)
				{
					throw FFmpegException("Error feeding filter graph", ret);
				}
				av_frame_free(&frame);
			}
		}
	}

	FrameSinkStream* Filter::CreateStream()
	{
		auto input = std::make_unique<VideoFilterInput>();
		inputs.push_back(std::move(input));
		inputStreams.push_back(std::make_unique<FrameSinkStream>(this, (int)inputs.size() - 1));
		return inputStreams.back().get();
	}

	void Filter::WriteFrame(int streamIndex, AVFrame* frame, StreamData* metaData)
	{
		if (!initialized)
		{
			inputs[streamIndex]->SetMetaData(metaData);
			inputs[streamIndex]->WriteFrame(frame);

			bool allInputsHaveFrames = true;
			for (size_t i = 0; i < inputs.size(); ++i)
			{
				if (!inputs[i]->HasFrame())
				{
					allInputsHaveFrames = false;
				}
			}

			if (allInputsHaveFrames)
			{
				ConfigureFilterGraph();
				DrainInputQueues();
				initialized = true;
				PollFilterGraphForFrames();
			}
			return;
		}

		int ret;
		if ((ret = av_buffersrc_add_frame_flags(bufferSources[streamIndex], frame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0)
		{
			throw FFmpegException("Error feeding filter graph", ret);
		}

		PollFilterGraphForFrames();
	}

	void Filter::Close(int streamIndex)
	{
		if (!initialized) return;

		int ret;
		if ((ret = av_buffersrc_add_frame_flags(bufferSources[streamIndex], NULL, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0)
		{
			throw FFmpegException("Error flushing filter graph", ret);
		}
		PollFilterGraphForFrames();

		inputs[streamIndex]->Close();

		bool allClosed = true;
		for (size_t i = 0; i < inputs.size(); ++i)
		{
			if (!inputs[i]->IsClosed()) allClosed = false;
		}
		if (allClosed)
		{
			target->Close();
		}
	}

	void Filter::PollFilterGraphForFrames()
	{
		int ret = 0;
		while (ret >= 0)
		{
			ret = av_buffersink_get_frame(buffersink_ctx, filt_frame.get());
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				return;
			}
			else if (ret < 0)
			{
				throw FFmpegException("Error during filtering", ret);
			}

			target->WriteFrame(filt_frame.get(), &outputMetaData);

			av_frame_unref(filt_frame.get());
		}
	}

	bool Filter::IsPrimed()
	{
		return target->IsPrimed();
	}
}
