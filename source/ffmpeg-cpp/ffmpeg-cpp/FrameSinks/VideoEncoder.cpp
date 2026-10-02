#include "VideoEncoder.h"

#include "FFmpegException.h"
#include "Muxing/VideoOutputStream.h"

namespace ffmpegcpp
{
	VideoEncoder::VideoEncoder(VideoCodec* codec, Muxer* muxer)
	{
		this->closedCodec = codec;

		this->output = new VideoOutputStream(muxer, codec);
		muxer->AddOutputStream(output);

		oneInputFrameSink = std::make_unique<OneInputFrameSink>(this, AVMEDIA_TYPE_VIDEO);
	}

	VideoEncoder::VideoEncoder(VideoCodec* codec, Muxer* muxer, AVPixelFormat format)
		: VideoEncoder(codec, muxer)
	{
		finalPixelFormat = format;
	}

	VideoEncoder::VideoEncoder(VideoCodec* codec, Muxer* muxer, AVRational frameRate)
		: VideoEncoder(codec, muxer)
	{
		finalFrameRate = frameRate;
		finalFrameRateSet = true;
	}

	VideoEncoder::VideoEncoder(VideoCodec* codec, Muxer* muxer, AVRational frameRate, AVPixelFormat format)
		: VideoEncoder(codec, muxer)
	{
		finalPixelFormat = format;
		finalFrameRate = frameRate;
		finalFrameRateSet = true;
	}

	VideoEncoder::~VideoEncoder()
	{
	}

	void VideoEncoder::OpenLazily(AVFrame* frame, StreamData* metaData)
	{
		int width = frame->width;
		int height = frame->height;

		AVPixelFormat format = finalPixelFormat;
		if (format == AV_PIX_FMT_NONE) format = (AVPixelFormat)frame->format;
		if (!closedCodec->IsPixelFormatSupported(format)) format = closedCodec->GetDefaultPixelFormat();

		AVRational frameRate = metaData->frameRate;
		if (!closedCodec->IsFrameRateSupported(&frameRate)) frameRate = closedCodec->GetClosestSupportedFrameRate(frameRate);
		if (finalFrameRateSet) frameRate = finalFrameRate;

		codec.reset(closedCodec->Open(width, height, &frameRate, format));

		pkt.reset(av_packet_alloc());
		if (!pkt)
		{
			throw FFmpegException("Failed to allocate packet");
		}

		formatConverter = std::make_unique<VideoFormatConverter>(codec->GetContext());
	}

	FrameSinkStream* VideoEncoder::CreateStream()
	{
		return oneInputFrameSink->CreateStream();
	}

	void VideoEncoder::WriteFrame(int streamIndex, AVFrame* frame, StreamData* metaData)
	{
		if (!codec)
		{
			OpenLazily(frame, metaData);
		}

		frame = formatConverter->ConvertFrame(frame);

		if (frame->format != codec->GetContext()->pix_fmt)
		{
			throw FFmpegException("Codec only accepts " + std::string(av_get_pix_fmt_name(codec->GetContext()->pix_fmt)) + " while frame is in format " + av_get_pix_fmt_name((AVPixelFormat)frame->format));
		}

		frame->pts = frameNumber;
		++frameNumber;

		int ret = avcodec_send_frame(codec->GetContext(), frame);
		if (ret < 0)
		{
			throw FFmpegException("Error sending a frame for encoding", ret);
		}

		PollCodecForPackets();
	}

	void VideoEncoder::Close(int streamIndex)
	{
		if (!codec) return;

		int ret = avcodec_send_frame(codec->GetContext(), NULL);
		if (ret < 0)
		{
			throw FFmpegException("Error flushing codec after encoding", ret);
		}

		PollCodecForPackets();
	}

	void VideoEncoder::PollCodecForPackets()
	{
		int ret = 0;
		while (ret >= 0)
		{
			ret = avcodec_receive_packet(codec->GetContext(), pkt.get());
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				return;
			}
			else if (ret < 0)
			{
				throw FFmpegException("Error during encoding", ret);
			}

			output->WritePacket(pkt.get(), codec.get());

			av_packet_unref(pkt.get());
		}
	}

	bool VideoEncoder::IsPrimed()
	{
		return output->IsPrimed();
	}
}
