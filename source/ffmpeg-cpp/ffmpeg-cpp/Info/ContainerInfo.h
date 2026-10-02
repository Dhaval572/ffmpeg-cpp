#pragma once

#include "ffmpeg.h"
#include "VideoStreamInfo.h"
#include "AudioStreamInfo.h"
#include "std.h"

namespace ffmpegcpp
{
	struct ContainerInfo
	{
		long durationInMicroSeconds = 0;
		float durationInSeconds = 0;
		float start = 0;
		float bitRate = 0;
		const AVInputFormat* format = nullptr;

		std::vector<VideoStreamInfo> videoStreams;
		std::vector<AudioStreamInfo> audioStreams;
	};
}
