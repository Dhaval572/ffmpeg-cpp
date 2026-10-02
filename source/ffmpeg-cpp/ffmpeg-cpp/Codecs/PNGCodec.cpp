#include "PNGCodec.h"

namespace ffmpegcpp
{
	PNGCodec::PNGCodec()
		: VideoCodec(AV_CODEC_ID_PNG)
	{
		codecContext->pix_fmt = GetDefaultPixelFormat();
	}

	void PNGCodec::SetCompressionLevel(int compressionLevel)
	{
		SetOption("compression_level", compressionLevel);
	}
}
