#include <catch2/catch_test_macros.hpp>
extern "C" {
#include <libavutil/dict.h>
#include <libavutil/audio_fifo.h>
}
#include "FfmpegDeleters.h"
#include <memory>

using namespace ffmpegcpp;

TEST_CASE("FramePtr: allocates and frees AVFrame", "[deleter]") {
	FramePtr frame(av_frame_alloc());
	REQUIRE(frame != nullptr);
	REQUIRE(frame->data[0] == nullptr);
}

TEST_CASE("PacketPtr: allocates and frees AVPacket", "[deleter]") {
	PacketPtr pkt(av_packet_alloc());
	REQUIRE(pkt != nullptr);
}

TEST_CASE("CodecContextPtr: allocates and frees AVCodecContext", "[deleter]") {
	CodecContextPtr ctx(avcodec_alloc_context3(nullptr));
	REQUIRE(ctx != nullptr);
}

TEST_CASE("FormatContextPtr: handles nullptr", "[deleter]") {
	FormatContextPtr ctx(nullptr);
	REQUIRE(ctx == nullptr);
}

TEST_CASE("DictionaryPtr: nullptr is safe", "[deleter]") {
	DictionaryPtr dict(nullptr);
	REQUIRE(dict == nullptr);
}

TEST_CASE("AudioFifoPtr: allocates and frees AVAudioFifo", "[deleter]") {
	AudioFifoPtr fifo(av_audio_fifo_alloc(AV_SAMPLE_FMT_S16, 2, 1024));
	REQUIRE(fifo != nullptr);
}

TEST_CASE("FileCloser: nullptr is a no-op", "[deleter]") {
	FileCloser closer;
	REQUIRE_NOTHROW(closer(nullptr));
}

TEST_CASE("FileCloser: closes valid FILE*", "[deleter]") {
	FILE* f = fopen("test_deleter_tmp.txt", "w");
	REQUIRE(f != nullptr);
	fputs("test", f);
	FileCloser closer;
	REQUIRE_NOTHROW(closer(f));
}

TEST_CASE("FfmpegDeleters: all Ptr aliases are unique_ptr", "[deleter]") {
	static_assert(std::is_same_v<FramePtr, std::unique_ptr<AVFrame, AVFrameDeleter>>);
	static_assert(std::is_same_v<PacketPtr, std::unique_ptr<AVPacket, AVPacketDeleter>>);
	static_assert(std::is_same_v<CodecContextPtr, std::unique_ptr<AVCodecContext, AVCodecContextDeleter>>);
	static_assert(std::is_same_v<DictionaryPtr, std::unique_ptr<AVDictionary, AVDictionaryDeleter>>);
	static_assert(std::is_same_v<FilePtr, std::unique_ptr<FILE, FileCloser>>);
}
