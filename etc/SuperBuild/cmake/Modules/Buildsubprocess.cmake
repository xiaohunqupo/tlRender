include(ExternalProject)

set(subprocess_GIT_REPOSITORY "https://github.com/sheredom/subprocess.h.git")
set(subprocess_GIT_TAG "b0c3e4f6491da6aef71ffa3beaa083e22f6855f5")

ExternalProject_Add(
    subprocess
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/subprocess
    GIT_REPOSITORY ${subprocess_GIT_REPOSITORY}
    GIT_TAG ${subprocess_GIT_TAG}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/include
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/subprocess/src/subprocess/subprocess.h
            ${CMAKE_INSTALL_PREFIX}/include/subprocess.h)
