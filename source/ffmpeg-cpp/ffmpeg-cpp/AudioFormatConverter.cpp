#include "AudioFormatConverter.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	AudioFormatConverter::AudioFormatConverter(ConvertedAudioProcessor* writer, AVCodecContext* codecContext)
	{
		this->output = writer;
		this->codecContext = codecContext;

		converted_frame.reset(av_frame_alloc());
		if (!converted_frame)
		{
			throw FFmpegException("Error allocating an audio frame");
		}

		int nb_samples;
		if (codecContext->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
			nb_samples = 10000;
		else
			nb_samples = codecContext->frame_size;

		converted_frame->format = codecContext->sample_fmt;
		av_channel_layout_copy(&converted_frame->ch_layout, &codecContext->ch_layout);
		converted_frame->sample_rate = codecContext->sample_rate;
		converted_frame->nb_samples = nb_samples;
		if (nb_samples)
		{
			int ret = av_frame_get_buffer(converted_frame.get(), 0);
			if (ret < 0)
			{
				throw FFmpegException("Error allocating an audio buffer", ret);
			}
		}

		tmp_frame.reset(av_frame_alloc());
		if (!tmp_frame)
		{
			throw FFmpegException("Error allocating an audio frame");
		}
		tmp_frame->format = codecContext->sample_fmt;
		av_channel_layout_copy(&tmp_frame->ch_layout, &codecContext->ch_layout);
		tmp_frame->sample_rate = codecContext->sample_rate;
		tmp_frame->nb_samples = 0;

		fifo.reset(av_audio_fifo_alloc(codecContext->sample_fmt, codecContext->ch_layout.nb_channels, nb_samples > 0 ? nb_samples : 1));
		if (!fifo)
		{
			throw FFmpegException("Failed to create FIFO queue for audio format converter");
		}
	}

	AudioFormatConverter::~AudioFormatConverter()
	{
	}

	void AudioFormatConverter::InitDelayed(AVFrame* frame)
	{
		SwrContext* raw_swr = nullptr;
		int ret = swr_alloc_set_opts2(&raw_swr,
			&codecContext->ch_layout, codecContext->sample_fmt, codecContext->sample_rate,
			&frame->ch_layout, (AVSampleFormat)frame->format, frame->sample_rate,
			0, NULL);
		if (ret < 0 || !raw_swr)
		{
			throw FFmpegException("Could not allocate resampler context", ret);
		}
		swr_ctx.reset(raw_swr);

		in_sample_rate = frame->sample_rate;
		out_sample_rate = codecContext->sample_rate;

		if ((ret = swr_init(swr_ctx.get())) < 0)
		{
			throw FFmpegException("Failed to initialize the resampling context", ret);
		}
	}

	void AudioFormatConverter::ProcessFrame(AVFrame* frame)
	{
		if (!initialized)
		{
			InitDelayed(frame);
			initialized = true;
		}

		int ret;
		ret = swr_convert_frame(swr_ctx.get(), tmp_frame.get(), frame);
		if (ret < 0)
		{
			throw FFmpegException("Error while converting audio frame to destination format", ret);
		}

		while (tmp_frame->nb_samples > 0)
		{
			AddToFifo(tmp_frame.get());
			ret = swr_convert_frame(swr_ctx.get(), tmp_frame.get(), NULL);
			if (ret < 0)
			{
				throw FFmpegException("Error while converting audio frame to destination format", ret);
			}
		}

		bool finished = (frame == NULL);
		int fifoSize = av_audio_fifo_size(fifo.get());
		while (fifoSize >= converted_frame->nb_samples ||
			(finished && fifoSize > 0))
		{
			PullConvertedFrameFromFifo();
			fifoSize = av_audio_fifo_size(fifo.get());
		}
	}

	void AudioFormatConverter::AddToFifo(AVFrame* frame)
	{
		int ret;
		if ((ret = av_audio_fifo_realloc(fifo.get(), av_audio_fifo_size(fifo.get()) + frame->nb_samples)) < 0)
		{
			throw FFmpegException("Could not reallocate FIFO", ret);
		}

		if (av_audio_fifo_write(fifo.get(), (void**)frame->extended_data, frame->nb_samples) < frame->nb_samples)
		{
			throw FFmpegException("Could not write data to FIFO");
		}
	}

	void AudioFormatConverter::PullConvertedFrameFromFifo()
	{
		const int frame_size = FFMIN(av_audio_fifo_size(fifo.get()), converted_frame->nb_samples);
		converted_frame->nb_samples = frame_size;

		int ret;
		if ((ret = av_audio_fifo_read(fifo.get(), (void**)converted_frame->data, frame_size)) < frame_size)
		{
			throw FFmpegException("Could not read data from FIFO", ret);
		}

		WriteCompleteConvertedFrame();
	}

	void AudioFormatConverter::WriteCompleteConvertedFrame()
	{
		AVRational inv_sample_rate;
		inv_sample_rate.num = 1;
		inv_sample_rate.den = codecContext->sample_rate;

		converted_frame->pts = av_rescale_q(samples_count, inv_sample_rate, codecContext->time_base);
		samples_count += converted_frame->nb_samples;

		output->WriteConvertedFrame(converted_frame.get());

		int ret = av_frame_make_writable(converted_frame.get());
		if (ret < 0)
		{
			throw FFmpegException("Failed to make audio frame writable", ret);
		}
	}
}
