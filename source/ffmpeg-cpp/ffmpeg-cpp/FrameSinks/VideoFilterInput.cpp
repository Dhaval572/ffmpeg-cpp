#include "VideoFilterInput.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
	VideoFilterInput::VideoFilterInput()
	{
		frame_queue.reset(av_fifo_alloc2(8, sizeof(AVFrame*), AV_FIFO_FLAG_AUTO_GROW));
		if (!frame_queue)
		{
			throw FFmpegException("Failed to allocate fifo queue for filter input");
		}
	}

	VideoFilterInput::~VideoFilterInput()
	{
		AVFrame* tmp = nullptr;
		while (av_fifo_can_read(frame_queue.get()) > 0)
		{
			av_fifo_read(frame_queue.get(), &tmp, 1);
			if (tmp) av_frame_free(&tmp);
		}
	}

	void VideoFilterInput::WriteFrame(AVFrame* frame)
	{
		AVFrame* tmp = av_frame_clone(frame);
		if (!tmp) throw FFmpegException("Failed to clone frame");

		av_fifo_write(frame_queue.get(), &tmp, 1);
		frameReceived = true;
	}

	bool VideoFilterInput::HasFrame()
	{
		return frameReceived;
	}

	bool VideoFilterInput::FetchFrame(AVFrame** frame)
	{
		if (av_fifo_can_read(frame_queue.get()) == 0) return false;

		AVFrame* tmp = nullptr;
		av_fifo_read(frame_queue.get(), &tmp, 1);

		*frame = tmp;

		return true;
	}

	bool VideoFilterInput::PeekFrame(AVFrame** frame)
	{
		if (av_fifo_can_read(frame_queue.get()) == 0) return false;

		AVFrame* tmp = nullptr;
		av_fifo_peek(frame_queue.get(), &tmp, 1, 0);

		*frame = tmp;

		return true;
	}

	void VideoFilterInput::SetMetaData(StreamData* metaData)
	{
		this->metaData = metaData;
	}

	StreamData* VideoFilterInput::GetMetaData()
	{
		return metaData;
	}

	void VideoFilterInput::Close()
	{
		closed = true;
	}

	bool VideoFilterInput::IsClosed()
	{
		return closed;
	}
}
