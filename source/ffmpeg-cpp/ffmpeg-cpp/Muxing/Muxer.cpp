#include "Muxer.h"
#include "FFmpegException.h"
#include "OutputStream.h"
#include "CodecDeducer.h"

namespace ffmpegcpp
{
	Muxer::Muxer(const char* fileName)
	{
		this->fileName = fileName;

		AVFormatContext* ctx = nullptr;
		avformat_alloc_output_context2(&ctx, NULL, NULL, fileName);
		containerContext.reset(ctx);
		if (!containerContext)
		{
			printf("WARNING: Could not deduce output format from file extension: using MP4. as default\n");
			ctx = nullptr;
			avformat_alloc_output_context2(&ctx, NULL, "mp4", fileName);
			containerContext.reset(ctx);
		}
		if (!containerContext)
		{
			throw FFmpegException("Could not allocate container context for " + this->fileName);
		}

		containerFormat = containerContext->oformat;
	}

	Muxer::~Muxer()
	{
		bool wasPrimed = opened;

		for (size_t i = 0; i < outputStreams.size(); ++i)
		{
			delete outputStreams[i];
		}
		outputStreams.clear();

		if (containerContext && wasPrimed)
		{
			av_write_trailer(containerContext.get());

			if (!(containerFormat->flags & AVFMT_NOFILE))
				avio_closep(&containerContext->pb);

			containerContext.reset();
		}
	}

	const AVCodec* Muxer::GetDefaultVideoFormat()
	{
		return CodecDeducer::DeduceEncoder(containerFormat->video_codec);
	}

	const AVCodec* Muxer::GetDefaultAudioFormat()
	{
		return CodecDeducer::DeduceEncoder(containerFormat->audio_codec);
	}

	void Muxer::AddOutputStream(OutputStream* outputStream)
	{
		if (opened) throw FFmpegException("You cannot open a new stream after something was written to the muxer");

		AVStream* stream = avformat_new_stream(containerContext.get(), NULL);
		if (!stream)
		{
			throw FFmpegException("Could not allocate stream for container " + std::string(containerContext->oformat->name));
		}

		stream->id = containerContext->nb_streams - 1;

		outputStream->OpenStream(stream, containerContext->oformat->flags);

		outputStreams.push_back(outputStream);
	}

	bool Muxer::IsPrimed()
	{
		if (opened) return true;
		bool allPrimed = true;
		for (size_t i = 0; i < outputStreams.size(); ++i)
		{
			if (!outputStreams[i]->IsPrimed()) allPrimed = false;
		}

		if (allPrimed)
		{
			Open();
			opened = true;
		}
		return allPrimed;
	}

	void Muxer::WritePacket(AVPacket* pkt)
	{
		if (!opened)
		{
			throw FFmpegException("You cannot submit a packet to the muxer until all output streams are fully primed!");
		}

		int ret = av_interleaved_write_frame(containerContext.get(), pkt);
		if (ret < 0)
		{
			throw FFmpegException("Error while writing frame to output container", ret);
		}
	}

	void Muxer::Open()
	{
		if (!(containerFormat->flags & AVFMT_NOFILE))
		{
			int ret = avio_open(&containerContext->pb, fileName.c_str(), AVIO_FLAG_WRITE);
			if (ret < 0)
			{
				throw FFmpegException("Could not open file for container " + fileName, ret);
			}
		}

		int ret = avformat_write_header(containerContext.get(), NULL);
		if (ret < 0)
		{
			throw FFmpegException("Error when writing header to output file " + fileName, ret);
		}
	}

	void Muxer::Close()
	{
		if (!IsPrimed())
		{
			throw FFmpegException("You cannot close a muxer when one of the streams wasn't primed. You need to make sure all streams are primed before closing the muxer.");
		}

		for (size_t i = 0; i < outputStreams.size(); ++i)
		{
			outputStreams[i]->DrainPacketQueue();
		}

		opened = false;
		if (containerContext)
		{
			av_write_trailer(containerContext.get());
			if (!(containerFormat->flags & AVFMT_NOFILE))
				avio_closep(&containerContext->pb);
			containerContext.reset();
		}

		for (size_t i = 0; i < outputStreams.size(); ++i)
		{
			delete outputStreams[i];
		}
		outputStreams.clear();
	}
}
