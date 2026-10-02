#pragma once

#include "ffmpeg.h"

#include "Muxing/Muxer.h"

#include "Sources/Demuxer.h"
#include "Sources/InputSource.h"
#include "Sources/RawAudioFileSource.h"
#include "Sources/RawVideoFileSource.h"
#include "Sources/RawAudioDataSource.h"
#include "Sources/RawVideoDataSource.h"
#include "Sources/EncodedFileSource.h"

#include "FrameSinks/VideoEncoder.h"
#include "FrameSinks/AudioEncoder.h"
#include "FrameSinks/Filter.h"

#include "Codecs/AudioCodec.h"

#include "Codecs/VP9Codec.h"
#include "Codecs/PNGCodec.h"
#include "Codecs/JPGCodec.h"
#include "Codecs/Codec.h"

#include "FFmpegException.h"

#ifdef FFMPEGCPP_WITH_NVENC
#include "Codecs/H264NVEncCodec.h"
#include "Codecs/H265NVEncCodec.h"
#endif
