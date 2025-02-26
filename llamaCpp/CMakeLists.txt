cmake_minimum_required(VERSION 3.12)
set(TARGET_MAIN LlamaCpp)
project(${TARGET_MAIN})

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED true)
add_library(${TARGET_MAIN} SHARED Main.cpp Main.h)
target_include_directories(${TARGET_MAIN} PRIVATE "lib/include" "lib/ggml/include")
#target_compile_features(${TARGET_MAIN} PRIVATE cxx_std_17)

set(TARGET ${TARGET_MAIN}_SMCM)
add_custom_target(${TARGET})
if (${BUILD_TYPE_EXT} STREQUAL "WIN64")
    #    if (MSVC)
    #        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    #        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
    #        add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
    #    endif ()
    target_link_libraries(${TARGET_MAIN} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/llama.lib
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/ggml-base.lib
    )
    set(LIB_FOLDER "win64")
#    set(LIB_FOLDER "win64_vulkan")
    add_custom_command(
            TARGET ${TARGET}
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/properties.xml" "${CMAKE_CURRENT_BINARY_DIR}/properties.xml"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/llama.dll" "${CMAKE_CURRENT_BINARY_DIR}/llama.dll"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/ggml-base.dll" "${CMAKE_CURRENT_BINARY_DIR}/ggml-base.dll"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/ggml.dll" "${CMAKE_CURRENT_BINARY_DIR}/ggml.dll"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/ggml-cpu.dll" "${CMAKE_CURRENT_BINARY_DIR}/ggml-cpu.dll"
#            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/ggml-vulkan.dll" "${CMAKE_CURRENT_BINARY_DIR}/ggml-vulkan.dll"
            #            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/ggml-rpc.dll" "${CMAKE_CURRENT_BINARY_DIR}/ggml-rpc.dll"
            COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${TARGET_MAIN}.smcm" --format=zip --
            "${CMAKE_CURRENT_BINARY_DIR}/properties.xml"
            "${CMAKE_CURRENT_BINARY_DIR}/llama.dll"
            "${CMAKE_CURRENT_BINARY_DIR}/ggml-base.dll"
            "${CMAKE_CURRENT_BINARY_DIR}/ggml.dll"
            "${CMAKE_CURRENT_BINARY_DIR}/ggml-cpu.dll"
#            "${CMAKE_CURRENT_BINARY_DIR}/ggml-vulkan.dll"
            #            "${CMAKE_CURRENT_BINARY_DIR}/ggml-rpc.dll"
            "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_MAIN}.dll"
    )
elseif (${BUILD_TYPE_EXT} STREQUAL "LINUX64")
    target_link_libraries(${TARGET_MAIN} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux64/libllama.so
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux64/libggml-base.so
    )
    set(LIB_FOLDER "linux64")
    add_custom_command(
            TARGET ${TARGET}
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/properties_linux.xml" "${CMAKE_CURRENT_BINARY_DIR}/properties.xml"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/libllama.so" "${CMAKE_CURRENT_BINARY_DIR}/libllama.so"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/libggml-base.so" "${CMAKE_CURRENT_BINARY_DIR}/libggml-base.so"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/libggml.so" "${CMAKE_CURRENT_BINARY_DIR}/libggml.so"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/libggml-cpu.so" "${CMAKE_CURRENT_BINARY_DIR}/libggml-cpu.so"
            #            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_FOLDER}/libggml-rpc.so" "${CMAKE_CURRENT_BINARY_DIR}/libggml-rpc.so"
            COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${TARGET_MAIN}.smcm" --format=zip --
            "${CMAKE_CURRENT_BINARY_DIR}/properties.xml"
            "${CMAKE_CURRENT_BINARY_DIR}/libllama.so"
            "${CMAKE_CURRENT_BINARY_DIR}/libggml-base.so"
            "${CMAKE_CURRENT_BINARY_DIR}/libggml.so"
            "${CMAKE_CURRENT_BINARY_DIR}/libggml-cpu.so"
            #            "${CMAKE_CURRENT_BINARY_DIR}/libggml-rpc.so"
            "${CMAKE_CURRENT_BINARY_DIR}/lib${TARGET_MAIN}.so"
    )
endif ()

