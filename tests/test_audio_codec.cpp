#include <catch2/catch_test_macros.hpp>
#include "Codecs/AudioCodec.h"
#include "FFmpegException.h"

using namespace ffmpegcpp;

TEST_CASE("AudioCodec: construction with codec ID", "[audio][codec]") {
	AudioCodec codec(AV_CODEC_ID_PCM_S16LE);
	REQUIRE(codec.IsFormatSupported(AV_SAMPLE_FMT_S16));
}

TEST_CASE("AudioCodec: construction with codec name", "[audio][codec]") {
	AudioCodec codec("pcm_s16le");
	REQUIRE(codec.IsFormatSupported(AV_SAMPLE_FMT_S16));
}

TEST_CASE("AudioCodec: unknown codec throws", "[audio][codec]") {
	REQUIRE_THROWS_AS(AudioCodec("nonexistent_audio_xyz"), FFmpegException);
}

TEST_CASE("AudioCodec: PCM supports S16 format", "[audio][codec]") {
	AudioCodec codec(AV_CODEC_ID_PCM_S16LE);
	REQUIRE(codec.IsFormatSupported(AV_SAMPLE_FMT_S16));
}

TEST_CASE("AudioCodec: default sample format is valid", "[audio][codec]") {
	AudioCodec codec(AV_CODEC_ID_PCM_S16LE);
	AVSampleFormat fmt = codec.GetDefaultSampleFormat();
	REQUIRE(fmt != AV_SAMPLE_FMT_NONE);
}

TEST_CASE("AudioCodec: default sample rate is positive", "[audio][codec]") {
	AudioCodec codec(AV_CODEC_ID_PCM_S16LE);
	REQUIRE(codec.GetDefaultSampleRate() > 0);
}

TEST_CASE("AudioCodec: common sample rates are supported", "[audio][codec]") {
	AudioCodec codec(AV_CODEC_ID_PCM_S16LE);
	REQUIRE(codec.IsSampleRateSupported(44100));
	REQUIRE(codec.IsSampleRateSupported(48000));
}

TEST_CASE("AudioCodec: 2 channels supported", "[audio][codec]") {
	AudioCodec codec(AV_CODEC_ID_PCM_S16LE);
	REQUIRE(codec.IsChannelsSupported(2));
}

TEST_CASE("AudioCodec: Open succeeds with PCM", "[audio][codec]") {
	AudioCodec codec(AV_CODEC_ID_PCM_S16LE);
	OpenCodec* open = codec.Open(128000, AV_SAMPLE_FMT_S16, 44100);
	REQUIRE(open != nullptr);
	REQUIRE(open->GetContext() != nullptr);
	REQUIRE(open->GetContext()->sample_rate == 44100);
	REQUIRE(open->GetContext()->sample_fmt == AV_SAMPLE_FMT_S16);
	delete open;
}
