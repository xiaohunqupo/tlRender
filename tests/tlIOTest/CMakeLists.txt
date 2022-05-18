set(HEADERS
    CineonTest.h
    DPXTest.h
    IOTest.h
    PPMTest.h
    SGITest.h)

set(SOURCE
    CineonTest.cpp
    DPXTest.cpp
    IOTest.cpp
    PPMTest.cpp
    SGITest.cpp)

if(FFmpeg_FOUND)
    list(APPEND HEADERS FFmpegTest.h)
    list(APPEND SOURCE FFmpegTest.cpp)
endif()
if(JPEG_FOUND)
    list(APPEND HEADERS JPEGTest.h)
    list(APPEND SOURCE JPEGTest.cpp)
endif()
if(PNG_FOUND)
    list(APPEND HEADERS PNGTest.h)
    list(APPEND SOURCE PNGTest.cpp)
endif()
if(OpenEXR_FOUND)
    list(APPEND HEADERS OpenEXRTest.h)
    list(APPEND SOURCE OpenEXRTest.cpp)
endif()
if(TIFF_FOUND)
    list(APPEND HEADERS TIFFTest.h)
    list(APPEND SOURCE TIFFTest.cpp)
endif()

add_library(tlIOTest ${SOURCE} ${HEADERS})
target_link_libraries(tlIOTest tlTestLib tlIO)
set_target_properties(tlIOTest PROPERTIES FOLDER tests)