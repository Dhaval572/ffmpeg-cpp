#pragma once

#include "ffmpegcpp.h"

class GeneratedAudioSource : public ffmpegcpp::InputSource
{
public:

	explicit GeneratedAudioSource(ffmpegcpp::FrameSink* frameSink);
	~GeneratedAudioSource() override;

	void PreparePipeline() override;
	bool IsDone() override;
	void Step() override;

private:

	int sampleRate;
	int channels;
	AVSampleFormat format;

	ffmpegcpp::RawAudioDataSource* output;

	int sampleCount = 735;

	uint16_t* samples;

	int frameNumber = 0;
};
