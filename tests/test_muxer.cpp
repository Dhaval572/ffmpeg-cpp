#include <catch2/catch_test_macros.hpp>
#include "Muxing/Muxer.h"
#include "FFmpegException.h"
#include <cstdio>

using namespace ffmpegcpp;

TEST_CASE("Muxer: WritePacket before primed throws", "[muxer]") 
{
	Muxer muxer("test_output.mp4");
	AVPacket* pkt = av_packet_alloc();
	REQUIRE_THROWS_AS(muxer.WritePacket(pkt), FFmpegException);
	av_packet_free(&pkt);
	std::remove("test_output.mp4");
}

TEST_CASE("Muxer: copy constructor is deleted", "[muxer]") 
{
	static_assert(!std::is_copy_constructible_v<Muxer>, "Muxer must not be copyable");
	static_assert(!std::is_copy_assignable_v<Muxer>, "Muxer must not be copy assignable");
}

TEST_CASE("Muxer: IsPrimed throws with zero streams", "[muxer]") 
{
	Muxer muxer("test_output2.mp4");
	REQUIRE_THROWS_AS(muxer.IsPrimed(), FFmpegException);
	std::remove("test_output2.mp4");
}

TEST_CASE("Muxer: Close throws with zero streams", "[muxer]") 
{
	Muxer muxer("test_output3.mp4");
	REQUIRE_THROWS_AS(muxer.Close(), FFmpegException);
	std::remove("test_output3.mp4");
}
