#include "AudioInputStream.h"
#include "FFmpegException.h"
#include "CodecDeducer.h"

namespace ffmpegcpp
{
	AudioInputStream::AudioInputStream(AVFormatContext* format, AVStream* stream)
		: InputStream(format, stream)
	{
	}

	void AudioInputStream::ConfigureCodecContext()
	{
		if (codecContext->ch_layout.nb_channels == 0)
		{
			av_channel_layout_default(&codecContext->ch_layout, 2);
		}
	}

	AudioInputStream::~AudioInputStream()
	{
	}

	void AudioInputStream::AddStreamInfo(ContainerInfo* containerInfo)
	{
		AudioStreamInfo info{};

		info.id = stream->id;

		info.timeBase = stream->time_base;

		AVCodecContext* tmpContext = avcodec_alloc_context3(NULL);
		if (!tmpContext) throw FFmpegException("Failed to allocate temporary codec context.");
		int ret = avcodec_parameters_to_context(tmpContext, stream->codecpar);
		if (ret < 0)
		{
			avcodec_free_context(&tmpContext);
			throw FFmpegException("Failed to read parameters from stream");
		}

		info.bitRate = CalculateBitRate(tmpContext);

		const AVCodec* codec = CodecDeducer::DeduceDecoder(tmpContext->codec_id);
		info.codec = codec;

		info.sampleRate = tmpContext->sample_rate;
		info.channels = tmpContext->ch_layout.nb_channels;
		av_channel_layout_describe(&tmpContext->ch_layout, info.channelLayoutName, sizeof(info.channelLayoutName));
		info.channelLayout = tmpContext->ch_layout.order == AV_CHANNEL_ORDER_NATIVE ? tmpContext->ch_layout.u.mask : 0;

		avcodec_free_context(&tmpContext);

		containerInfo->audioStreams.push_back(info);
	}
}
