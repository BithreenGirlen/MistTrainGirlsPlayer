cmake_minimum_required (VERSION 3.10)
project(MistTrainGirlsPlayer-deps)

set(SFML_MAJOR_VERSION "2")
set(SFML_MINOR_VERSION ".6.1")
set(SFML_VERSION ${SFML_MAJOR_VERSION}${SFML_MINOR_VERSION})

set(SPINE_VERSION "3.8")
set(SPINE_RUNTIME_LANGUAGE "spine-cpp")

set(SFML_DIR ${CMAKE_CURRENT_LIST_DIR}/SFML-${SFML_VERSION})
set(SPINE_RUNTIME_DIR ${CMAKE_CURRENT_LIST_DIR}/${SPINE_RUNTIME_LANGUAGE}-${SPINE_VERSION})

set(TEMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/temp)

function(download_and_extract HOST_URL ARCHIVE_NAME)
	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}.zip")
		file(DOWNLOAD "${HOST_URL}" "${TEMP_DIR}/${ARCHIVE_NAME}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}")
		file(ARCHIVE_EXTRACT INPUT "${TEMP_DIR}/${ARCHIVE_NAME}.zip" DESTINATION "${TEMP_DIR}")
	endif()
endfunction()

# Downlonad pre-built binaries of SFML for VC
if (NOT EXISTS "${SFML_DIR}")
	set(SFML_VC_BUILT "SFML-${SFML_VERSION}-windows-vc17-64-bit")
	download_and_extract("https://www.sfml-dev.org/files/${SFML_VC_BUILT}.zip" ${SFML_VC_BUILT})

	file(COPY "${TEMP_DIR}/SFML-${SFML_VERSION}/include" DESTINATION "${SFML_DIR}")
	file(COPY "${TEMP_DIR}/SFML-${SFML_VERSION}/lib" DESTINATION "${SFML_DIR}")

	set(DEBUG_DLLS "sfml-graphics-d-${SFML_MAJOR_VERSION}.dll" "sfml-system-d-${SFML_MAJOR_VERSION}.dll" "sfml-window-d-${SFML_MAJOR_VERSION}.dll" "sfml-audio-d-${SFML_MAJOR_VERSION}.dll" "openal32.dll")
	set(RELEASE_DLLS "sfml-graphics-${SFML_MAJOR_VERSION}.dll" "sfml-system-${SFML_MAJOR_VERSION}.dll" "sfml-window-${SFML_MAJOR_VERSION}.dll" "sfml-audio-${SFML_MAJOR_VERSION}.dll" "openal32.dll")
	# Debug dll
	foreach(FILENAME IN LISTS DEBUG_DLLS)
		file(COPY "${TEMP_DIR}/SFML-${SFML_VERSION}/bin/${FILENAME}" DESTINATION "${CMAKE_CURRENT_LIST_DIR}/../x64/Debug")
	endforeach()

	#Release dll
	foreach(FILENAME IN LISTS RELEASE_DLLS)
		file(COPY "${TEMP_DIR}/SFML-${SFML_VERSION}/bin/${FILENAME}" DESTINATION "${CMAKE_CURRENT_LIST_DIR}/../x64/Release")
	endforeach()
endif()

# Download spine generic runtime
if (NOT EXISTS "${SPINE_RUNTIME_DIR}")
	set (SPINE_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/heads/${SPINE_VERSION}.zip")
	set (SPINE_FOLDER_NAME "spine-runtimes-${SPINE_VERSION}")
	download_and_extract("${SPINE_RUNTIME_URL}" "${SPINE_FOLDER_NAME}")

	file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/src" DESTINATION "${SPINE_RUNTIME_DIR}")
	file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/include" DESTINATION "${SPINE_RUNTIME_DIR}")
endif()

# Delete Spine runtimes for other languages because they are irrelevant for the project.
file(REMOVE_RECURSE "${TEMP_DIR}")
