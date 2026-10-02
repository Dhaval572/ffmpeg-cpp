#include "Codecs/VideoCodec.h"
#include "FFmpegException.h"
#include <catch2/catch_test_macros.hpp>
using namespace ffmpegcpp;

TEST_CASE("VideoCodec: construction with codec ID", "[video][codec]") {
  VideoCodec codec(AV_CODEC_ID_PNG);
  REQUIRE(codec.IsPixelFormatSupported(AV_PIX_FMT_RGB24));
}

TEST_CASE("VideoCodec: unknown codec throws", "[video][codec]") {
  REQUIRE_THROWS_AS(VideoCodec("nonexistent_video_xyz"), FFmpegException);
}

TEST_CASE("VideoCodec: IsPixelFormatSupported(NONE) returns true",
          "[video][codec]") {
  VideoCodec codec(AV_CODEC_ID_PNG);
  REQUIRE(codec.IsPixelFormatSupported(AV_PIX_FMT_NONE));
}

TEST_CASE("VideoCodec: PNG supports RGB24", "[video][codec]") {
  VideoCodec codec(AV_CODEC_ID_PNG);
  REQUIRE(codec.IsPixelFormatSupported(AV_PIX_FMT_RGB24));
}

TEST_CASE("VideoCodec: default pixel format is valid", "[video][codec]") {
  VideoCodec codec(AV_CODEC_ID_PNG);
  AVPixelFormat fmt = codec.GetDefaultPixelFormat();
  REQUIRE(fmt != AV_PIX_FMT_NONE);
}

TEST_CASE("VideoCodec: GetClosestSupportedFrameRate exact match",
          "[video][codec]") {
  VideoCodec codec(AV_CODEC_ID_MPEG2VIDEO);
  AVRational target = {24, 1};
  AVRational result = codec.GetClosestSupportedFrameRate(target);
  REQUIRE(result.num > 0);
  REQUIRE(result.den > 0);
}

TEST_CASE("VideoCodec: GetClosestSupportedFrameRate for unknown codec returns "
          "original",
          "[video][codec]") {
  VideoCodec codec(AV_CODEC_ID_PNG);
  AVRational target = {24, 1};
  AVRational result = codec.GetClosestSupportedFrameRate(target);
  REQUIRE(result.num == 24);
  REQUIRE(result.den == 1);
}

TEST_CASE("VideoCodec: Open succeeds with PNG", "[video][codec]") {
  VideoCodec codec(AV_CODEC_ID_PNG);
  AVRational fps = {24, 1};
  OpenCodec *open = codec.Open(640, 480, &fps, AV_PIX_FMT_RGB24);
  REQUIRE(open != nullptr);
  REQUIRE(open->GetContext() != nullptr);
  REQUIRE(open->GetContext()->width == 640);
  REQUIRE(open->GetContext()->height == 480);
  delete open;
}

TEST_CASE("VideoCodec: Open rejects unsupported pixel format",
          "[video][codec]") {
  VideoCodec codec(AV_CODEC_ID_PNG);
  AVRational fps = {24, 1};
  REQUIRE_THROWS_AS(codec.Open(640, 480, &fps, AV_PIX_FMT_YUV420P),
                    FFmpegException);
}
