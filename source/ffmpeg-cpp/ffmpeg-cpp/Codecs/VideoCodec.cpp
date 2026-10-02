#include "VideoCodec.h"
#include "FFmpegException.h"

#include <limits>

namespace ffmpegcpp
{
	VideoCodec::VideoCodec(const char* codecName)
		: Codec(codecName)
	{
	}

	VideoCodec::VideoCodec(AVCodecID codecId)
		: Codec(codecId)
	{
	}

	VideoCodec::~VideoCodec()
	{
	}

	void VideoCodec::SetQualityScale(int qscale)
	{
		codecContext->flags |= AV_CODEC_FLAG_QSCALE;
		codecContext->global_quality = FF_QP2LAMBDA * 0;
	}

	bool VideoCodec::IsPixelFormatSupported(AVPixelFormat format)
	{
		if (format == AV_PIX_FMT_NONE) return true;

		const enum AVPixelFormat* p = nullptr;
		int nb = 0;
		if (avcodec_get_supported_config(nullptr, GetCodec(), AV_CODEC_CONFIG_PIX_FORMAT, 0, (const void**)&p, &nb) < 0 || !p)
			return true;

		while (*p != AV_PIX_FMT_NONE)
		{
			if (*p == format) return true;
			p++;
		}
		return false;
	}

	bool VideoCodec::IsFrameRateSupported(AVRational* frameRate)
	{
		const AVRational* p = nullptr;
		int nb = 0;
		if (avcodec_get_supported_config(nullptr, GetCodec(), AV_CODEC_CONFIG_FRAME_RATE, 0, (const void**)&p, &nb) < 0 || !p)
			return true;

		while (p->num)
		{
			if (av_cmp_q(*p, *frameRate) == 0) return true;
			p++;
		}
		return false;
	}

	OpenCodec* VideoCodec::Open(int width, int height, AVRational* frameRate, AVPixelFormat format)
	{
		if (!IsPixelFormatSupported(format)) throw FFmpegException("Pixel format " + std::string(av_get_pix_fmt_name(format)) + " is not supported by codec " + GetCodec()->name);
		if (!IsFrameRateSupported(frameRate)) throw FFmpegException("Frame rate " + std::to_string(frameRate->num) + "/" + std::to_string(frameRate->den) + " is not supported by codec " + GetCodec()->name);

		if (GetCodec()->type != AVMEDIA_TYPE_VIDEO) throw FFmpegException("A video output stream must be initialized with a video codec");

		codecContext->width = width;
		codecContext->height = height;
		codecContext->pix_fmt = format;

		AVRational time_base;
		time_base.num = frameRate->den;
		time_base.den = frameRate->num;
		codecContext->time_base = time_base;
		codecContext->framerate = *frameRate;

		return Codec::Open();
	}

	AVPixelFormat VideoCodec::GetDefaultPixelFormat()
	{
		const enum AVPixelFormat* p = nullptr;
		int nb = 0;
		if (avcodec_get_supported_config(nullptr, GetCodec(), AV_CODEC_CONFIG_PIX_FORMAT, 0, (const void**)&p, &nb) < 0 || !p || *p == AV_PIX_FMT_NONE)
			throw FFmpegException("Codec " + std::string(GetCodec()->name) + " does not have a default pixel format, you have to specify one");
		return *p;
	}

	AVRational VideoCodec::GetClosestSupportedFrameRate(AVRational originalFrameRate)
	{
		const AVRational* p = nullptr;
		int nb = 0;
		if (avcodec_get_supported_config(nullptr, GetCodec(), AV_CODEC_CONFIG_FRAME_RATE, 0, (const void**)&p, &nb) < 0 || !p)
			return originalFrameRate;

		AVRational bestFrameRate;
		bestFrameRate.num = 0;
		bestFrameRate.den = 1;
		double bestDiff = std::numeric_limits<double>::max();
		double fVal = av_q2d(originalFrameRate);
		while (p->num)
		{
			double pVal = av_q2d(*p);
			double diff = abs(pVal - fVal);
			if (diff < bestDiff)
			{
				bestDiff = diff;
				bestFrameRate.num = p->num;
				bestFrameRate.den = p->den;
			}
			p++;
		}

		if (bestFrameRate.num == 0) return originalFrameRate;

		return bestFrameRate;
	}
}
