cmake_minimum_required(VERSION 3.14)
set(CMAKE_C_STANDARD 11)

execute_process(
		COMMAND bash -c "egrep '^\\s*SDKRoot' $HOME/.Playdate/config"
		COMMAND head -n 1
		COMMAND cut -c9-
		OUTPUT_VARIABLE SDK
		OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Pure DOOM

# Game Name Customization
set(PLAYDATE_GAME_NAME PureDoom)
set(PLAYDATE_GAME_DEVICE PureDoom_DEVICE)

aux_source_directory(../src/DOOM/ DOOM_SOURCE_FILES)
aux_source_directory(src/ MAIN_SOURCE_FILES)

include_directories(../src/)

project(${PLAYDATE_GAME_NAME} C ASM)

if (TOOLCHAIN STREQUAL "armgcc")
	add_link_options(-specs=nano.specs -specs=nosys.specs)
	add_executable(${PLAYDATE_GAME_DEVICE} ${SDK}/C_API/buildsupport/setup.c ${DOOM_SOURCE_FILES} ${MAIN_SOURCE_FILES})
else()
	set(CMAKE_OSX_ARCHITECTURES arm64)
	add_library(${PLAYDATE_GAME_NAME} SHARED ${DOOM_SOURCE_FILES} ${MAIN_SOURCE_FILES} )
endif()

include(${SDK}/C_API/buildsupport/playdate_game.cmake)

