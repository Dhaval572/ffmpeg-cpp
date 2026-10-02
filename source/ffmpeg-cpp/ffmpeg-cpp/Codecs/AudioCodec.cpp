#include "AudioCodec.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	AudioCodec::AudioCodec(const char* codecName)
		: Codec(codecName)
	{
	}

	AudioCodec::AudioCodec(AVCodecID codecId)
		: Codec(codecId)
	{
	}

	AudioCodec::~AudioCodec()
	{
	}

	static bool check_sample_fmt(const AVCodec* codec, enum AVSampleFormat sample_fmt)
	{
		const enum AVSampleFormat* p = nullptr;
		int nb = 0;
		if (avcodec_get_supported_config(nullptr, codec, AV_CODEC_CONFIG_SAMPLE_FORMAT, 0, (const void**)&p, &nb) < 0 || !p)
			return false;

		while (*p != AV_SAMPLE_FMT_NONE)
		{
			if (*p == sample_fmt) return true;
			p++;
		}
		return false;
	}

	static int select_sample_rate(const AVCodec* codec)
	{
		const int* p = nullptr;
		int nb = 0;
		if (avcodec_get_supported_config(nullptr, codec, AV_CODEC_CONFIG_SAMPLE_RATE, 0, (const void**)&p, &nb) < 0 || !p)
			return 44100;

		int best_samplerate = 0;
		while (*p)
		{
			if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
				best_samplerate = *p;
			p++;
		}
		return best_samplerate != 0 ? best_samplerate : 44100;
	}

	static AVChannelLayout select_channel_layout(const AVCodec* codec)
	{
		AVChannelLayout best{};
		av_channel_layout_default(&best, 2);

		const AVChannelLayout* layouts = nullptr;
		int nb = 0;
		if (avcodec_get_supported_config(nullptr, codec, AV_CODEC_CONFIG_CHANNEL_LAYOUT, 0, (const void**)&layouts, &nb) < 0 || !layouts)
			return best;

		int best_nb_channels = 0;
		const AVChannelLayout* p = layouts;
		while (p->nb_channels)
		{
			if (p->nb_channels > best_nb_channels)
			{
				best_nb_channels = p->nb_channels;
				av_channel_layout_copy(&best, p);
			}
			p++;
		}
		return best;
	}

	bool AudioCodec::IsChannelsSupported(int channels)
	{
		const AVChannelLayout* layouts = nullptr;
		int nb = 0;
		const AVCodec* codec = GetCodec();
		if (avcodec_get_supported_config(nullptr, codec, AV_CODEC_CONFIG_CHANNEL_LAYOUT, 0, (const void**)&layouts, &nb) < 0 || !layouts)
			return true;

		AVChannelLayout def{};
		av_channel_layout_default(&def, channels);

		const AVChannelLayout* p = layouts;
		while (p->nb_channels)
		{
			if (def.nb_channels == p->nb_channels &&
				(def.order != AV_CHANNEL_ORDER_NATIVE || p->order != AV_CHANNEL_ORDER_NATIVE || def.u.mask == p->u.mask))
				return true;
			p++;
		}
		return false;
	}

	bool AudioCodec::IsFormatSupported(AVSampleFormat format)
	{
		return check_sample_fmt(GetCodec(), format);
	}

	bool AudioCodec::IsSampleRateSupported(int sampleRate)
	{
		const int* p = nullptr;
		int nb = 0;
		if (avcodec_get_supported_config(nullptr, GetCodec(), AV_CODEC_CONFIG_SAMPLE_RATE, 0, (const void**)&p, &nb) < 0 || !p)
			return true;

		while (*p)
		{
			if (*p == sampleRate) return true;
			p++;
		}
		return false;
	}

	AVSampleFormat AudioCodec::GetDefaultSampleFormat()
	{
		const enum AVSampleFormat* p = nullptr;
		int nb = 0;
		if (avcodec_get_supported_config(nullptr, GetCodec(), AV_CODEC_CONFIG_SAMPLE_FORMAT, 0, (const void**)&p, &nb) < 0 || !p || *p == AV_SAMPLE_FMT_NONE)
			return AV_SAMPLE_FMT_FLTP;
		return *p;
	}

	int AudioCodec::GetDefaultSampleRate()
	{
		return select_sample_rate(GetCodec());
	}

	OpenCodec* AudioCodec::Open(int bitRate, AVSampleFormat format, int sampleRate)
	{
		if (!IsFormatSupported(format)) throw FFmpegException("Sample format " + std::string(av_get_sample_fmt_name(format)) + " is not supported by codec " + GetCodec()->name);
		if (!IsSampleRateSupported(sampleRate)) throw FFmpegException("Sample rate " + std::to_string(sampleRate) + " is not supported by codec " + GetCodec()->name);

		if (GetCodec()->type != AVMEDIA_TYPE_AUDIO) throw FFmpegException("An audio output stream must be initialized with an audio codec");

		codecContext->bit_rate = bitRate;
		codecContext->sample_fmt = format;
		codecContext->sample_rate = sampleRate;

		AVChannelLayout layout = select_channel_layout(GetCodec());
		av_channel_layout_copy(&codecContext->ch_layout, &layout);
		av_channel_layout_uninit(&layout);

		codecContext->flags = 0;

		return Codec::Open();
	}
}
