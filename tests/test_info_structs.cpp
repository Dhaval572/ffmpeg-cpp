#include <catch2/catch_test_macros.hpp>
#include "Info/AudioStreamInfo.h"
#include "Info/VideoStreamInfo.h"
#include "Info/ContainerInfo.h"
#include "Demuxing/StreamData.h"

using namespace ffmpegcpp;

TEST_CASE("AudioStreamInfo: defaults are zeroed", "[info][audio]") {
	AudioStreamInfo info;
	REQUIRE(info.id == 0);
	REQUIRE(info.timeBase.num == 0);
	REQUIRE(info.timeBase.den == 1);
	REQUIRE(info.codec == nullptr);
	REQUIRE(info.bitRate == 0);
	REQUIRE(info.sampleRate == 0);
	REQUIRE(info.channels == 0);
	REQUIRE(info.channelLayout == 0);
	REQUIRE(info.channelLayoutName[0] == '\0');
}

TEST_CASE("VideoStreamInfo: defaults are zeroed", "[info][video]") {
	VideoStreamInfo info;
	REQUIRE(info.id == 0);
	REQUIRE(info.frameRate.num == 0);
	REQUIRE(info.frameRate.den == 1);
	REQUIRE(info.timeBase.num == 0);
	REQUIRE(info.timeBase.den == 1);
	REQUIRE(info.codec == nullptr);
	REQUIRE(info.bitRate == 0);
	REQUIRE(info.format == AV_PIX_FMT_NONE);
	REQUIRE(info.formatName == nullptr);
	REQUIRE(info.width == 0);
	REQUIRE(info.height == 0);
}

TEST_CASE("ContainerInfo: defaults are zeroed", "[info][container]") {
	ContainerInfo info;
	REQUIRE(info.durationInMicroSeconds == 0);
	REQUIRE(info.durationInSeconds == 0);
	REQUIRE(info.start == 0);
	REQUIRE(info.bitRate == 0);
	REQUIRE(info.format == nullptr);
	REQUIRE(info.videoStreams.empty());
	REQUIRE(info.audioStreams.empty());
}

TEST_CASE("ContainerInfo: streams can be added", "[info][container]") {
	ContainerInfo info;
	VideoStreamInfo v;
	v.width = 640;
	v.height = 480;
	info.videoStreams.push_back(v);

	AudioStreamInfo a;
	a.sampleRate = 44100;
	info.audioStreams.push_back(a);

	REQUIRE(info.videoStreams.size() == 1);
	REQUIRE(info.audioStreams.size() == 1);
	REQUIRE(info.videoStreams[0].width == 640);
	REQUIRE(info.audioStreams[0].sampleRate == 44100);
}

TEST_CASE("StreamData: defaults are zeroed", "[info][stream]") {
	StreamData data;
	REQUIRE(data.type == AVMEDIA_TYPE_UNKNOWN);
	REQUIRE(data.timeBase.num == 0);
	REQUIRE(data.timeBase.den == 1);
	REQUIRE(data.frameRate.num == 0);
	REQUIRE(data.frameRate.den == 1);
}
