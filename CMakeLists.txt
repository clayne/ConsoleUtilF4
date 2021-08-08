cmake_minimum_required(VERSION 3.21)

# ---- Options ----

option(COPY_BUILD "Copy the build output to the Fallout 4 directory." OFF)

# ---- Cache build vars ----

macro(set_from_environment VARIABLE)
	if(NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif()
endmacro()

set_from_environment(Fallout4Path)

# ---- Project ----

project(
	ConsoleUtilF4
	VERSION 1.0.1
	LANGUAGES CXX
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

# ---- Include guards ----

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are not allowed.")
endif()

# ---- Dependencies ----

add_subdirectory("extern/CommonLibF4/CommonLibF4" CommonLibF4)

find_package(fmt REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)

# ---- Add source files ----

include(cmake/sourcelist.cmake)

source_group(
	TREE "${CMAKE_CURRENT_SOURCE_DIR}"
	FILES
		${SOURCES}
)

source_group(
	TREE "${CMAKE_CURRENT_BINARY_DIR}"
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/include/Version.h"
)

# ---- Create DLL ----

add_library(
	"${PROJECT_NAME}"
	SHARED
	${SOURCES}
	"${CMAKE_CURRENT_BINARY_DIR}/include/Version.h"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	".clang-format"
	".editorconfig"
)

target_compile_features(
	"${PROJECT_NAME}"
	PRIVATE
		cxx_std_20
)

target_include_directories(
	"${PROJECT_NAME}"
	PRIVATE
		"${CMAKE_CURRENT_BINARY_DIR}/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/c++"
)

target_link_libraries(
	"${PROJECT_NAME}"
	PRIVATE
		CommonLibF4::CommonLibF4
		fmt::fmt
		spdlog::spdlog
)

if(MSVC)
	target_compile_options(
		"${PROJECT_NAME}"
		PRIVATE
			"/sdl"	# Enable Additional Security Checks
			"/utf-8"	# Set Source and Executable character sets to UTF-8
			"/Zi"	# Debug Information Format

			"/permissive-"	# Standards conformance

			"$<$<CONFIG:DEBUG>:>"
			"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		"${PROJECT_NAME}"
		PRIVATE
			"/WX"	# Treat Linker Warnings as Errors

			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif()

target_precompile_headers(
	"${PROJECT_NAME}"
	PRIVATE
		"src/c++/PCH.h"
)

# ---- File copying ----

if(COPY_BUILD)
	if(DEFINED Fallout4Path)
		add_custom_command(
			TARGET "${PROJECT_NAME}"
			POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${PROJECT_NAME}>" "${Fallout4Path}/Data/F4SE/Plugins/"
			COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${Fallout4Path}/Data/F4SE/Plugins/"
		)
	else()
		message(WARNING "Fallout4Path is not defined. Skipping post-build copy command.")
	endif()
endif()

# ---- Build artifacts ----

find_package(Python3 QUIET MODULE)

if(DEFINED Fallout4Path AND Python3_FOUND)
	set(SCRIPT "scripts/papyrus_compile.py")

	add_custom_target(
		"papyrus_compile"
		COMMAND
			"$<TARGET_FILE:Python3::Interpreter>"
			"${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}"
			"--copy-build=$<IF:$<BOOL:${COPY_BUILD}>,true,false>"
			"--debug=$<IF:$<CONFIG:DEBUG>,true,false>"
			"--f4dir=${Fallout4Path}"
			"--src-dir=${CMAKE_CURRENT_SOURCE_DIR}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		VERBATIM
		SOURCES
			"${SCRIPT}"
	)

	source_group(
		TREE "${CMAKE_CURRENT_SOURCE_DIR}"
		FILES
			"${SCRIPT}"
	)

	set(SCRIPT "scripts/archive_artifacts.py")

	add_custom_target(
		"archive_artifacts"
		COMMAND
			"$<TARGET_FILE:Python3::Interpreter>"
			"${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}"
			"--dll=$<TARGET_FILE:${PROJECT_NAME}>"
			"--name=${PROJECT_NAME}"
			"--pdb=$<TARGET_PDB_FILE:${PROJECT_NAME}>"
			"--src-dir=${CMAKE_CURRENT_SOURCE_DIR}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		VERBATIM
		SOURCES
			"${SCRIPT}"
	)

	add_dependencies(
		"archive_artifacts"
		"papyrus_compile"
		"${PROJECT_NAME}"
	)

	source_group(
		TREE "${CMAKE_CURRENT_SOURCE_DIR}"
		FILES
			"${SCRIPT}"
	)
else()
	if(NOT DEFINED Fallout4Path)
		set(REASON "Fallout4Path is not defined")
	elseif(NOT Python3_FOUND)
		set(REASON "Python 3 was not found")
	else()
		set(REASON "unknown")
	endif()

	message(WARNING "${REASON}. Skipping artifact generation.")
endif()
