

cmake_minimum_required(VERSION 3.1...3.28)

set(target_name "mdstat2collectd")

project(${target_name}
  VERSION 1.0
  LANGUAGES C)

add_executable(${target_name}
	src/main.c src/mdstat_reader.c src/utils.c src/output_collectd.c
	)

target_include_directories(${target_name}
	PRIVATE src/
	)

target_compile_definitions(${target_name}
	PUBLIC -DDEFAULT_INTERVAL=60
	)
	
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set ( mdstat_definition "../mdstat.txt" )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  set ( mdstat_definition "/proc/mdstat" )
else()
  set ( mdstat_definition "/proc/mdstat" )
endif()

target_compile_definitions(${target_name}
	PUBLIC -DMDSTAT_FILE="${mdstat_definition}"
	)

target_compile_options(${target_name}
	PRIVATE -Wall PRIVATE -Wextra PRIVATE -Wpedantic
	)

install(TARGETS ${target_name}
	DESTINATION bin
	)


######
# Packaging (debian)
#####

set ( CPACK_PACKAGING_INSTALL_PREFIX "/" )

# Let's compute Package Version and Package Release from GIT
find_package ( Git )

if ( Git_FOUND )
  execute_process(
	COMMAND ${GIT_EXECUTABLE} describe --tags --match "v*"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
	RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)

  if ( NOT GIT_DESCRIBE_ERROR_CODE )
  # GIT tag exists !

    string ( FIND ${GIT_DESCRIBE_VERSION} "-g" GIT_DESCRIBE_ADDITIONAL_COMMIT_POSITION )
    if ( NOT GIT_DESCRIBE_ADDITIONAL_COMMIT_POSITION EQUAL -1 )
  # GIT commits exists over the tag, pattern should be like this : <tag_name>-<commit_counter>-g<last_hash>

  # remove "-g<last_hash>"
      string ( SUBSTRING ${GIT_DESCRIBE_VERSION} 0 ${GIT_DESCRIBE_ADDITIONAL_COMMIT_POSITION} SHORT_GIT_DESCRIBE)

  # extract "-<commit_counter>"
      string ( FIND ${SHORT_GIT_DESCRIBE} "-" GIT_DESCRIBE_DASH_POSITION )
      string ( SUBSTRING ${SHORT_GIT_DESCRIBE} ${GIT_DESCRIBE_DASH_POSITION} -1 TEMP_ITERATION_COUNTER )

  # make the ITERATION_COUNTER positive and add 1, and it will become the debian package release counter
      math ( EXPR CPACK_DEBIAN_PACKAGE_RELEASE "${TEMP_ITERATION_COUNTER} * -1 + 1" )

      math ( EXPR GIT_END_OF_TAG_NAME "${GIT_DESCRIBE_DASH_POSITION} - 1" )
      string ( SUBSTRING ${GIT_DESCRIBE_VERSION} 1 ${GIT_END_OF_TAG_NAME} CPACK_DEBIAN_PACKAGE_VERSION )

    else ()
      string ( SUBSTRING ${GIT_DESCRIBE_VERSION} 1 -1 CPACK_DEBIAN_PACKAGE_VERSION )
    endif ()     # if ( NOT GIT_DESCRIBE_ADDITIONAL_COMMIT_POSITION EQUAL -1 )

    set ( COMPLETE_FULL_PACKAGE_VERSION "${SHORT_GIT_TAG_NUMBER}-${CPACK_DEBIAN_PACKAGE_RELEASE}" )
  endif ()       # if ( NOT GIT_DESCRIBE_ERROR_CODE )

else ()         # if ( Git_FOUND )
  set ( CPACK_DEBIAN_PACKAGE_VERSION "0.0.0" )
  set ( CPACK_DEBIAN_PACKAGE_RELEASE "1" )
endif ()         # if ( Git_FOUND )


set ( CPACK_GENERATOR "DEB" )

set ( CPACK_PACKAGE_NAME "${target_name}" )
set ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Linux Software Raid collector plugin for collectd " )
set ( CPACK_PACKAGE_DESCRIPTION "Read /proc/mdstat and push statuses to collectd via PluginExec" )

set ( CPACK_DEBIAN_FILE_NAME DEB-DEFAULT )
set ( CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64" )
set ( CPACK_DEBIAN_PACKAGE_DEPENDS "collectd" )
set ( CPACK_DEBIAN_PACKAGE_MAINTAINER "bug-mdstat2collectd@gilbow.org" )
set ( CPACK_DEBIAN_PACKAGE_SECTION "devel" )

# epoch of last commit
if ( Git_FOUND )
  execute_process(
	COMMAND ${GIT_EXECUTABLE} show --format=%at --no-patch
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_COMMIT_EPOCH
	RESULT_VARIABLE GIT_SHOW_ERROR_CODE
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)

  if ( NOT GIT_SHOW_ERROR_CODE )
    set ( SOURCE_DATE_EPOCH ${GIT_COMMIT_EPOCH} )
  endif ()       # if ( NOT GIT_SHOW_ERROR_CODE )
  
endif ()         # if ( Git_FOUND )


install ( FILES ${CMAKE_SOURCE_DIR}/mdstat2collectd.conf.example DESTINATION "etc/collectd/collectd.conf.d/" RENAME "mdstat2collectd.conf" )

# 

# Need to be AFTER variables definition !
include ( CPack )

