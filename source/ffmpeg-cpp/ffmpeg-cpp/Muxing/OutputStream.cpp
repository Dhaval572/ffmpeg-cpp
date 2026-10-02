#include "OutputStream.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	OutputStream::OutputStream(Muxer* muxer, Codec* codec)
	{
		this->muxer = muxer;
		this->codec = codec;
	}

	OutputStream::~OutputStream()
	{
		for (size_t i = 0; i < packetQueue.size(); ++i)
		{
			av_packet_free(&packetQueue[i]);
		}
		packetQueue.clear();
	}

	void OutputStream::SendPacketToMuxer(AVPacket* pkt)
	{
		if (muxer->IsPrimed())
		{
			DrainPacketQueue();

			PreparePacketForMuxer(pkt);
			muxer->WritePacket(pkt);
		}
		else
		{
			AVPacket* tmp_pkt = av_packet_alloc();
			if (!tmp_pkt)
			{
				throw FFmpegException("Failed to allocate packet");
			}
			av_packet_ref(tmp_pkt, pkt);
			packetQueue.push_back(tmp_pkt);
		}
	}

	void OutputStream::DrainPacketQueue()
	{
		if (packetQueue.size() > 0) printf("Drain %d packets from the packet queue...", (int)packetQueue.size());
		for (size_t i = 0; i < packetQueue.size(); ++i)
		{
			AVPacket* tmp_pkt = packetQueue[i];

			PreparePacketForMuxer(tmp_pkt);
			muxer->WritePacket(tmp_pkt);

			av_packet_unref(tmp_pkt);
			av_packet_free(&tmp_pkt);
		}

		packetQueue.clear();
	}
}
