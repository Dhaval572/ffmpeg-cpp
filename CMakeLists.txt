cmake_minimum_required(VERSION 3.16)

project(ffmpeg-cpp
	VERSION 1.0.0
	DESCRIPTION "A clean C++ wrapper around the FFmpeg libraries"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FFMPEGCPP_WITH_NVENC "Build NVIDIA NVENC codec wrappers (requires NVIDIA GPU + FFmpeg with NVENC)" OFF)
option(FFMPEGCPP_BUILD_EXAMPLES "Build the example programs" ON)
option(FFMPEGCPP_BUILD_SIMPLE_INTERFACE "Build the simple_interface shared library" ON)
option(FFMPEGCPP_BUILD_TESTS "Build unit tests" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)

# Prefer pkg-config on Unix; fall back to FindFFmpeg module.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
	pkg_check_modules(FFMPEG QUIET IMPORTED_TARGET
		libavcodec
		libavformat
		libavfilter
		libavutil
		libswscale
		libswresample
	)
endif()

if(NOT FFMPEG_FOUND)
	find_package(FFmpeg REQUIRED)
	set(FFMPEG_LIBRARIES FFmpeg::FFmpeg)
	set(FFMPEG_INCLUDE_DIRS "${FFMPEG_INCLUDE_DIRS}")
endif()

