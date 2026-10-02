#pragma once

#include <string>
#include <exception>

namespace ffmpegcpp
{
	class FFmpegException : public std::exception
	{
	public:

		explicit FFmpegException(std::string error);
		FFmpegException(std::string error, int returnValue);

		const char* what() const noexcept override
		{
			return message.c_str();
		}

	private:

		std::string message;
	};
}
