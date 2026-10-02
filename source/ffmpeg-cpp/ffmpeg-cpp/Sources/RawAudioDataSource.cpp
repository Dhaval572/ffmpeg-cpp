#include "RawAudioDataSource.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	static uint64_t DefaultChannelMask(int channels)
	{
		AVChannelLayout layout{};
		av_channel_layout_default(&layout, channels);
		uint64_t mask = (layout.order == AV_CHANNEL_ORDER_NATIVE) ? layout.u.mask : 0;
		av_channel_layout_uninit(&layout);
		return mask;
	}

	RawAudioDataSource::RawAudioDataSource(AVSampleFormat sampleFormat, int sampleRate, int channels, FrameSink* output)
		: RawAudioDataSource(sampleFormat, sampleRate, channels, (int64_t)DefaultChannelMask(channels), output)
	{
	}

	RawAudioDataSource::RawAudioDataSource(AVSampleFormat sampleFormat, int sampleRate, int channels, int64_t channelLayout, FrameSink* output)
	{
		this->output = output->CreateStream();

		frame.reset(av_frame_alloc());
		if (!frame)
		{
			throw FFmpegException("Could not allocate video frame");
		}

		frame->format = sampleFormat;
		frame->sample_rate = sampleRate;
		if (channelLayout != 0)
		{
			av_channel_layout_from_mask(&frame->ch_layout, (uint64_t)channelLayout);
		}
		if (frame->ch_layout.nb_channels == 0)
		{
			av_channel_layout_default(&frame->ch_layout, channels);
		}
		frame->nb_samples = 735;

		int ret = av_frame_get_buffer(frame.get(), 0);
		if (ret < 0)
		{
			throw FFmpegException("Could not allocate the video frame data", ret);
		}
	}

	RawAudioDataSource::~RawAudioDataSource()
	{
	}

	void RawAudioDataSource::CleanUp()
	{
	}

	void RawAudioDataSource::WriteData(void* data, int sampleCount)
	{
		frame->nb_samples = sampleCount;

		int ret = av_frame_make_writable(frame.get());
		if (ret < 0)
		{
			throw FFmpegException("Failed to make audio frame writable", ret);
		}

		int bytesPerSample = av_get_bytes_per_sample((AVSampleFormat)frame->format);
		memcpy(*frame->data, data, frame->nb_samples * frame->ch_layout.nb_channels * bytesPerSample);

		if (metaData == nullptr)
		{
			metaData = std::make_unique<StreamData>();
			metaData->type = AVMEDIA_TYPE_AUDIO;
		}

		output->WriteFrame(frame.get(), metaData.get());
	}

	void RawAudioDataSource::Close()
	{
		output->Close();
	}

	bool RawAudioDataSource::IsPrimed()
	{
		return output->IsPrimed();
	}
}
