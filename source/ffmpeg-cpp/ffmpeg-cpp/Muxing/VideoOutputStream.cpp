#include "VideoOutputStream.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	VideoOutputStream::VideoOutputStream(Muxer* muxer, Codec* codec)
		: OutputStream(muxer, codec)
	{
	}

	void VideoOutputStream::OpenStream(AVStream* stream, int containerFlags)
	{
		this->stream = stream;

		if (containerFlags & AVFMT_GLOBALHEADER)
		{
			codec->SetGlobalContainerHeader();
		}
	}

	void VideoOutputStream::LazilyInitialize(OpenCodec* openCodec)
	{
		stream->time_base = openCodec->GetContext()->time_base;
		stream->avg_frame_rate = openCodec->GetContext()->framerate;

		stream->disposition = 1;

		int ret = avcodec_parameters_from_context(stream->codecpar, openCodec->GetContext());
		if (ret < 0)
		{
			throw FFmpegException("Could not copy codec parameters to stream", ret);
		}

		codecTimeBase = openCodec->GetContext()->time_base;

		if (openCodec->GetContext()->nb_coded_side_data)
		{
			for (int i = 0; i < openCodec->GetContext()->nb_coded_side_data; i++)
			{
				const AVPacketSideData* sd_src = &openCodec->GetContext()->coded_side_data[i];
				uint8_t* dst_data = (uint8_t*)av_mallocz(sd_src->size);
				if (!dst_data)
				{
					throw FFmpegException("Failed to allocate memory for new side_data");
				}
				memcpy(dst_data, sd_src->data, sd_src->size);
				av_packet_side_data_add(&stream->codecpar->coded_side_data, &stream->codecpar->nb_coded_side_data, sd_src->type, dst_data, sd_src->size, 0);
			}
		}
	}

	void VideoOutputStream::WritePacket(AVPacket* pkt, OpenCodec* openCodec)
	{
		if (!initialized)
		{
			LazilyInitialize(openCodec);
			initialized = true;
		}

		SendPacketToMuxer(pkt);
	}

	void VideoOutputStream::PreparePacketForMuxer(AVPacket* pkt)
	{
		av_packet_rescale_ts(pkt, codecTimeBase, stream->time_base);
		pkt->stream_index = stream->index;

		if (stream->time_base.num != 0 && stream->avg_frame_rate.num != 0)
		{
			pkt->duration = stream->time_base.den / stream->time_base.num / stream->avg_frame_rate.num * stream->avg_frame_rate.den;
		}
	}

	bool VideoOutputStream::IsPrimed()
	{
		return initialized;
	}
}
