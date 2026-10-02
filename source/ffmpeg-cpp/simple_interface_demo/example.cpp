
#include "SimpleInterface.h"

#include <iostream>

int main()
{
	void* handle = ffmpegCppCreate("out.mp4");
	if (ffmpegCppIsError(handle))
	{
		std::cerr << "Error: " << ffmpegCppGetError(handle) << std::endl;
		ffmpegCppClose(handle);
		return 1;
	}

	ffmpegCppAddVideoStream(handle, "samples/big_buck_bunny.mp4");
	ffmpegCppAddVideoFilter(handle, "transpose=cclock[middle];[middle]vignette");
	ffmpegCppAddAudioStream(handle, "samples/big_buck_bunny.mp4");

	if (ffmpegCppIsError(handle))
	{
		std::cerr << "Error: " << ffmpegCppGetError(handle) << std::endl;
		ffmpegCppClose(handle);
		return 1;
	}

	ffmpegCppGenerate(handle);
	if (ffmpegCppIsError(handle))
	{
		std::cerr << "Error: " << ffmpegCppGetError(handle) << std::endl;
		ffmpegCppClose(handle);
		return 1;
	}

	ffmpegCppClose(handle);
	std::cout << "Done!" << std::endl;
	return 0;
}
