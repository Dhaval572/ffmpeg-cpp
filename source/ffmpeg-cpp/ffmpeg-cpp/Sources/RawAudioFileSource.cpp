#include "RawAudioFileSource.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	RawAudioFileSource::RawAudioFileSource(const char* fileName, const char* inputFormat, int sampleRate, int channels, FrameSink* frameSink)
	{
		const AVInputFormat* file_iformat = av_find_input_format(inputFormat);
		if (!file_iformat)
		{
			throw FFmpegException("Unknown input format: " + std::string(inputFormat));
		}

		AVDictionary* format_opts = NULL;
		av_dict_set_int(&format_opts, "sample_rate", sampleRate, 0);
		av_dict_set_int(&format_opts, "channels", channels, 0);

		demuxer = std::make_unique<Demuxer>(fileName, file_iformat, format_opts);

		demuxer->DecodeBestAudioStream(frameSink);
	}

	RawAudioFileSource::~RawAudioFileSource()
	{
	}

	void RawAudioFileSource::PreparePipeline()
	{
		demuxer->PreparePipeline();
	}

	bool RawAudioFileSource::IsDone()
	{
		return demuxer->IsDone();
	}

	void RawAudioFileSource::Step()
	{
		demuxer->Step();
	}
}
