#include "GeneratedAudioSource.h"

#include <cmath>
#include <cstdlib>

GeneratedAudioSource::GeneratedAudioSource(ffmpegcpp::FrameSink* frameSink)
{
	this->sampleRate = 44100;
	this->channels = 2;
	this->format = AV_SAMPLE_FMT_S16;

	output = new ffmpegcpp::RawAudioDataSource(format, this->sampleRate, this->channels, frameSink);

	samples = new uint16_t[channels * 2 * sampleCount];
}

GeneratedAudioSource::~GeneratedAudioSource()
{
	delete output;
	delete[] samples;
}

void GeneratedAudioSource::PreparePipeline()
{
	while (!output->IsPrimed() && !IsDone())
	{
		Step();
	}
}

bool GeneratedAudioSource::IsDone()
{
	return frameNumber >= 120;
}

void GeneratedAudioSource::Step()
{
	float t = 0.0f;
	float tincr = 2 * (float)M_PI * 440.0f / sampleRate;
	for (int i = 0; i < 120; i++)
	{
		for (int j = 0; j < sampleCount; j++)
		{
			samples[2 * j] = (int)(sin(t) * 10000);

			for (int k = 1; k < channels; k++)
				samples[2 * j + k] = samples[2 * j];
			t += tincr;
		}

		output->WriteData(samples, sampleCount);
		++frameNumber;
	}

	if (IsDone())
	{
		output->Close();
	}
}
