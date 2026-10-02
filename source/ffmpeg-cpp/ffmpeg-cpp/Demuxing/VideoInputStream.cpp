#include "VideoInputStream.h"
#include "FFmpegException.h"
#include "CodecDeducer.h"

namespace ffmpegcpp
{
	VideoInputStream::VideoInputStream(AVFormatContext* format, AVStream* stream)
		: InputStream(format, stream)
	{
	}

	VideoInputStream::~VideoInputStream()
	{
	}

	void VideoInputStream::ConfigureCodecContext()
	{
	}

	void VideoInputStream::AddStreamInfo(ContainerInfo* containerInfo)
	{
		VideoStreamInfo info{};

		info.id = stream->id;

		info.timeBase = stream->time_base;
		info.frameRate = av_guess_frame_rate(format, stream, NULL);

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

		info.format = tmpContext->pix_fmt;
		info.formatName = av_get_pix_fmt_name(info.format);

		info.width = tmpContext->width;
		info.height = tmpContext->height;

		avcodec_free_context(&tmpContext);

		containerInfo->videoStreams.push_back(info);
	}
}
