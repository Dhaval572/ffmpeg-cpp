#include "InputStream.h"
#include "CodecDeducer.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	InputStream::InputStream(AVFormatContext* format, AVStream* stream)
	{
		this->stream = stream;
		this->format = format;

		const AVCodec* codec = CodecDeducer::DeduceDecoder(stream->codecpar->codec_id);
		if (!codec)
		{
			throw FFmpegException("Failed to find codec for stream " + std::to_string(stream->index));
		}

		codecContext = avcodec_alloc_context3(codec);
		if (!codecContext)
		{
			throw FFmpegException("Failed to allocate the codec context for " + std::string(codec->name));
		}

		codecContext->framerate = stream->avg_frame_rate;

		int ret;
		if ((ret = avcodec_parameters_to_context(codecContext, stream->codecpar)) < 0)
		{
			avcodec_free_context(&codecContext);
			throw FFmpegException("Failed to copy " + std::string(codec->name) + " codec parameters to decoder context", ret);
		}

		ConfigureCodecContext();

		if ((ret = avcodec_open2(codecContext, codec, NULL)) < 0)
		{
			avcodec_free_context(&codecContext);
			throw FFmpegException("Failed to open codec " + std::string(codec->name), ret);
		}

		frame.reset(av_frame_alloc());
		if (!frame)
		{
			avcodec_free_context(&codecContext);
			throw FFmpegException("Could not allocate frame");
		}
	}

	InputStream::~InputStream()
	{
		if (codecContext != nullptr)
		{
			avcodec_free_context(&codecContext);
			codecContext = nullptr;
		}
	}

	void InputStream::ConfigureCodecContext()
	{
	}

	void InputStream::Open(FrameSink* frameSink)
	{
		output = frameSink->CreateStream();
	}

	StreamData* InputStream::DiscoverMetaData()
	{
		AVRational tb = stream->time_base;
		AVRational fr = av_guess_frame_rate(format, stream, NULL);

		metaData = std::make_unique<StreamData>();
		metaData->timeBase = tb;
		metaData->frameRate = fr;
		metaData->type = codecContext->codec->type;

		return metaData.get();
	}

	void InputStream::DecodePacket(AVPacket* pkt)
	{
		int ret;

		ret = avcodec_send_packet(codecContext, pkt);
		if (ret < 0)
		{
			throw FFmpegException("Error submitting the packet to the decoder", ret);
		}

		while (ret >= 0)
		{
			ret = avcodec_receive_frame(codecContext, frame.get());
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
			{
				throw FFmpegException("Error during decoding", ret);
			}

			if (!frame->sample_aspect_ratio.num)
			{
				frame->sample_aspect_ratio = stream->sample_aspect_ratio;
			}

			if (metaData == nullptr)
			{
				metaData = std::make_unique<StreamData>();
				DiscoverMetaData();
			}

			if (output != nullptr)
			{
				output->WriteFrame(frame.get(), metaData.get());
			}
			++nFramesProcessed;
		}
	}

	int InputStream::GetFramesProcessed()
	{
		return nFramesProcessed;
	}

	void InputStream::Close()
	{
		if (output != nullptr) output->Close();
	}

	bool InputStream::IsPrimed()
	{
		return output != nullptr && output->IsPrimed();
	}

	float InputStream::CalculateBitRate(AVCodecContext* ctx)
	{
		int64_t bit_rate;
		int bits_per_sample;

		switch (ctx->codec_type)
		{
		case AVMEDIA_TYPE_VIDEO:
		case AVMEDIA_TYPE_DATA:
		case AVMEDIA_TYPE_SUBTITLE:
		case AVMEDIA_TYPE_ATTACHMENT:
			bit_rate = ctx->bit_rate;
			break;
		case AVMEDIA_TYPE_AUDIO:
			bits_per_sample = av_get_bits_per_sample(ctx->codec_id);
			bit_rate = bits_per_sample ? ctx->sample_rate * (int64_t)ctx->ch_layout.nb_channels * bits_per_sample : ctx->bit_rate;
			break;
		default:
			bit_rate = 0;
			break;
		}
		return bit_rate / 1000.0f;
	}
}
