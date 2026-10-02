#include "OpenCodec.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	OpenCodec::OpenCodec(CodecContextPtr context)
		: context(std::move(context))
	{
		if (!avcodec_is_open(this->context.get()))
		{
			throw FFmpegException("Codec context for " + std::string(this->context->codec->name) + " hasn't been opened yet");
		}
	}

	OpenCodec::~OpenCodec()
	{
	}

	AVCodecContext* OpenCodec::GetContext()
	{
		return context.get();
	}
}
