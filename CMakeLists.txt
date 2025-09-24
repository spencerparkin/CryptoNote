# CMakeLists.txt file for CryptoNote project

cmake_minimum_required(VERSION 3.27)

project(CryptoNote)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ThirdParty)

set(CRYPTO_NOTE_SOURCES
	Source/App.cpp
	Source/App.h
	Source/Frame.cpp
	Source/Frame.h
	Source/NotePanel.cpp
	Source/NotePanel.h
)

source_group("Source" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CRYPTO_NOTE_SOURCES})

add_executable(CryptoNote WIN32
	${CRYPTO_NOTE_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/wx_dpi_aware_pmv2.manifest
)

target_compile_definitions(CryptoNote PUBLIC
	__WXMSW__
	_UNICODE
)

target_compile_definitions(CryptoNote PRIVATE
	WIN32_LEAN_AND_MEAN
	NOMINMAX
	_USE_MATH_DEFINES
)

target_link_libraries(CryptoNote
	wx::core
	wx::base
	wx::aui
	libssl.lib
)

target_link_directories(CryptoNote PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openssl/x64/lib
)

target_include_directories(CryptoNote PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openssl/x64/include
	${CMAKE_CURRENT_SOURCE_DIR}/Source
)