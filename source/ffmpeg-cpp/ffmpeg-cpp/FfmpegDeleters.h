#pragma once

#include "ffmpeg.h"

#include <memory>

namespace ffmpegcpp
{
	struct AVFrameDeleter
	{
		void operator()(AVFrame* p) const { av_frame_free(&p); }
	};

	struct AVPacketDeleter
	{
		void operator()(AVPacket* p) const { av_packet_free(&p); }
	};

	struct AVCodecContextDeleter
	{
		void operator()(AVCodecContext* p) const { avcodec_free_context(&p); }
	};

	struct AVFormatContextDeleter
	{
		void operator()(AVFormatContext* p) const { avformat_close_input(&p); }
	};

	struct AVFormatOutputContextDeleter
	{
		void operator()(AVFormatContext* p) const { avformat_free_context(p); }
	};

	struct AVFilterGraphDeleter
	{
		void operator()(AVFilterGraph* p) const { avfilter_graph_free(&p); }
	};

	struct SwrContextDeleter
	{
		void operator()(SwrContext* p) const { swr_free(&p); }
	};

	struct SwsContextDeleter
	{
		void operator()(SwsContext* p) const { sws_freeContext(p); }
	};

	struct AVAudioFifoDeleter
	{
		void operator()(AVAudioFifo* p) const { av_audio_fifo_free(p); }
	};

	struct AVFifoDeleter
	{
		void operator()(AVFifo* p) const { av_fifo_freep2(&p); }
	};

	struct AVDictionaryDeleter
	{
		void operator()(AVDictionary* p) const { av_dict_free(&p); }
	};

	struct AVParserContextDeleter
	{
		void operator()(AVCodecParserContext* p) const { av_parser_close(p); }
	};

	struct FileCloser
	{
		void operator()(FILE* p) const { if (p) fclose(p); }
	};

	using FramePtr = std::unique_ptr<AVFrame, AVFrameDeleter>;
	using PacketPtr = std::unique_ptr<AVPacket, AVPacketDeleter>;
	using CodecContextPtr = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;
	using FormatContextPtr = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;
	using FormatOutputContextPtr = std::unique_ptr<AVFormatContext, AVFormatOutputContextDeleter>;
	using FilterGraphPtr = std::unique_ptr<AVFilterGraph, AVFilterGraphDeleter>;
	using SwrContextPtr = std::unique_ptr<SwrContext, SwrContextDeleter>;
	using SwsContextPtr = std::unique_ptr<SwsContext, SwsContextDeleter>;
	using AudioFifoPtr = std::unique_ptr<AVAudioFifo, AVAudioFifoDeleter>;
	using FifoPtr = std::unique_ptr<AVFifo, AVFifoDeleter>;
	using DictionaryPtr = std::unique_ptr<AVDictionary, AVDictionaryDeleter>;
	using ParserContextPtr = std::unique_ptr<AVCodecParserContext, AVParserContextDeleter>;
	using FilePtr = std::unique_ptr<FILE, FileCloser>;
}
