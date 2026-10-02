#include <catch2/catch_test_macros.hpp>
extern "C" {
#include <libavutil/error.h>
}
#include "FFmpegException.h"

using namespace ffmpegcpp;

TEST_CASE("FFmpegException: message is preserved", "[exception]") {
	FFmpegException e("something went wrong");
	REQUIRE(std::string(e.what()) == "something went wrong");
}

TEST_CASE("FFmpegException: empty message is safe", "[exception]") {
	FFmpegException e("");
	REQUIRE(e.what() != nullptr);
}

TEST_CASE("FFmpegException: two-arg ctor appends av_strerror", "[exception]") {
	FFmpegException e("operation failed", AVERROR(EINVAL));
	std::string msg = e.what();
	REQUIRE(msg.find("operation failed") == 0);
	REQUIRE(msg.size() > std::string("operation failed").size());
}

TEST_CASE("FFmpegException: inherits std::exception", "[exception]") {
	FFmpegException e("test");
	const std::exception& base = e;
	REQUIRE(std::string(base.what()) == "test");
}
