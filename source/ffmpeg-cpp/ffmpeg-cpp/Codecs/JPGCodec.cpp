#include "JPGCodec.h"

namespace ffmpegcpp
{
	JPGCodec::JPGCodec()
		: VideoCodec(AV_CODEC_ID_MJPEG)
	{
		codecContext->pix_fmt = GetDefaultPixelFormat();
	}

	void JPGCodec::SetCompressionLevel(int compressionLevel)
	{
		SetOption("compression_level", compressionLevel);
	}
}
