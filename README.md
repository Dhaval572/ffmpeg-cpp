# ffmpeg-cpp

A clean C++ wrapper around the FFmpeg libraries. Makes the most commonly used functionality of FFmpeg easily available for any C++ project with an easy-to-use interface.

Builds on **Windows, Linux, and macOS** using CMake. Requires **FFmpeg 6.0 or newer**.

# Quick Start

```bash
git clone <https://github.com/Dhaval572/ffmpeg-cpp.git>
cd ffmpeg-cpp
make
```

Then run an example: `make demo` · `make help`

# Using in your project

Add ffmpeg-cpp to your CMake project in one of three ways:

**Vendored / submodule:**
```cmake
add_subdirectory(ffmpeg-cpp)
target_link_libraries(myapp PRIVATE ffmpeg-cpp::ffmpeg-cpp)
```

**FetchContent:**
```cmake
include(FetchContent)
FetchContent_Declare(ffmpeg-cpp GIT_REPOSITORY <https://github.com/Dhaval572/ffmpeg-cpp.git> GIT_TAG main)
FetchContent_MakeAvailable(ffmpeg-cpp)
target_link_libraries(myapp PRIVATE ffmpeg-cpp::ffmpeg-cpp)
```

**Installed:**
```bash
cmake -B build && cmake --build build && cmake --install build
```
```cmake
find_package(ffmpeg-cpp REQUIRED)
target_link_libraries(myapp PRIVATE ffmpeg-cpp::ffmpeg-cpp)
```

## Example: extract audio from a video

```cpp
#include <ffmpegcpp.h>

using namespace ffmpegcpp;

int main()
{
    Muxer muxer("audio_only.m4a");

    AudioCodec codec(AV_CODEC_ID_AAC);
    AudioEncoder encoder(&codec, &muxer);

    Demuxer demuxer("input.mp4");
    demuxer.DecodeBestAudioStream(&encoder);

    demuxer.PreparePipeline();
    while (!demuxer.IsDone())
    {
        demuxer.Step();
    }

    muxer.Close();
    return 0;
}
```

Consumer `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(myapp)
set(CMAKE_CXX_STANDARD 20)
find_package(ffmpeg-cpp REQUIRED)
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE ffmpeg-cpp::ffmpeg-cpp)
```

