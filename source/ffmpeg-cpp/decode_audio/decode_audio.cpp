
#include <iostream>

#include "ffmpegcpp.h"

using namespace std;
using namespace ffmpegcpp;

class RawAudioFileSink : public AudioFrameSink, public FrameWriter
{
public:

	explicit RawAudioFileSink(const char* fileName)
	{
		file = fopen(fileName, "wb");
	}

	~RawAudioFileSink() override
	{
		if (file) fclose(file);
	}

	FrameSinkStream* CreateStream() override
	{
		stream = new FrameSinkStream(this, 0);
		return stream;
	}

	void WriteFrame(int streamIndex, AVFrame* frame, StreamData* streamData) override
	{
		// Just write out the samples channel by channel to a file.
		int data_size = av_get_bytes_per_sample((AVSampleFormat)frame->format);
		for (int i = 0; i < frame->nb_samples; i++)
		{
			for (int ch = 0; ch < frame->ch_layout.nb_channels; ch++)
			{
				fwrite(frame->extended_data[ch] + data_size * i, 1, data_size, file);
			}
		}
	}

	void Close(int streamIndex) override
	{
		if (file)
		{
			fclose(file);
			file = nullptr;
		}
		delete stream;
		stream = nullptr;
	}

	bool IsPrimed() override
	{
		return true;
	}

private:
	FILE* file = nullptr;
	FrameSinkStream* stream = nullptr;
};

int main()
{
	// This example will decode an audio stream from a container and output it as raw audio data in the chosen format.
	try
	{
		// Load this container file so we can extract audio from it.
		Demuxer* demuxer = new Demuxer("samples/big_buck_bunny.mp4");

		// Create a file sink that will just output the raw audio data.
		RawAudioFileSink* fileSink = new RawAudioFileSink("rawaudio");

		// tie the file sink to the best audio stream in the input container.
		demuxer->DecodeBestAudioStream(fileSink);

		// Prepare the output pipeline. This will push a small amount of frames to the file sink until it IsPrimed returns true.
		demuxer->PreparePipeline();

		// Push all the remaining frames through.
		while (!demuxer->IsDone())
		{
			demuxer->Step();
		}

		// done
		delete demuxer;
		delete fileSink;
	}
	catch (const FFmpegException& e)
	{
		cerr << "Exception caught!" << endl;
		cerr << e.what() << endl;
		throw;
	}

	cout << "Decoding complete!" << endl;
	return 0;
}
