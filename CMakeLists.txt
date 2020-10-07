cmake_minimum_required (VERSION 3.8)

project("Helicopter Commander")

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable (HelicopterCommander
	"src/Main.cpp"
	"src/AudioRecording.cpp")
target_link_libraries(HelicopterCommander ${CONAN_LIBS})

