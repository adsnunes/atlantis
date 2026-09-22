cmake_minimum_required(VERSION 3.16)
project(atlantis C)

if(MSVC)
    set(CMAKE_C_STANDARD 17)
else()
    set(CMAKE_C_STANDARD 23)
endif()
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

option(USE_LIBAI "Build with Apple Intelligence (macOS 26+)" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
set(ATLANTIS_VERSION "dev" CACHE STRING "Version string")

# Sanitizer flags
set(SANITIZER_FLAGS "")
if(ENABLE_ASAN)
    list(APPEND SANITIZER_FLAGS -fsanitize=address -fno-omit-frame-pointer)
endif()
if(ENABLE_UBSAN)
    list(APPEND SANITIZER_FLAGS -fsanitize=undefined)
endif()

file(GLOB ATLANTIS_SRCS src/atlantis*.c)
list(FILTER ATLANTIS_SRCS EXCLUDE REGEX "atlantis_backend_.*\\.c$")
list(FILTER ATLANTIS_SRCS EXCLUDE REGEX "atlantis_compat\\.c$")
file(GLOB HIGHLIGHT_SRCS src/highlight/*.c src/highlight/lang/*.c)

set(PCRE2_BUILD_PCRE2GREP OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
if(EMSCRIPTEN)
    set(PCRE2_SUPPORT_JIT OFF CACHE BOOL "" FORCE)
else()
    set(PCRE2_SUPPORT_JIT ON CACHE BOOL "" FORCE)
endif()
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(third-party/pcre2)

add_subdirectory(third-party/utf8proc)

# cyaml for YAML frontmatter parsing
set(CYAML_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CYAML_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(third-party/cyaml)

if(NOT EMSCRIPTEN)
    # Select platform-specific backend
    if(WIN32)
        set(ATLANTIS_BACKEND_SRC src/atlantis_backend_win32.c)
    else()
        find_package(CURL REQUIRED)
        set(ATLANTIS_BACKEND_SRC src/atlantis_backend_posix.c)
    endif()

    if(WIN32)
        set(ATLANTIS_COMPAT_SRC src/atlantis_compat.c)
    endif()

    add_executable(atlantis
        ${ATLANTIS_SRCS}
        ${HIGHLIGHT_SRCS}
        ${ATLANTIS_BACKEND_SRC}
        ${ATLANTIS_COMPAT_SRC}
        src/main_term.c
        third-party/cJSON.c
    )

    target_include_directories(atlantis PRIVATE
        src
        third-party
        third-party/nanosvg/src
        ${CMAKE_BINARY_DIR}/third-party/pcre2
        third-party/pcre2/src
        third-party/cyaml/src
    )

    if(MSVC)
        target_compile_options(atlantis PRIVATE /W4 /WX /wd4244 /wd4267 /wd4245 /wd4456 /wd4566 /wd4702 /utf-8)
        target_compile_definitions(atlantis PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(atlantis PRIVATE
            -Wall -Wextra -Wpedantic -Werror -Wunused
            $<$<C_COMPILER_ID:AppleClang,Clang>:-Wno-keyword-macro -Wno-newline-eof -Wno-deprecated-declarations>
            ${SANITIZER_FLAGS}
        )
        if(SANITIZER_FLAGS)
            target_link_options(atlantis PRIVATE ${SANITIZER_FLAGS})
        endif()
    endif()
    target_compile_definitions(atlantis PRIVATE VERSION="${ATLANTIS_VERSION}")

    target_link_libraries(atlantis PRIVATE
        pcre2-8-static
        utf8proc
        cyaml_static
    )

    if(WIN32)
        target_link_libraries(atlantis PRIVATE winhttp)
    else()
        target_link_libraries(atlantis PRIVATE CURL::libcurl m)
    endif()

    if(APPLE)
        target_link_libraries(atlantis PRIVATE "-framework ApplicationServices")
    endif()

    if(APPLE AND USE_LIBAI)
        target_sources(atlantis PRIVATE src/search.c)
        target_compile_definitions(atlantis PRIVATE USE_LIBAI)
        target_include_directories(atlantis PRIVATE libai)
        target_compile_options(atlantis PRIVATE -Wno-unused-variable -Wno-unused-but-set-variable)

        add_custom_target(libai_build
            COMMAND make static-rel
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libai
        )
        add_dependencies(atlantis libai_build)

        target_link_libraries(atlantis PRIVATE
            ${CMAKE_SOURCE_DIR}/libai/build/static/${CMAKE_SYSTEM_PROCESSOR}/release/libai.a
            "-framework Foundation"
            "-framework FoundationModels"
            -L/usr/lib/swift -lswiftCore
        )
    endif()

    install(TARGETS atlantis RUNTIME DESTINATION bin)

    # Test target for atlantis_block
    add_executable(test-block
        tests/test_block.c
        src/atlantis_block.c
        src/atlantis_gap.c
        src/atlantis_md.c
        src/atlantis_wrap.c
        third-party/cJSON.c
    )

    target_include_directories(test-block PRIVATE
        src
        third-party
        ${CMAKE_BINARY_DIR}/third-party/pcre2
        third-party/pcre2/src
    )

    if(MSVC)
        target_compile_options(test-block PRIVATE /W4 /WX /utf-8)
        target_compile_definitions(test-block PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(test-block PRIVATE
            -Wall -Wextra -Wpedantic -Werror -Wunused
            $<$<C_COMPILER_ID:AppleClang,Clang>:-Wno-keyword-macro -Wno-newline-eof -Wno-deprecated-declarations>
            ${SANITIZER_FLAGS}
        )
        if(SANITIZER_FLAGS)
            target_link_options(test-block PRIVATE ${SANITIZER_FLAGS})
        endif()
    endif()

    target_link_libraries(test-block PRIVATE pcre2-8-static utf8proc)
    if(NOT WIN32)
        target_link_libraries(test-block PRIVATE m)
    endif()
else()
    file(GLOB WEB_SRCS src/atlantis*.c)
    list(FILTER WEB_SRCS EXCLUDE REGEX "atlantis_args.c")
    list(FILTER WEB_SRCS EXCLUDE REGEX "atlantis_backend_posix.c")
    list(FILTER WEB_SRCS EXCLUDE REGEX "atlantis_backend_win32.c")

    add_executable(atlantis-web
        ${WEB_SRCS}
        ${HIGHLIGHT_SRCS}
        src/atlantis_backend_web.c
        src/main_web.c
        third-party/cJSON.c
    )

    target_include_directories(atlantis-web PRIVATE
        src
        third-party
        ${CMAKE_BINARY_DIR}/third-party/pcre2
        third-party/pcre2/src
        third-party/cyaml/src
        third-party/nanosvg/src
    )

    target_compile_options(atlantis-web PRIVATE -O2)
    target_link_libraries(atlantis-web PRIVATE pcre2-8-static utf8proc cyaml_static)

    set_target_properties(atlantis-web PROPERTIES
        OUTPUT_NAME "atlantis"
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/web"
    )

    target_link_options(atlantis-web PRIVATE
        -sWASM=1
        -sALLOW_MEMORY_GROWTH=1
        -sASYNCIFY
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString,stringToUTF8,lengthBytesUTF8,setValue,getValue
        -sEXPORTED_FUNCTIONS=_main,_malloc,_free,_web_on_resize,_web_on_key,_web_on_mouse,_atlantis_web_load_file,_atlantis_web_new_document,_atlantis_web_save
        -sFORCE_FILESYSTEM=1
        -lidbfs.js
        --shell-file ${CMAKE_SOURCE_DIR}/src/shell.html
    )
endif()
