cmake_minimum_required(VERSION 3.15)
project(sr2)
add_subdirectory(r2)


set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)

file(GLOB_RECURSE sr2_game_src
    "src/*.h"
    "src/*.cpp"
    "src/*.hpp"
)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_executable(sr2_game ${sr2_game_src})

MACRO(SOURCE_GROUP_BY_FOLDER target)
  SET(SOURCE_GROUP_DELIMITER "/")
  SET(last_dir "")
  SET(files "")
  FOREACH(file ${${target}_src})
    file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}" ${file})
    GET_FILENAME_COMPONENT(dir "${relative_file}" PATH)
    IF (NOT "${dir}" STREQUAL "${last_dir}")
      IF (files)
        SOURCE_GROUP("${last_dir}" FILES ${files})
      ENDIF (files)
      SET(files "")
    ENDIF (NOT "${dir}" STREQUAL "${last_dir}")
    SET(files ${files} ${file})
    SET(last_dir "${dir}")
  ENDFOREACH(file)
  IF (files)
    SOURCE_GROUP("${last_dir}" FILES ${files})
  ENDIF (files)
ENDMACRO(SOURCE_GROUP_BY_FOLDER)
SOURCE_GROUP_BY_FOLDER(sr2_game)

target_include_directories(sr2_game PUBLIC src)
target_include_directories(sr2_game PUBLIC include)
target_include_directories(sr2_game PUBLIC r2/engine)
target_include_directories(sr2_game PUBLIC r2/engine/include)
target_include_directories(sr2_game PUBLIC r2/engine/include/OIS)
target_include_directories(sr2_game PUBLIC "C:\\Program Files (x86)\\OpenAL 1.1 SDK\\include")

target_link_directories(sr2_game PUBLIC lib)
target_link_directories(sr2_game PUBLIC build/r2/engine)
target_link_directories(sr2_game PUBLIC r2/engine/lib)
target_link_directories(sr2_game PUBLIC "C:\\Program Files (x86)\\OpenAL 1.1 SDK\\libs\\Win64")

SET(debug_libs
	r2
	debug/v8pp
	debug/v8
	debug/v8_libbase
	debug/v8_libplatform
	debug/glfw3
	debug/OIS_d
	debug/Bullet3Collision_Debug
	debug/Bullet3Common_Debug
	debug/Bullet3Dynamics_Debug
	debug/Bullet3Geometry_Debug
	debug/BulletCollision_Debug
	debug/BulletDynamics_Debug
	debug/LinearMath_Debug
	debug/zlibstaticd
	OpenAL32
	opengl32
)

SET(release_libs
	r2
	release/v8pp
	release/v8
	release/v8_libbase
	release/v8_libplatform
	release/glfw3
	release/OIS
	release/Bullet3Collision
	release/Bullet3Common
	release/Bullet3Dynamics
	release/Bullet3Geometry
	release/BulletCollision
	release/BulletDynamics
	release/LinearMath
	release/zlibstatic
	OpenAL32
	opengl32
)

target_link_libraries(sr2_game ${debug_libs})
#target_link_libraries(sr2_game ${release_libs})


set_property(TARGET sr2_game PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")

add_definitions(-D_CRT_NO_VA_START_VALIDATION)