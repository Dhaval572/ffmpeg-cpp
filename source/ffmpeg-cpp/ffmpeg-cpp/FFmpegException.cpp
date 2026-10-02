#include "FFmpegException.h"
#include "ffmpeg.h"

namespace ffmpegcpp
{
	FFmpegException::FFmpegException(std::string error)
		: message(std::move(error))
	{
	}

	FFmpegException::FFmpegException(std::string error, int returnValue)
	{
		char errbuf[AV_ERROR_MAX_STRING_SIZE] = {};
		av_strerror(returnValue, errbuf, sizeof(errbuf));
		message = error + ": " + errbuf;
	}
}
