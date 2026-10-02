
#include <iostream>

#include "ffmpegcpp.h"

using namespace std;
using namespace ffmpegcpp;

int main()
{
	// This example will apply some filters to a video and write it back.
	try
	{
		// Create a muxer that will output the video as MKV.
		Muxer* muxer = new Muxer("Vivaldi_filtered.aac");

		// Create an MP3 codec that will encode the raw data.
		AudioCodec* codec = new AudioCodec(AV_CODEC_ID_AAC);

		// Create an encoder that will encode the raw audio data as MP3.
		// Tie it to the muxer so it will be written to the file.
		AudioEncoder* encoder = new AudioEncoder(codec, muxer);

		// Create a video filter and do some funny stuff with the video data.
		Filter* filter = new Filter("areverse", encoder);

		// Load a video from a container and send it to the filter first.
		Demuxer* demuxer = new Demuxer("samples/Vivaldi_Sonata_eminor_.mp3");
		demuxer->DecodeBestAudioStream(filter);

		// Prepare the output pipeline. This will push a small amount of frames to the file sink until it IsPrimed returns true.
		demuxer->PreparePipeline();

		// Push all the remaining frames through.
		while (!demuxer->IsDone())
		{
			demuxer->Step();
		}

		// Save everything to disk by closing the muxer.
		muxer->Close();

		delete demuxer;
		delete filter;
		delete encoder;
		delete codec;
		delete muxer;
	}
	catch (const FFmpegException& e)
	{
		cerr << "Exception caught!" << endl;
		cerr << e.what() << endl;
		throw;
	}

	cout << "Encoding complete!" << endl;
	return 0;
}
