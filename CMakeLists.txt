cmake_minimum_required(VERSION 3.10)
project(lite3 C)

set(CMAKE_C_STANDARD 11)

# Find node-gyp headers (version-agnostic)
file(GLOB NODE_GYP_DIRS "$ENV{HOME}/Library/Caches/node-gyp/20.*")
list(GET NODE_GYP_DIRS 0 NODE_GYP_CACHE)

if(NODE_GYP_CACHE)
    message(STATUS "Found node-gyp cache: ${NODE_GYP_CACHE}")
    include_directories(${NODE_GYP_CACHE}/include/node)
else()
    message(WARNING "node-gyp cache not found - run install_deps target first")
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/deps/lite3/include
)

add_definitions(
    -DNAPI_VERSION=9
    -DBUILDING_NODE_EXTENSION
)

# Dummy library target for IDE awareness (not actually built)
add_library(lite3_ide SHARED EXCLUDE_FROM_ALL
    src/addon.c
    src/addon_encode.c
    src/addon_decode.c
)

# Read Node version from .nvmrc
file(STRINGS "${CMAKE_SOURCE_DIR}/.nvmrc" NODE_VERSION)

# Actual build target using nvm + node-gyp
add_custom_target(build_addon
    COMMAND ${CMAKE_COMMAND} -E env NVM_VERSION=${NODE_VERSION}
            $ENV{HOME}/.nvm/nvm-exec pnpm build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building native addon via node-gyp"
)

add_custom_target(clean_addon
    COMMAND ${CMAKE_COMMAND} -E env NVM_VERSION=${NODE_VERSION}
            $ENV{HOME}/.nvm/nvm-exec pnpm clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf node_modules
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Cleaning build artifacts"
)

add_custom_target(install_deps
    COMMAND ${CMAKE_COMMAND} -E env NVM_VERSION=${NODE_VERSION}
            $ENV{HOME}/.nvm/nvm-exec pnpm install
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Installing dependencies"
)

# Debug build target
add_custom_target(build_addon_debug
    COMMAND ${CMAKE_COMMAND} -E env NVM_VERSION=${NODE_VERSION}
            $ENV{HOME}/.nvm/nvm-exec node-gyp rebuild --debug
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building native addon with debug symbols"
)

# Debug test script path (create this file for your test case)
set(DEBUG_TEST_SCRIPT "${CMAKE_SOURCE_DIR}/test/debug.js")

# Run node under lldb
add_custom_target(debug_addon
    COMMAND lldb -o run -- $ENV{HOME}/.nvm/nvm-exec node ${DEBUG_TEST_SCRIPT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS build_addon_debug
    COMMENT "Running node under lldb debugger"
)

# Quick debug - runs lldb with inline test (stops at lldb prompt)
add_custom_target(debug_quick
    COMMAND lldb -- $ENV{HOME}/.nvm/nvm-exec node -e
            "const m = require('./build/Debug/lite3.node')$<SEMICOLON> console.log(m.decode(m.encode({foo: 'bar'})))"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Debug with lldb (type 'run' to start)"
)