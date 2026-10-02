#include "AudioEncoder.h"
#include "Muxing/AudioOutputStream.h"

#include "FFmpegException.h"

namespace ffmpegcpp
{
	AudioEncoder::AudioEncoder(AudioCodec* codec, Muxer* muxer)
	{
		this->closedCodec = codec;

		output = new AudioOutputStream(muxer, codec);
		muxer->AddOutputStream(output);

		oneInputFrameSink = std::make_unique<OneInputFrameSink>(this, AVMEDIA_TYPE_AUDIO);
	}

	AudioEncoder::AudioEncoder(AudioCodec* codec, Muxer* muxer, int bitRate)
		: AudioEncoder(codec, muxer)
	{
		finalBitRate = bitRate;
	}

	void AudioEncoder::OpenLazily(AVFrame* frame, StreamData* metaData)
	{
		int bitRate = finalBitRate;
		if (bitRate == -1) bitRate = 0;
		int sampleRate = closedCodec->GetDefaultSampleRate();
		AVSampleFormat format = closedCodec->GetDefaultSampleFormat();

		codec.reset(closedCodec->Open(bitRate, format, sampleRate));

		pkt.reset(av_packet_alloc());
		if (!pkt)
		{
			throw FFmpegException("Failed to allocate packet");
		}

		formatConverter = std::make_unique<AudioFormatConverter>(this, codec->GetContext());
	}

	AudioEncoder::~AudioEncoder()
	{
	}

	FrameSinkStream* AudioEncoder::CreateStream()
	{
		return oneInputFrameSink->CreateStream();
	}

	void AudioEncoder::WriteFrame(int streamIndex, AVFrame* frame, StreamData* metaData)
	{
		if (!codec)
		{
			OpenLazily(frame, metaData);
		}

		frame->pts = frameNumber;
		frameNumber += frame->nb_samples;

		if (frame->ch_layout.nb_channels == 0)
		{
			av_channel_layout_default(&frame->ch_layout, 2);
		}
		else if (frame->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC)
		{
			AVChannelLayout normalized;
			av_channel_layout_default(&normalized, frame->ch_layout.nb_channels);
			av_channel_layout_uninit(&frame->ch_layout);
			av_channel_layout_copy(&frame->ch_layout, &normalized);
			av_channel_layout_uninit(&normalized);
		}

		formatConverter->ProcessFrame(frame);
	}

	void AudioEncoder::WriteConvertedFrame(AVFrame* frame)
	{
		int ret = avcodec_send_frame(codec->GetContext(), frame);
		if (ret < 0)
		{
			throw FFmpegException("Error sending a frame for encoding", ret);
		}
		PollCodecForPackets();
	}

	void AudioEncoder::Close(int streamIndex)
	{
		if (!codec) return;

		formatConverter->ProcessFrame(NULL);

		WriteConvertedFrame(NULL);
	}

	void AudioEncoder::PollCodecForPackets()
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

	bool AudioEncoder::IsPrimed()
	{
		return output->IsPrimed();
	}
}
