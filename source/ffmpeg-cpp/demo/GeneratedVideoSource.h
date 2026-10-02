#pragma once

#include "ffmpegcpp.h"

#include <cstdint>

class GeneratedVideoSource : public ffmpegcpp::InputSource
{
public:

	GeneratedVideoSource(int width, int height, ffmpegcpp::FrameSink* frameSink);
	~GeneratedVideoSource() override;

	void PreparePipeline() override;
	bool IsDone() override;
	void Step() override;

private:

	ffmpegcpp::RawVideoDataSource* output;

	int frameNumber = 0;

	uint8_t* rgb = nullptr;
};
