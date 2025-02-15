cmake_minimum_required(VERSION 3.12)
project(smcmodules_pub)
#SET(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 14)
#set(BUILD_ARCH "-m64")
#set(CMAKE_SHARED_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
#set_property(GLOBAL PROPERTY PREFIX "")

include_directories("../SMCModuleDefinitionProvider") # contain SMC c++ provider
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
message("type: ${CMAKE_BUILD_TYPE}")
message("BUILD_TYPE_EXT=${BUILD_TYPE_EXT}")

if (${BUILD_TYPE_EXT} STREQUAL "WIN64")
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/../SMCModuleDefinitionProvider/Win64/ModuleLoaderProviderWIN64.lib)
elseif (${BUILD_TYPE_EXT} STREQUAL "WIN32")
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/../SMCModuleDefinitionProvider/Win32/ModuleLoaderProviderWIN32.lib)
elseif (${BUILD_TYPE_EXT} STREQUAL "LINUX32")
    set(CMAKE_CXX_FLAGS "-m32")
    #    set(CMAKE_CXX_FLAGS "-m32 -fpic")
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/../SMCModuleDefinitionProvider/Linux32/libModuleLoaderProviderLINUX32.so)
elseif (${BUILD_TYPE_EXT} STREQUAL "LINUX64")
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/../SMCModuleDefinitionProvider/Linux64/libModuleLoaderProviderLINUX64.so)
endif ()

add_subdirectory(example)
add_subdirectory(llamaCpp)

