cmake_minimum_required(VERSION 3.12)
project(smcmodules)

set(CMAKE_CXX_STANDARD 17)
#set(BUILD_ARCH "-m64")

#set(CMAKE_SHARED_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

include_directories("../SMCModuleDefinitionProvider")

#set_property(GLOBAL PROPERTY PREFIX "")

if($ENV{_WIN64})
    message("arch:" "_WIN64")
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/../SMCModuleDefinitionProvider/cmake-build-release-visual-studio-64/ModuleLoaderProviderWIN64.lib)
elseif($ENV{_WIN32})
    message("arch:" "_WIN32")
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/../SMCModuleDefinitionProvider/cmake-build-release-visual-studio-32/ModuleLoaderProviderWIN32.lib)
endif()

add_subdirectory(example)

