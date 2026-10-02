#pragma once

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/error.h>
	#include <libavutil/opt.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/audio_fifo.h>
	#include <libavutil/fifo.h>
	#include <libavutil/timestamp.h>
	#include <libavformat/avformat.h>
	#include <libavfilter/avfilter.h>
	#include <libavfilter/buffersink.h>
	#include <libavfilter/buffersrc.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
}
