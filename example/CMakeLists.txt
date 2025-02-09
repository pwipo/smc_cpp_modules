#add_library(TestModule SHARED Test.cpp Test.h ../webcam/WebCam.cpp ../webcam/WebCam.h)

#set_target_properties(TestModule PROPERTIES PREFIX "")
#[[
install(TARGETS TestModule EXPORT TestModuleSMCM
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
        INCLUDES DESTINATION include)
]]
#install(TARGETS TestModule EXPORT TestModuleSMCM DESTINATION lib2)

#install(FILES text.zip DESTINATION lib/foo)

#install(EXPORT fooTargets FILE "FooTargets.cmake" DESTINATION lib/foo)

#include(CPack)

#add_custom_target(TestModule_SMCM COMMAND ${CMAKE_COMMAND} -E tar "cf" "text.zip" --format=zip -- "${CMAKE_CURRENT_BINARY_DIR}/properties.xml" "${CMAKE_CURRENT_BINARY_DIR}/libTestModule.dll")

#install(FILES "${CMAKE_CURRENT_BINARY_DIR}/text.zip" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/datadir" RENAME "cppTest.smcm")

add_custom_target(Example_SMCM)

add_library(Example SHARED Test.cpp Test.h)

if (_WIN64)
    message("arch:" "_WIN64")
    add_custom_command(
            TARGET Example_SMCM
            #        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets/video ${CMAKE_CURRENT_BINARY_DIR}/video
            #        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets/data ${CMAKE_CURRENT_BINARY_DIR}/data
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/properties.xml" "${CMAKE_CURRENT_BINARY_DIR}/properties.xml"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/text.txt" "${CMAKE_CURRENT_BINARY_DIR}/text.txt"
            #        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libTestModule.dll ${CMAKE_CURRENT_BINARY_DIR}/smcm
            #        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/assets/strings_rules_en.csv ${CMAKE_CURRENT_BINARY_DIR}/
            COMMAND ${CMAKE_COMMAND} -E tar "cfv" "ExampleCpp.smcm" --format=zip -- "${CMAKE_CURRENT_BINARY_DIR}/properties.xml" "${CMAKE_CURRENT_BINARY_DIR}/text.txt" "${CMAKE_CURRENT_BINARY_DIR}/Example.dll"
            #        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/smcm
            #        COMMAND ${CMAKE_COMMAND} -E rename  ${CMAKE_CURRENT_BINARY_DIR}/data.zip ${CMAKE_CURRENT_BINARY_DIR}/cppTest.smcm
    )
elseif (_WIN32)
    message("arch:" "_WIN32")
    add_custom_command(
            TARGET Example_SMCM
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/properties.xml" "${CMAKE_CURRENT_BINARY_DIR}/properties.xml"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/text.txt" "${CMAKE_CURRENT_BINARY_DIR}/text.txt"
            COMMAND ${CMAKE_COMMAND} -E tar "cfv" "ExampleCpp.smcm" --format=zip -- "${CMAKE_CURRENT_BINARY_DIR}/properties.xml" "${CMAKE_CURRENT_BINARY_DIR}/text.txt" "${CMAKE_CURRENT_BINARY_DIR}/Example.dll"
    )
elseif (_LINUX64)
    message("arch:" "_LINUX64")
    add_custom_command(
            TARGET Example_SMCM
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/properties_linux.xml" "${CMAKE_CURRENT_BINARY_DIR}/properties.xml"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/text.txt" "${CMAKE_CURRENT_BINARY_DIR}/text.txt"
            #            COMMAND ${CMAKE_COMMAND} -E make_directory "for_centos"
            COMMAND ${CMAKE_COMMAND} -E tar "cfv" "ExampleCpp.smcm" --format=zip -- "${CMAKE_CURRENT_BINARY_DIR}/properties.xml" "${CMAKE_CURRENT_BINARY_DIR}/text.txt" "${CMAKE_CURRENT_BINARY_DIR}/libExample.so"
    )
elseif (_LINUX32)
    message("arch:" "_LINUX32")
    add_custom_command(
            TARGET Example_SMCM
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/properties_linux.xml" "${CMAKE_CURRENT_BINARY_DIR}/properties.xml"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/text.txt" "${CMAKE_CURRENT_BINARY_DIR}/text.txt"
#            COMMAND ${CMAKE_COMMAND} -E make_directory "for_centos"
            COMMAND ${CMAKE_COMMAND} -E tar "cfv" "ExampleCpp.smcm" --format=zip -- "${CMAKE_CURRENT_BINARY_DIR}/properties.xml" "${CMAKE_CURRENT_BINARY_DIR}/text.txt" "${CMAKE_CURRENT_BINARY_DIR}/libExample.so"
    )
endif ()

