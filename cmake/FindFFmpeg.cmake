# FindFFmpeg.cmake - Locate FFmpeg development libraries.
#
# Defines:
#   FFmpeg_FOUND
#   FFmpeg_INCLUDE_DIRS
#   FFmpeg_LIBRARIES
#   FFmpeg::FFmpeg (imported target)

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(PC_AVCC QUIET libavcodec)
	pkg_check_modules(PC_AVFC QUIET libavformat)
	pkg_check_modules(PC_AVFL QUIET libavfilter)
	pkg_check_modules(PC_AVUT QUIET libavutil)
	pkg_check_modules(PC_SWSC QUIET libswscale)
	pkg_check_modules(PC_SWRS QUIET libswresample)
endif()

find_path(FFmpeg_INCLUDE_DIR
	NAMES libavcodec/avcodec.h
	HINTS
		${PC_AVCC_INCLUDEDIR}
		${PC_AVCC_INCLUDE_DIRS}
		$ENV{FFMPEG_DIR}/include
		$ENV{FFMPEG_DIR}/include/ffmpeg
		${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/include
	PATH_SUFFIXES ffmpeg
)

set(_ffmpeg_components avcodec avformat avfilter avutil swscale swresample)
set(FFmpeg_LIBRARIES "")
set(FFmpeg_FOUND TRUE)

foreach(_comp IN LISTS _ffmpeg_components)
	string(TOUPPER "${_comp}" _comp_upper)
	find_library(FFmpeg_${_comp_upper}_LIBRARY
		NAMES ${_comp}
		HINTS
			${PC_AVCC_LIBDIR}
			${PC_AVCC_LIBRARY_DIRS}
			$ENV{FFMPEG_DIR}/lib
			${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/lib
	)
	if(NOT FFmpeg_${_comp_upper}_LIBRARY)
		set(FFmpeg_FOUND FALSE)
	else()
		list(APPEND FFmpeg_LIBRARIES ${FFmpeg_${_comp_upper}_LIBRARY})
	endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg
	REQUIRED_VARS FFmpeg_INCLUDE_DIR FFmpeg_AVCODEC_LIBRARY FFmpeg_AVFORMAT_LIBRARY
)

if(FFmpeg_FOUND AND NOT TARGET FFmpeg::FFmpeg)
	add_library(FFmpeg::FFmpeg INTERFACE IMPORTED)
	set_target_properties(FFmpeg::FFmpeg PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${FFmpeg_INCLUDE_DIR}"
		INTERFACE_LINK_LIBRARIES "${FFmpeg_LIBRARIES}"
	)
endif()

mark_as_advanced(
	FFmpeg_INCLUDE_DIR
	FFmpeg_AVCODEC_LIBRARY
	FFmpeg_AVFORMAT_LIBRARY
	FFmpeg_AVFILTER_LIBRARY
	FFmpeg_AVUTIL_LIBRARY
	FFmpeg_SWSCALE_LIBRARY
	FFmpeg_SWRESAMPLE_LIBRARY
)
