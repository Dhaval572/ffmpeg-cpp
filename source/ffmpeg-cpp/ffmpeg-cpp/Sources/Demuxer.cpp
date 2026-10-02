#include "Demuxer.h"
#include "FFmpegException.h"
#include "CodecDeducer.h"

namespace ffmpegcpp
{
	Demuxer::Demuxer(const char* fileName)
		: Demuxer(fileName, NULL, NULL)
	{
	}

	Demuxer::Demuxer(const char* fileName, const AVInputFormat* inputFormat, AVDictionary* format_opts)
	{
		this->fileName = fileName;

		AVFormatContext* ctx = nullptr;
		int ret;
		if ((ret = avformat_open_input(&ctx, fileName, inputFormat, &format_opts)) < 0)
		{
			throw FFmpegException("Failed to open input container " + std::string(fileName), ret);
		}
		containerContext.reset(ctx);

		if ((ret = avformat_find_stream_info(containerContext.get(), NULL)) < 0)
		{
			throw FFmpegException("Failed to read streams from " + std::string(fileName), ret);
		}

		inputStreams.resize(containerContext->nb_streams);

		pkt.reset(av_packet_alloc());
		if (!pkt)
		{
			throw FFmpegException("Failed to create packet for input stream");
		}
	}

	Demuxer::~Demuxer()
	{
	}

	void Demuxer::DecodeBestAudioStream(FrameSink* frameSink)
	{
		int ret = av_find_best_stream(containerContext.get(), AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
		if (ret < 0)
		{
			throw FFmpegException("Could not find " + std::string(av_get_media_type_string(AVMEDIA_TYPE_AUDIO)) + " stream in input file " + fileName, ret);
		}
		DecodeAudioStream(ret, frameSink);
	}

	void Demuxer::DecodeBestVideoStream(FrameSink* frameSink)
	{
		int ret = av_find_best_stream(containerContext.get(), AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
		if (ret < 0)
		{
			throw FFmpegException("Could not find " + std::string(av_get_media_type_string(AVMEDIA_TYPE_VIDEO)) + " stream in input file " + fileName, ret);
		}
		DecodeVideoStream(ret, frameSink);
	}

	void Demuxer::DecodeAudioStream(int streamIndex, FrameSink* frameSink)
	{
		if (inputStreams[streamIndex])
		{
			throw FFmpegException("That stream is already tied to a frame sink, you cannot process the same stream multiple times");
		}

		InputStream* inputStream = GetInputStream(streamIndex);
		if (inputStream == nullptr)
		{
			throw FFmpegException("No usable audio stream found at index " + std::to_string(streamIndex));
		}
		inputStream->Open(frameSink);
	}

	void Demuxer::DecodeVideoStream(int streamIndex, FrameSink* frameSink)
	{
		if (inputStreams[streamIndex])
		{
			throw FFmpegException("That stream is already tied to a frame sink, you cannot process the same stream multiple times");
		}

		InputStream* inputStream = GetInputStream(streamIndex);
		if (inputStream == nullptr)
		{
			throw FFmpegException("No usable video stream found at index " + std::to_string(streamIndex));
		}
		inputStream->Open(frameSink);
	}

	InputStream* Demuxer::GetInputStream(int streamIndex)
	{
		if (inputStreams[streamIndex]) return inputStreams[streamIndex].get();

		if (IsDone()) return nullptr;

		AVStream* stream = containerContext->streams[streamIndex];
		const AVCodec* codec = CodecDeducer::DeduceDecoder(stream->codecpar->codec_id);
		if (codec == nullptr) return nullptr;

		InputStream* created = nullptr;
		switch (codec->type)
		{
		case AVMEDIA_TYPE_VIDEO:
			created = new VideoInputStream(containerContext.get(), stream);
			break;
		case AVMEDIA_TYPE_AUDIO:
			created = new AudioInputStream(containerContext.get(), stream);
			break;
		default:
			return nullptr;
		}

		inputStreams[streamIndex].reset(created);
		return created;
	}

	InputStream* Demuxer::GetInputStreamById(int streamId)
	{
		for (int i = 0; i < (int)containerContext->nb_streams; ++i)
		{
			AVStream* stream = containerContext->streams[i];
			if (stream->id == streamId) return GetInputStream(i);
		}

		return nullptr;
	}

	void Demuxer::PreparePipeline()
	{
		bool allPrimed = false;
		do
		{
			Step();

			allPrimed = true;
			for (int i = 0; i < (int)containerContext->nb_streams; ++i)
			{
				InputStream* stream = inputStreams[i].get();
				if (stream != nullptr)
				{
					if (!stream->IsPrimed()) allPrimed = false;
				}
			}

		} while (!allPrimed && !IsDone());
	}

	bool Demuxer::IsDone()
	{
		return done;
	}

	void Demuxer::Step()
	{
		int ret = av_read_frame(containerContext.get(), pkt.get());

		if (ret == AVERROR_EOF)
		{
			for (int i = 0; i < (int)containerContext->nb_streams; ++i)
			{
				InputStream* stream = inputStreams[i].get();
				if (stream != nullptr)
				{
					pkt->stream_index = i;
					DecodePacket();
					stream->Close();
				}
			}

			done = true;
			return;
		}

		if (ret == AVERROR(EAGAIN)) return;

		if (ret < 0)
		{
			throw FFmpegException("Error during demuxing", ret);
		}

		DecodePacket();
	}

	void Demuxer::DecodePacket()
	{
		int streamIndex = pkt->stream_index;
		InputStream* inputStream = inputStreams[streamIndex].get();

		if (inputStream != nullptr)
		{
			inputStream->DecodePacket(pkt.get());
		}

		av_packet_unref(pkt.get());
	}

	ContainerInfo Demuxer::GetInfo()
	{
		ContainerInfo info;

		int64_t duration = containerContext->duration + (containerContext->duration <= INT64_MAX - 5000 ? 5000 : 0);
		info.durationInMicroSeconds = duration;
		info.durationInSeconds = (float)info.durationInMicroSeconds / AV_TIME_BASE;
		info.start = (float)containerContext->start_time / AV_TIME_BASE;
		info.bitRate = containerContext->bit_rate;
		info.format = containerContext->iformat;

		for (int i = 0; i < (int)containerContext->nb_streams; ++i)
		{
			InputStream* stream = GetInputStream(i);
			if (stream == nullptr) continue;
			stream->AddStreamInfo(&info);
		}

		return info;
	}

	int Demuxer::GetFrameCount(int streamId)
	{
		for (int i = 0; i < (int)containerContext->nb_streams; ++i)
		{
			GetInputStream(i);
		}

		while (!IsDone())
		{
			Step();
		}

		InputStream* stream = GetInputStreamById(streamId);
		return stream ? stream->GetFramesProcessed() : 0;
	}

	const char* Demuxer::GetFileName()
	{
		return fileName.c_str();
	}
}