set(FFMPEGCPP_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/ffmpeg-cpp")

set(FFMPEGCPP_SOURCES
	${FFMPEGCPP_LIBRARY_DIR}/AudioFormatConverter.cpp
	${FFMPEGCPP_LIBRARY_DIR}/CodecDeducer.cpp
	${FFMPEGCPP_LIBRARY_DIR}/FrameContainer.cpp
	${FFMPEGCPP_LIBRARY_DIR}/FFmpegException.cpp
	${FFMPEGCPP_LIBRARY_DIR}/OpenCodec.cpp
	${FFMPEGCPP_LIBRARY_DIR}/VideoFormatConverter.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Codecs/Codec.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Codecs/AudioCodec.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Codecs/VideoCodec.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Codecs/VP9Codec.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Codecs/PNGCodec.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Codecs/JPGCodec.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Demuxing/StreamData.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Demuxing/InputStream.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Demuxing/AudioInputStream.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Demuxing/VideoInputStream.cpp
	${FFMPEGCPP_LIBRARY_DIR}/FrameSinks/FrameSinkStream.cpp
	${FFMPEGCPP_LIBRARY_DIR}/FrameSinks/OneInputFrameSink.cpp
	${FFMPEGCPP_LIBRARY_DIR}/FrameSinks/VideoFilterInput.cpp
	${FFMPEGCPP_LIBRARY_DIR}/FrameSinks/Filter.cpp
	${FFMPEGCPP_LIBRARY_DIR}/FrameSinks/VideoEncoder.cpp
	${FFMPEGCPP_LIBRARY_DIR}/FrameSinks/AudioEncoder.cpp
	${FFMPEGCPP_LIBRARY_DIR}/FrameSinks/AudioFilter.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Muxing/Muxer.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Muxing/OutputStream.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Muxing/AudioOutputStream.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Muxing/VideoOutputStream.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Sources/Demuxer.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Sources/RawAudioDataSource.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Sources/RawVideoDataSource.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Sources/RawAudioFileSource.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Sources/RawVideoFileSource.cpp
	${FFMPEGCPP_LIBRARY_DIR}/Sources/EncodedFileSource.cpp
)

if(FFMPEGCPP_WITH_NVENC)
	list(APPEND FFMPEGCPP_SOURCES
		${FFMPEGCPP_LIBRARY_DIR}/Codecs/H264NVEncCodec.cpp
		${FFMPEGCPP_LIBRARY_DIR}/Codecs/H265NVEncCodec.cpp
	)
endif()

add_library(ffmpeg-cpp ${FFMPEGCPP_SOURCES})
add_library(ffmpeg-cpp::ffmpeg-cpp ALIAS ffmpeg-cpp)

target_include_directories(ffmpeg-cpp
	PUBLIC
		$<BUILD_INTERFACE:${FFMPEGCPP_LIBRARY_DIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp>
		$<INSTALL_INTERFACE:include>
)

if(TARGET PkgConfig::FFMPEG)
	target_link_libraries(ffmpeg-cpp PUBLIC PkgConfig::FFMPEG)
else()
	target_include_directories(ffmpeg-cpp PUBLIC ${FFMPEG_INCLUDE_DIRS})
	target_link_libraries(ffmpeg-cpp PUBLIC ${FFMPEG_LIBRARIES})
endif()

target_link_libraries(ffmpeg-cpp PUBLIC Threads::Threads)

if(FFMPEGCPP_WITH_NVENC)
	target_compile_definitions(ffmpeg-cpp PUBLIC FFMPEGCPP_WITH_NVENC)
endif()

if(MSVC)
	target_compile_options(ffmpeg-cpp PRIVATE /W3)
else()
	target_compile_options(ffmpeg-cpp PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

set_target_properties(ffmpeg-cpp PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	POSITION_INDEPENDENT_CODE ON
)

# ---------------------------------------------------------------------------
# simple_interface shared library
# ---------------------------------------------------------------------------
if(FFMPEGCPP_BUILD_SIMPLE_INTERFACE)
	add_library(simple_interface SHARED
		${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/simple_interface/SimpleInterface.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/simple_interface/dllmain.cpp
	)

	target_include_directories(simple_interface
		PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/simple_interface>
			$<INSTALL_INTERFACE:include>
	)

	target_link_libraries(simple_interface PRIVATE ffmpeg-cpp)

	set_target_properties(simple_interface PROPERTIES
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR}
		POSITION_INDEPENDENT_CODE ON
	)

	if(WIN32)
		set_target_properties(simple_interface PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
	endif()
endif()

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
if(FFMPEGCPP_BUILD_EXAMPLES)
	set(FFMPEGCPP_SAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/samples")

	function(ffmpegcpp_add_example name source_dir)
		add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/${source_dir}/${name}.cpp)
		target_link_libraries(${name} PRIVATE ffmpeg-cpp)
		if(EXISTS "${FFMPEGCPP_SAMPLES_DIR}")
			add_custom_command(TARGET ${name} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory
					"${FFMPEGCPP_SAMPLES_DIR}"
					"$<TARGET_FILE_DIR:${name}>/samples"
				COMMENT "Copying samples for ${name}"
			)
		endif()
	endfunction()

	if(FFMPEGCPP_WITH_NVENC)
		# demo can use NVENC when enabled
	endif()

	ffmpegcpp_add_example(print_info print_info)
	ffmpegcpp_add_example(decode_audio decode_audio)
	ffmpegcpp_add_example(decode_video decode_video)
	ffmpegcpp_add_example(encode_audio encode_audio)
	ffmpegcpp_add_example(encode_video encode_video)
	ffmpegcpp_add_example(filtering_audio filtering_audio)
	ffmpegcpp_add_example(filtering_video filtering_video)
	ffmpegcpp_add_example(remuxing remuxing)
	ffmpegcpp_add_example(difference difference)

	add_executable(demo
		${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/demo/demo.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/demo/GeneratedVideoSource.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/demo/GeneratedAudioSource.cpp
	)
	target_link_libraries(demo PRIVATE ffmpeg-cpp)
	if(EXISTS "${FFMPEGCPP_SAMPLES_DIR}")
		add_custom_command(TARGET demo POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${FFMPEGCPP_SAMPLES_DIR}"
				"$<TARGET_FILE_DIR:demo>/samples"
			COMMENT "Copying samples for demo"
		)
	endif()

	if(FFMPEGCPP_BUILD_SIMPLE_INTERFACE)
		add_executable(simple_interface_demo
			${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/simple_interface_demo/example.cpp
		)
		target_include_directories(simple_interface_demo PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/simple_interface
		)
		target_link_libraries(simple_interface_demo PRIVATE simple_interface)
		if(EXISTS "${FFMPEGCPP_SAMPLES_DIR}")
			add_custom_command(TARGET simple_interface_demo POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory
					"${FFMPEGCPP_SAMPLES_DIR}"
					"$<TARGET_FILE_DIR:simple_interface_demo>/samples"
				COMMENT "Copying samples for simple_interface_demo"
			)
		endif()
	endif()
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS ffmpeg-cpp
	EXPORT ffmpeg-cpp-targets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${FFMPEGCPP_LIBRARY_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ffmpeg-cpp
	FILES_MATCHING PATTERN "*.h"
	PATTERN "Codecs" EXCLUDE
	PATTERN "Demuxing" EXCLUDE
	PATTERN "FrameSinks" EXCLUDE
	PATTERN "Info" EXCLUDE
	PATTERN "Muxing" EXCLUDE
	PATTERN "Sources" EXCLUDE
)

install(DIRECTORY
	${FFMPEGCPP_LIBRARY_DIR}/Codecs/
	${FFMPEGCPP_LIBRARY_DIR}/Demuxing/
	${FFMPEGCPP_LIBRARY_DIR}/FrameSinks/
	${FFMPEGCPP_LIBRARY_DIR}/Info/
	${FFMPEGCPP_LIBRARY_DIR}/Muxing/
	${FFMPEGCPP_LIBRARY_DIR}/Sources/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ffmpeg-cpp
	FILES_MATCHING PATTERN "*.h"
)

if(FFMPEGCPP_BUILD_SIMPLE_INTERFACE)
	install(TARGETS simple_interface
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
	install(FILES
		${CMAKE_CURRENT_SOURCE_DIR}/source/ffmpeg-cpp/simple_interface/SimpleInterface.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ffmpeg-cpp/simple_interface
	)
endif()

install(EXPORT ffmpeg-cpp-targets
	FILE ffmpeg-cpp-targets.cmake
	NAMESPACE ffmpeg-cpp::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ffmpeg-cpp
)

if(FFMPEGCPP_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
