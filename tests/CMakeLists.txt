find_package(Catch2 3 REQUIRED)

enable_testing()
include(Catch)

set(TEST_SOURCES
	test_exception.cpp
	test_info_structs.cpp
	test_codec_deducer.cpp
	test_audio_codec.cpp
	test_video_codec.cpp
	test_codecs.cpp
	test_muxer.cpp
	test_deleters.cpp
)

add_executable(ffmpeg-cpp-tests ${TEST_SOURCES})
target_link_libraries(ffmpeg-cpp-tests PRIVATE ffmpeg-cpp Catch2::Catch2WithMain)

catch_discover_tests(ffmpeg-cpp-tests)
