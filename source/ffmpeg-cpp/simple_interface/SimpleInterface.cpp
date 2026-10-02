#include "SimpleInterface.h"

#include <string>
#include <vector>
#include <cstring>

#include "ffmpegcpp.h"
#include "ffmpeg.h"

using namespace ffmpegcpp;

template<class CodecType, class FrameSinkType>
struct StreamContext
{
	CodecType* codec = nullptr;

	std::string sourceFileName;
	Demuxer* demuxer = nullptr;

	bool hasFilter = false;
	std::string filterString;
	Filter* filter = nullptr;

	FrameSinkType* encoder = nullptr;

	void CleanUp()
	{
		if (encoder != nullptr) { delete encoder; encoder = nullptr; }
		if (filter != nullptr) { delete filter; filter = nullptr; }
		if (codec != nullptr) { delete codec; codec = nullptr; }
	}
};

struct Context
{
	std::string outputFileName;
	Muxer* muxer = nullptr;

	StreamContext<VideoCodec, VideoFrameSink> videoContext;
	StreamContext<AudioCodec, AudioFrameSink> audioContext;

	std::vector<Demuxer*> uniqueDemuxers;

	bool errored = false;
	std::string error;
};

static void SetError(Context* ctx, const std::string& error)
{
	ctx->errored = true;
	ctx->error = error;
}

static void CleanUp(Context* ctx)
{
	if (ctx->muxer != nullptr) { delete ctx->muxer; ctx->muxer = nullptr; }

	ctx->videoContext.CleanUp();
	ctx->audioContext.CleanUp();

	for (size_t i = 0; i < ctx->uniqueDemuxers.size(); ++i)
	{
		delete ctx->uniqueDemuxers[i];
	}
	ctx->uniqueDemuxers.clear();

	delete ctx;
}

static Demuxer* GetExistingDemuxer(Context* ctx, const char* fileName)
{
	for (size_t i = 0; i < ctx->uniqueDemuxers.size(); ++i)
	{
		if (std::string(ctx->uniqueDemuxers[i]->GetFileName()) == std::string(fileName)) return ctx->uniqueDemuxers[i];
	}
	return nullptr;
}

extern "C" FFMPEGCPP_EXPORT void* ffmpegCppCreate(const char* outputFileName)
{
	Context* ctx = new Context();
	try
	{
		ctx->outputFileName = std::string(outputFileName);
		ctx->muxer = new Muxer(ctx->outputFileName.c_str());
		return ctx;
	}
	catch (const FFmpegException& e)
	{
		SetError(ctx, "Failed to create output file " + ctx->outputFileName + ": " + e.what());
		CleanUp(ctx);
		return nullptr;
	}
}

extern "C" FFMPEGCPP_EXPORT void ffmpegCppAddVideoStream(void* handle, const char* videoFileName)
{
	Context* ctx = (Context*)handle;
	try
	{
		ctx->videoContext.sourceFileName = videoFileName;
		ctx->videoContext.demuxer = GetExistingDemuxer(ctx, videoFileName);
		if (ctx->videoContext.demuxer == nullptr)
		{
			ctx->videoContext.demuxer = new Demuxer(ctx->videoContext.sourceFileName.c_str());
			ctx->uniqueDemuxers.push_back(ctx->videoContext.demuxer);
		}

		ctx->videoContext.codec = new VideoCodec(ctx->muxer->GetDefaultVideoFormat()->id);
		ctx->videoContext.encoder = new VideoEncoder(ctx->videoContext.codec, ctx->muxer);
	}
	catch (const FFmpegException& e)
	{
		SetError(ctx, "Failed to add video stream " + std::string(videoFileName) + ": " + e.what());
	}
}

extern "C" FFMPEGCPP_EXPORT void ffmpegCppAddAudioStream(void* handle, const char* audioFileName)
{
	Context* ctx = (Context*)handle;
	try
	{
		ctx->audioContext.sourceFileName = audioFileName;
		ctx->audioContext.demuxer = GetExistingDemuxer(ctx, audioFileName);
		if (ctx->audioContext.demuxer == nullptr)
		{
			ctx->audioContext.demuxer = new Demuxer(ctx->audioContext.sourceFileName.c_str());
			ctx->uniqueDemuxers.push_back(ctx->audioContext.demuxer);
		}

		ctx->audioContext.codec = new AudioCodec(ctx->muxer->GetDefaultAudioFormat()->id);
		ctx->audioContext.encoder = new AudioEncoder(ctx->audioContext.codec, ctx->muxer);
	}
	catch (const FFmpegException& e)
	{
		SetError(ctx, "Failed to add audio stream " + std::string(audioFileName) + ": " + e.what());
	}
}

extern "C" FFMPEGCPP_EXPORT void ffmpegCppAddVideoFilter(void* handle, const char* filterString)
{
	Context* ctx = (Context*)handle;
	ctx->videoContext.filterString = filterString;
	ctx->videoContext.hasFilter = true;
}

extern "C" FFMPEGCPP_EXPORT void ffmpegCppAddAudioFilter(void* handle, const char* filterString)
{
	Context* ctx = (Context*)handle;
	ctx->audioContext.filterString = filterString;
	ctx->audioContext.hasFilter = true;
}

extern "C" FFMPEGCPP_EXPORT void ffmpegCppGenerate(void* handle)
{
	Context* ctx = (Context*)handle;
	try
	{
		FrameSink* videoFrameSink = ctx->videoContext.encoder;
		FrameSink* audioFrameSink = ctx->audioContext.encoder;
		if (ctx->videoContext.hasFilter && videoFrameSink != nullptr)
		{
			ctx->videoContext.filter = new Filter(ctx->videoContext.filterString.c_str(), videoFrameSink);
			videoFrameSink = ctx->videoContext.filter;
		}
		if (ctx->audioContext.hasFilter && audioFrameSink != nullptr)
		{
			ctx->audioContext.filter = new Filter(ctx->audioContext.filterString.c_str(), audioFrameSink);
			audioFrameSink = ctx->audioContext.filter;
		}

		if (ctx->videoContext.demuxer != nullptr && videoFrameSink != nullptr)
		{
			ctx->videoContext.demuxer->DecodeBestVideoStream(videoFrameSink);
		}
		if (ctx->audioContext.demuxer != nullptr && audioFrameSink != nullptr)
		{
			ctx->audioContext.demuxer->DecodeBestAudioStream(audioFrameSink);
		}

		for (size_t i = 0; i < ctx->uniqueDemuxers.size(); ++i)
		{
			ctx->uniqueDemuxers[i]->PreparePipeline();
		}

		for (size_t i = 0; i < ctx->uniqueDemuxers.size(); ++i)
		{
			Demuxer* demuxer = ctx->uniqueDemuxers[i];
			while (!demuxer->IsDone()) demuxer->Step();
		}

		ctx->muxer->Close();
	}
	catch (const FFmpegException& e)
	{
		SetError(ctx, "Failed to generate output file: " + std::string(e.what()));
	}
}

extern "C" FFMPEGCPP_EXPORT bool ffmpegCppIsError(void* handle)
{
	return ((Context*)handle)->errored;
}

extern "C" FFMPEGCPP_EXPORT const char* ffmpegCppGetError(void* handle)
{
	Context* ctx = (Context*)handle;
	return ctx->error.c_str();
}

extern "C" FFMPEGCPP_EXPORT void ffmpegCppClose(void* handle)
{
	CleanUp((Context*)handle);
}
