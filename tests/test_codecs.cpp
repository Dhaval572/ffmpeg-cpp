#include <catch2/catch_test_macros.hpp>
#include "Codecs/PNGCodec.h"
#include "Codecs/VP9Codec.h"
#include "CodecDeducer.h"
#include "FFmpegException.h"

using namespace ffmpegcpp;

TEST_CASE("PNGCodec: construction succeeds", "[png][codec]") {
	PNGCodec codec;
	REQUIRE(codec.IsPixelFormatSupported(AV_PIX_FMT_RGB24));
}

TEST_CASE("PNGCodec: SetCompressionLevel does not throw", "[png][codec]") {
	PNGCodec codec;
	REQUIRE_NOTHROW(codec.SetCompressionLevel(5));
}

TEST_CASE("PNGCodec: default pixel format is valid", "[png][codec]") {
	PNGCodec codec;
	REQUIRE(codec.GetDefaultPixelFormat() != AV_PIX_FMT_NONE);
}

TEST_CASE("VP9Codec: construction succeeds when libvpx-vp9 is available", "[vp9][codec]") {
	try {
		CodecDeducer::DeduceEncoder("libvpx-vp9");
	} catch (const FFmpegException&) {
		SKIP("libvpx-vp9 not available in this FFmpeg build");
	}
	VP9Codec codec;
	REQUIRE_NOTHROW(codec.SetCrf(30));
	REQUIRE_NOTHROW(codec.SetCpuUsed(5));
	REQUIRE_NOTHROW(codec.SetLossless(true));
	REQUIRE_NOTHROW(codec.SetDeadline("realtime"));
}
