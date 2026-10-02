#include <catch2/catch_test_macros.hpp>
#include "CodecDeducer.h"
#include "FFmpegException.h"

using namespace ffmpegcpp;

TEST_CASE("CodecDeducer: known encoder name returns codec", "[deducer]") {
	const AVCodec* c = CodecDeducer::DeduceEncoder("png");
	REQUIRE(c != nullptr);
	REQUIRE(std::string(c->name) == "png");
}

TEST_CASE("CodecDeducer: known encoder ID returns codec", "[deducer]") {
	const AVCodec* c = CodecDeducer::DeduceEncoder(AV_CODEC_ID_PNG);
	REQUIRE(c != nullptr);
}

TEST_CASE("CodecDeducer: unknown encoder name throws", "[deducer]") {
	REQUIRE_THROWS_AS(CodecDeducer::DeduceEncoder("nonexistent_codec_xyz"), FFmpegException);
}

TEST_CASE("CodecDeducer: unknown encoder ID throws", "[deducer]") {
	REQUIRE_THROWS_AS(CodecDeducer::DeduceEncoder(AV_CODEC_ID_NONE), FFmpegException);
}

TEST_CASE("CodecDeducer: known decoder name returns codec", "[deducer]") {
	const AVCodec* c = CodecDeducer::DeduceDecoder("png");
	REQUIRE(c != nullptr);
}

TEST_CASE("CodecDeducer: decoder with AV_CODEC_ID_NONE returns nullptr", "[deducer]") {
	const AVCodec* c = CodecDeducer::DeduceDecoder(AV_CODEC_ID_NONE);
	REQUIRE(c == nullptr);
}

TEST_CASE("CodecDeducer: DeduceEncoderFromFilename throws not implemented", "[deducer]") {
	try {
		CodecDeducer::DeduceEncoderFromFilename("test.mp4");
		FAIL("should have thrown");
	} catch (const FFmpegException& e) {
		REQUIRE(std::string(e.what()).find("Not implemented") != std::string::npos);
	}
}