The pipeline pattern is always the same: create sources/sinks, connect them, call `PreparePipeline()`, loop `Step()` until done, then `Close()`. See the [API Reference](#api-reference) for all classes.

# Requirements

| Tool           | Linux (Debian/Ubuntu)              | Linux (Fedora)             | macOS                    | Windows                                         |
| -------------- | ---------------------------------- | -------------------------- | ------------------------ | ----------------------------------------------- |
| Git            | `sudo apt install git`             | `sudo dnf install git`     | pre-installed            | [git-scm.com](https://git-scm.com/download/win) |
| CMake 3.16+    | `sudo apt install cmake`           | `sudo dnf install cmake`   | `brew install cmake`     | bundled with VS 2019+                           |
| C++20 compiler | `sudo apt install build-essential` | `sudo dnf install gcc-c++` | `xcode-select --install` | VS 2019+ with C++ workload                      |
| FFmpeg 6.0+    | `sudo apt install ffmpeg-devel`    | `sudo dnf install ffmpeg-devel` | `brew install ffmpeg` | [ffmpeg.org](https://ffmpeg.org/download.html)  |

FFmpeg is found via `pkg-config`. Install development libraries with the commands above.

# Make targets

| Command         | What it does                                |
| --------------- | ------------------------------------------- |
| `make`          | Build the library and examples              |
| `make <example>`| Build (if needed) and run an example        |
| `make test`     | Run unit tests (Catch2)                     |
| `make setup`    | Install vcpkg dependencies (Catch2)         |
| `make clean`    | Remove the build directory                  |
| `make help`     | List all targets                            |

Examples: `print_info` `decode_audio` `decode_video` `encode_audio` `encode_video` `filtering_audio` `filtering_video` `remuxing` `difference` `demo` `simple_interface_demo`

Environment variables: `CONFIG` (build type) · `EXTRA` (extra CMake flags) · `VCPKG_ROOT`

# Testing

```bash
make test
```

Uses [Catch2 v3](https://github.com/catchorg/Catch2) via vcpkg. Tests live in `tests/` — to add one, append to `TEST_SOURCES` in `tests/CMakeLists.txt`.

# Building with CMake directly

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

| Option                             | Default | Description                                    |
| ---------------------------------- | ------- | ---------------------------------------------- |
| `FFMPEGCPP_WITH_NVENC`             | `OFF`   | Enable NVIDIA NVENC H.264/H.265 codec wrappers |
| `FFMPEGCPP_BUILD_EXAMPLES`         | `ON`    | Build the example programs                     |
| `FFMPEGCPP_BUILD_SIMPLE_INTERFACE` | `ON`    | Build the `simple_interface` shared library    |

---

# API Reference

Include everything with a single header:

```cpp
#include <ffmpegcpp.h>
```

All classes live in the `ffmpegcpp` namespace. All functions use FFmpeg types (`AVCodecID`, `AVPixelFormat`, `AVSampleFormat`, `AVRational`, etc.) directly.

## Data flow

```
InputSource (Demuxer / RawAudioFileSource / RawVideoFileSource / EncodedFileSource)
    |
    v
FrameSink::CreateStream()  -->  FrameSinkStream::WriteFrame(AVFrame*, StreamData*)
    |
    +-- Filter(filterString, target)         // FFmpeg filter graph
    +-- VideoEncoder(VideoCodec*, Muxer*)    // encodes --> Muxer
    +-- AudioEncoder(AudioCodec*, Muxer*)    // encodes --> Muxer
    |
    v
Muxer::Close()    // writes container trailer
```

Typical pipeline:

```cpp
Muxer       muxer("output.mp4");
VideoCodec  codec(AV_CODEC_ID_MPEG2VIDEO);
VideoEncoder encoder(&codec, &muxer);
Filter      filter("scale=640:150,transpose=cclock", &encoder);
Demuxer     demuxer("input.mp4");

demuxer.DecodeBestVideoStream(&filter);
demuxer.PreparePipeline();
while (!demuxer.IsDone())
{
    demuxer.Step();
}
muxer.Close();
```

## FFmpegException

All errors are thrown as `FFmpegException`, which inherits from `std::exception`.

```cpp
class FFmpegException : public std::exception
{
public:
    explicit FFmpegException(std::string error);
    FFmpegException(std::string error, int returnValue);  // appends av_strerror(returnValue)
    const char* what() const noexcept override;
};
```

## Codec classes

### Codec (base)

```cpp
class Codec
{
public:
    Codec(const char* codecName);
    Codec(AVCodecID codecId);
    virtual ~Codec();

    void SetOption(const char* name, const char* value);
    void SetOption(const char* name, int value);
    void SetOption(const char* name, double value);
    void SetGenericOption(const char* name, const char* value);
    void SetGlobalContainerHeader();
};
```

### AudioCodec

```cpp
class AudioCodec : public Codec
{
public:
    AudioCodec(const char* codecName);
    AudioCodec(AVCodecID codecId);

    OpenCodec* Open(int bitRate, AVSampleFormat format, int sampleRate);

    bool IsFormatSupported(AVSampleFormat format);
    bool IsChannelsSupported(int channels);
    bool IsSampleRateSupported(int sampleRate);

    AVSampleFormat GetDefaultSampleFormat();
    int GetDefaultSampleRate();
};
```

### VideoCodec

```cpp
class VideoCodec : public Codec
{
public:
    VideoCodec(const char* codecName);
    VideoCodec(AVCodecID codecId);

    OpenCodec* Open(int width, int height, AVRational* frameRate, AVPixelFormat format);

    void SetQualityScale(int qscale);

    bool IsPixelFormatSupported(AVPixelFormat format);
    bool IsFrameRateSupported(AVRational* frameRate);

    AVPixelFormat GetDefaultPixelFormat();
    AVRational GetClosestSupportedFrameRate(AVRational frameRate);
};
```

### VP9Codec

```cpp
class VP9Codec : public VideoCodec
{
public:
    VP9Codec();
    void SetDeadline(const char* deadline);   // "realtime", "good", "best"
    void SetCpuUsed(int cpuUsed);             // 0-8, higher = faster
    void SetLossless(bool lossless);
    void SetCrf(int crf);                     // quality, lower = better
};
```

### PNGCodec / JPGCodec

```cpp
class PNGCodec : public VideoCodec
{
public:
    PNGCodec();
    void SetCompressionLevel(int compressionLevel);
};

class JPGCodec : public VideoCodec
{
public:
    JPGCodec();
    void SetCompressionLevel(int compressionLevel);
};
```

### Hardware encoders (optional)

Available when built with `-DFFMPEGCPP_WITH_NVENC=ON` and an NVIDIA GPU:

```cpp
class H264NVEncCodec : public VideoCodec
{
public:
    H264NVEncCodec();
    void SetPreset(const char* preset);
};

class H265NVEncCodec : public VideoCodec
{
public:
    H265NVEncCodec();
    void SetPreset(const char* preset);
};
```

## Demuxer

Opens a media file and decodes its streams.

```cpp
class Demuxer : public InputSource
{
public:
    explicit Demuxer(const char* fileName);
    Demuxer(const char* fileName, const AVInputFormat* inputFormat,
            AVDictionary* inputFormatOptions);

    void DecodeBestAudioStream(FrameSink* frameSink);
    void DecodeBestVideoStream(FrameSink* frameSink);

    void DecodeAudioStream(int streamId, FrameSink* frameSink);
    void DecodeVideoStream(int streamId, FrameSink* frameSink);

    void PreparePipeline();   // decode a few frames so downstream can configure
    bool IsDone();
    void Step();              // decode the next packet

    ContainerInfo GetInfo();
    int GetFrameCount(int streamId);
    const char* GetFileName();
};
```

`PreparePipeline` / `IsDone` / `Step` are the core pump loop. Call `PreparePipeline` once, then loop on `Step` until `IsDone` returns true.

## Muxer

Writes encoded packets to an output file. Takes ownership of the `OutputStream*` objects added to it.

```cpp
class Muxer
{
public:
    explicit Muxer(const char* fileName);

    void AddOutputStream(OutputStream* stream);
    void WritePacket(AVPacket* pkt);
    void Close();
    bool IsPrimed();

    const AVCodec* GetDefaultVideoFormat();
    const AVCodec* GetDefaultAudioFormat();
};
```

## Encoders

Encoders are both a `FrameSink` (so you can feed frames into them) and a `FrameWriter` (so they can be targets of a `Filter`). The codec is opened lazily on the first frame.

### VideoEncoder

```cpp
class VideoEncoder : public VideoFrameSink, public FrameWriter
{
public:
    VideoEncoder(VideoCodec* codec, Muxer* muxer);
    VideoEncoder(VideoCodec* codec, Muxer* muxer, AVPixelFormat format);
    VideoEncoder(VideoCodec* codec, Muxer* muxer, AVRational frameRate);
    VideoEncoder(VideoCodec* codec, Muxer* muxer, AVRational frameRate, AVPixelFormat format);

    FrameSinkStream* CreateStream();
};
```

### AudioEncoder

```cpp
class AudioEncoder : public AudioFrameSink, public ConvertedAudioProcessor, public FrameWriter
{
public:
    AudioEncoder(AudioCodec* codec, Muxer* muxer);
    AudioEncoder(AudioCodec* codec, Muxer* muxer, int bitRate);

    FrameSinkStream* CreateStream();
};
```

## Filter

Applies an FFmpeg filter graph string. Each `CreateStream()` call adds one input. Uses standard FFmpeg filter syntax.

```cpp
class Filter : public FrameSink, public FrameWriter
{
public:
    Filter(const char* filterString, FrameSink* target);

    FrameSinkStream* CreateStream();
};
```

Examples:

```cpp
new Filter("scale=640:150", encoder);
new Filter("transpose=cclock[middle];[middle]vignette", encoder);
new Filter("crop=1080:1920:740:0,transpose=3", encoder);
```

## Input sources

### RawAudioFileSource

Reads a raw PCM file (no header) and feeds it to a `FrameSink`.

```cpp
class RawAudioFileSource : public InputSource
{
public:
    RawAudioFileSource(const char* fileName, const char* inputFormat,
                       int sampleRate, int channels, FrameSink* frameSink);
    // inputFormat: "s16le", "fltp", etc.

    void PreparePipeline();
    bool IsDone();
    void Step();
};
```

### RawVideoFileSource

Reads a raw video file (e.g. raw H.264) and feeds it to a `FrameSink`.

```cpp
class RawVideoFileSource : public InputSource
{
public:
    RawVideoFileSource(const char* fileName, FrameSink* frameSink);

    void PreparePipeline();
    bool IsDone();
    void Step();
};
```

### EncodedFileSource

Decodes an encoded audio/video file directly into a `FrameSink`.

```cpp
class EncodedFileSource : public InputSource
{
public:
    EncodedFileSource(const char* inFileName, AVCodecID codecId, FrameSink* output);
    EncodedFileSource(const char* inFileName, const char* codecName, FrameSink* output);

    void PreparePipeline();
    bool IsDone();
    void Step();
};
```

### RawAudioDataSource / RawVideoDataSource

Feed raw buffers from memory (no file).

```cpp
class RawAudioDataSource
{
public:
    RawAudioDataSource(AVSampleFormat sampleFormat, int sampleRate,
                       int channels, FrameSink* output);
    RawAudioDataSource(AVSampleFormat sampleFormat, int sampleRate,
                       int channels, int64_t channelLayout, FrameSink* output);

    void WriteData(void* data, int sampleCount);
    void Close();
    bool IsPrimed();
};

class RawVideoDataSource
{
public:
    RawVideoDataSource(int width, int height, AVPixelFormat pixelFormat,
                       int framesPerSecond, FrameSink* output);
    RawVideoDataSource(int width, int height, AVPixelFormat sourcePixelFormat,
                       AVPixelFormat targetPixelFormat,
                       int framesPerSecond, FrameSink* output);

    void WriteFrame(void* data, int bytesPerRow);
    void Close();
    int GetWidth();
    int GetHeight();
    bool IsPrimed();
};
```

## Info structs

Returned by `Demuxer::GetInfo()`.

```cpp
struct AudioStreamInfo
{
    int id;
    AVRational timeBase;
    const AVCodec* codec;
    float bitRate;
    int sampleRate;
    int channels;
    uint64_t channelLayout;
    char channelLayoutName[255];
};

struct VideoStreamInfo
{
    int id;
    AVRational frameRate;
    AVRational timeBase;
    const AVCodec* codec;
    float bitRate;
    AVPixelFormat format;
    const char* formatName;
    int width, height;
};

struct ContainerInfo
{
    long durationInMicroSeconds;
    float durationInSeconds;
    float start;
    float bitRate;
    const AVInputFormat* format;
    std::vector<VideoStreamInfo> videoStreams;
    std::vector<AudioStreamInfo> audioStreams;
};
```

---

# Simple Interface (C API)

A small C-compatible shared library for FFI, .NET, or any language that can call C functions. Header: `simple_interface/SimpleInterface.h`.

```c
void*       ffmpegCppCreate(const char* outputFileName);
void        ffmpegCppAddVideoStream(void* handle, const char* videoFileName);
void        ffmpegCppAddAudioStream(void* handle, const char* audioFileName);
void        ffmpegCppAddVideoFilter(void* handle, const char* filterString);
void        ffmpegCppAddAudioFilter(void* handle, const char* filterString);
void        ffmpegCppGenerate(void* handle);
bool        ffmpegCppIsError(void* handle);
const char* ffmpegCppGetError(void* handle);
void        ffmpegCppClose(void* handle);
```

Usage:

```c
#include "SimpleInterface.h"

void* handle = ffmpegCppCreate("out.mp4");
ffmpegCppAddVideoStream(handle, "samples/big_buck_bunny.mp4");
ffmpegCppAddVideoFilter(handle, "transpose=cclock[middle];[middle]vignette");
ffmpegCppAddAudioStream(handle, "samples/big_buck_bunny.mp4");
ffmpegCppGenerate(handle);

if (ffmpegCppIsError(handle))
    printf("Error: %s\n", ffmpegCppGetError(handle));

ffmpegCppClose(handle);
```

## C\#

P/Invoke declarations for the `simple_interface` shared library — see `SimpleInterface.h` for the full signatures. All functions use `CallingConvention.Cdecl` and return `IntPtr` for the handle.

---

# Why?

I developed this project to be able to integrate FFmpeg into our program without having to call the executable to do an operation. This is important because starting up an external executable tends to be blocked by antivirus software and can cause issues with users. It has been tested for the most common functionality, and some of the examples from https://github.com/FFmpeg/FFmpeg/tree/master/doc/examples are mirrored in the project as well.

# Roadmap

- Testing with more codecs, containers

# License

This library is licensed under LGPL (https://en.wikipedia.org/wiki/GNU_Lesser_General_Public_License).

Please note though that FFmpeg, which you will need to build this library, is not. Depending on how you build it, it is either LGPL or GPL. So if you use the GPL-version of FFmpeg in your project, this library will be GPL too.
