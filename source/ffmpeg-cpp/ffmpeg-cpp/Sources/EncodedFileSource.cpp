#include "EncodedFileSource.h"
#include "FFmpegException.h"
#include "CodecDeducer.h"

namespace ffmpegcpp
{
	EncodedFileSource::EncodedFileSource(const char* inFileName, const char* codecName, FrameSink* output)
	{
		const AVCodec* codec = CodecDeducer::DeduceDecoder(codecName);
		Init(inFileName, codec, output);
	}

	EncodedFileSource::EncodedFileSource(const char* inFileName, AVCodecID codecId, FrameSink* output)
	{
		const AVCodec* codec = CodecDeducer::DeduceDecoder(codecId);
		Init(inFileName, codec, output);
	}

	EncodedFileSource::~EncodedFileSource()
	{
	}

	void EncodedFileSource::Init(const char* inFileName, const AVCodec* codec, FrameSink* output)
	{
		this->output = output->CreateStream();
		this->codec = codec;

		parser.reset(av_parser_init(codec->id));
		if (!parser)
		{
			throw FFmpegException("Parser for codec not found " + std::string(codec->name));
		}

		codecContext.reset(avcodec_alloc_context3(codec));
		if (!codecContext)
		{
			throw FFmpegException("Failed to allocate context for codec " + std::string(codec->name));
		}

		int ret = avcodec_open2(codecContext.get(), codec, NULL);
		if (ret < 0)
		{
			throw FFmpegException("Failed to open context for codec " + std::string(codec->name), ret);
		}

		file.reset(fopen(inFileName, "rb"));
		if (!file)
		{
			throw FFmpegException("Could not open file " + std::string(inFileName));
		}

		decoded_frame.reset(av_frame_alloc());
		if (!decoded_frame)
		{
			throw FFmpegException("Could not allocate video frame");
		}

		pkt.reset(av_packet_alloc());
		if (!pkt)
		{
			throw FFmpegException("Failed to allocate packet");
		}

		if (codecContext->codec->type == AVMEDIA_TYPE_VIDEO)
		{
			bufferSize = 4096;
		}
		else if (codecContext->codec->type == AVMEDIA_TYPE_AUDIO)
		{
			bufferSize = 20480;
		}
		else
		{
			throw FFmpegException("Codec " + std::string(codecContext->codec->name) + " is not supported as a RawFileSource");
		}

		buffer = std::make_unique<uint8_t[]>(bufferSize + AV_INPUT_BUFFER_PADDING_SIZE);
		memset(buffer.get() + bufferSize, 0, AV_INPUT_BUFFER_PADDING_SIZE);
	}

	void EncodedFileSource::PreparePipeline()
	{
		while (!output->IsPrimed() && !IsDone())
		{
			Step();
		}
	}

	bool EncodedFileSource::IsDone()
	{
		return done;
	}

	void EncodedFileSource::Step()
	{
		uint8_t* data;
		size_t data_size;
		int ret;

		data_size = fread(buffer.get(), 1, bufferSize, file.get());
		if (!data_size) return;

		data = buffer.get();
		while (data_size > 0)
		{
			ret = av_parser_parse2(parser.get(), codecContext.get(), &pkt->data, &pkt->size,
				data, (int)data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (ret < 0)
			{
				throw FFmpegException("Error while parsing file", ret);
			}
			data += ret;
			data_size -= ret;

			if (pkt->size)
			{
				Decode(pkt.get(), decoded_frame.get());
			}
		}

		if (feof(file.get()))
		{
			pkt->data = NULL;
			pkt->size = 0;
			Decode(pkt.get(), decoded_frame.get());

			output->Close();

			done = true;
		}
	}

	void EncodedFileSource::Decode(AVPacket* pkt, AVFrame* frame)
	{
		int ret;

		ret = avcodec_send_packet(codecContext.get(), pkt);
		if (ret < 0)
		{
			throw FFmpegException("Error submitting the packet to the decoder", ret);
		}

		while (ret >= 0)
		{
			ret = avcodec_receive_frame(codecContext.get(), frame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
			{
				throw FFmpegException("Error during decoding", ret);
			}

			if (metaData == nullptr)
			{
				metaData = std::make_unique<StreamData>();
				metaData->timeBase = codecContext->time_base;
				if (metaData->timeBase.num == 0 || metaData->timeBase.den == 0)
				{
					metaData->timeBase.num = 1;
					metaData->timeBase.den = 90000;
				}
				metaData->frameRate.num = metaData->timeBase.den;
				metaData->frameRate.den = metaData->timeBase.num;
				metaData->type = codecContext->codec->type;
			}

			output->WriteFrame(frame, metaData.get());
		}
	}
}
