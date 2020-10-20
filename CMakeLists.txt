cmake_minimum_required (VERSION 3.8)

project("Helicopter Commander")

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_NAME HelicopterCommander)

add_executable (HelicopterCommander
	"Source/Main.cpp"
	"Source/AudioRecording.cpp")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	if (MSVC_VERSION GREATER_EQUAL "1900")
		include(CheckCXXCompilerFlag)
		CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
		if (_cpp_latest_flag_supported)
			target_compile_options(${EXECUTABLE_NAME} PRIVATE "/std:c++latest")
		endif()
	endif()
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
	# Windows 10 x64 library
	# If you're using another OS, you have to build rnnoise and portaudio
	# library manually, then change the 2 lines below
	"${CMAKE_SOURCE_DIR}/Lib/rnnoise.lib"
	"${CMAKE_SOURCE_DIR}/Lib/portaudio_static_x64.lib")

target_include_directories(${EXECUTABLE_NAME} PRIVATE
	"${CMAKE_SOURCE_DIR}/Lib/rnnoise/include"
	"${CMAKE_SOURCE_DIR}/Lib/portaudio/include"
	"${CMAKE_SOURCE_DIR}/Lib/eigen"
	"${CMAKE_SOURCE_DIR}/Lib/frugally-deep/include"
	"${CMAKE_SOURCE_DIR}/Lib/json/single_include"
	"${CMAKE_SOURCE_DIR}/Lib/FunctionalPlus/include")
