#pragma once

// Portable export macro for the simple_interface C API.
#if defined(_WIN32)
	#define FFMPEGCPP_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
	#define FFMPEGCPP_EXPORT __attribute__((visibility("default")))
#else
	#define FFMPEGCPP_EXPORT
#endif

extern "C" FFMPEGCPP_EXPORT void* ffmpegCppCreate(const char* outputFileName);

extern "C" FFMPEGCPP_EXPORT void ffmpegCppAddVideoStream(void* handle, const char* videoFileName);
extern "C" FFMPEGCPP_EXPORT void ffmpegCppAddAudioStream(void* handle, const char* audioFileName);

extern "C" FFMPEGCPP_EXPORT void ffmpegCppAddVideoFilter(void* handle, const char* filterString);
extern "C" FFMPEGCPP_EXPORT void ffmpegCppAddAudioFilter(void* handle, const char* filterString);

extern "C" FFMPEGCPP_EXPORT void ffmpegCppGenerate(void* handle);

extern "C" FFMPEGCPP_EXPORT bool ffmpegCppIsError(void* handle);
extern "C" FFMPEGCPP_EXPORT const char* ffmpegCppGetError(void* handle);

extern "C" FFMPEGCPP_EXPORT void ffmpegCppClose(void* handle);
