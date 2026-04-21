include(ExternalProject)

set(ftk_GIT_REPOSITORY "https://github.com/grizzlypeak3d/feather-tk.git")
set(ftk_GIT_TAG "f17c04790a887235a7ee8f5fe8ddef6cafae7a3e")

set(ftk_DEPS ZLIB PNG)
set(ftk_ARGS
    -Dftk_API=${ftk_API}
    -Dftk_ZLIB=OFF
    -Dftk_PNG=OFF
    -Dftk_PYTHON=${TLRENDER_PYTHON}
    -Dftk_TESTS=OFF
    -Dftk_EXAMPLES=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    ftk-sb
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk-sb
    DEPENDS ${ftk_DEPS}
    GIT_REPOSITORY ${ftk_GIT_REPOSITORY}
    GIT_TAG ${ftk_GIT_TAG}
    INSTALL_COMMAND ""
    SOURCE_SUBDIR etc/SuperBuild
    LIST_SEPARATOR |
    CMAKE_ARGS ${ftk_ARGS})

ExternalProject_Add(
    ftk
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk
    DEPENDS ftk-sb
    GIT_REPOSITORY ${ftk_GIT_REPOSITORY}
    GIT_TAG ${ftk_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${ftk_ARGS})
