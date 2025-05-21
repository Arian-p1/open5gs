# Build freeDiameter library
SET(FD_LIBS ${FD_PROJECT_BINARY_DIR}/libfdcore/libfdcore.so
    ${FD_PROJECT_BINARY_DIR}/libfdproto/libfdproto.so)

# Monitor
add_library(ogsdiamonitor STATIC
    monitor.c
)

# Init
add_library(ogsdiaminit STATIC
    init.c
)

# Common
add_library(ogsdiamcommon STATIC
    common.c
)

# S6A
add_library(ogsdiams6a STATIC
    s6a.c
)

# GX
add_library(ogsdiamgx STATIC
    gx.c
)

# RX
add_library(ogsdiamrx STATIC
    rx.c
)

# Nextranet AAA
add_library(ogsdiamnextranetaaa STATIC
    nextranet-aaa.c
)

# S6B
add_library(ogsdiams6b STATIC
    s6b.c
)

# SWM
add_library(ogsdiamswm STATIC
    swm.c
)

# SWX
add_library(ogsdiamswx STATIC
    swx.c
)

# GY
add_library(ogsdiamgy STATIC
    gy.c
)

# All lib
add_library(ogsdiameter STATIC
    monitor.h
    common.h init.h context.h
    session.h event.h
    s6a.h gx.h rx.h
    s6b.h swm.h swx.h
    gy.h
    ogs-diameter-nextranet-aaa.h
)

target_include_directories(ogsdiamonitor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiaminit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiamcommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiams6a PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiamgx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiamrx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiamnextranetaaa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiams6b PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiamswm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiamswx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiamgy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ogsdiameter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(ogsdiamonitor PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiaminit PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiamcommon PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiams6a PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiamgx PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiamrx PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiamnextranetaaa PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiams6b PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiamswm PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiamswx PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiamgy PUBLIC ${FREEDIAMETER_CFLAGS})
target_compile_options(ogsdiameter PUBLIC ${FREEDIAMETER_CFLAGS})

target_link_libraries(ogsdiamonitor ${FD_LIBS} ogscore)
target_link_libraries(ogsdiaminit ${FD_LIBS} ogscore)
target_link_libraries(ogsdiamcommon ${FD_LIBS} ogscore)
target_link_libraries(ogsdiams6a ${FD_LIBS} ogscore)
target_link_libraries(ogsdiamgx ${FD_LIBS} ogscore)
target_link_libraries(ogsdiamrx ${FD_LIBS} ogscore)
target_link_libraries(ogsdiamnextranetaaa ${FD_LIBS} ogscore)
target_link_libraries(ogsdiams6b ${FD_LIBS} ogscore)
target_link_libraries(ogsdiamswm ${FD_LIBS} ogscore)
target_link_libraries(ogsdiamswx ${FD_LIBS} ogscore)
target_link_libraries(ogsdiamgy ${FD_LIBS} ogscore)
target_link_libraries(ogsdiameter 
    ${FD_LIBS} 
    ogsdiamonitor
    ogsdiaminit
    ogsdiamcommon
    ogsdiams6a
    ogsdiamgx
    ogsdiamrx
    ogsdiamnextranetaaa
    ogsdiams6b
    ogsdiamswm
    ogsdiamswx
    ogsdiamgy
    ogscore
)

# We need to link with rt to pull in clock_gettime.
if (LINUX AND NOT FREERTOS)
    target_link_libraries(ogsdiamonitor rt)
    target_link_libraries(ogsdiaminit rt)
    target_link_libraries(ogsdiamcommon rt)
    target_link_libraries(ogsdiams6a rt)
    target_link_libraries(ogsdiamgx rt)
    target_link_libraries(ogsdiamrx rt)
    target_link_libraries(ogsdiamnextranetaaa rt)
    target_link_libraries(ogsdiams6b rt)
    target_link_libraries(ogsdiamswm rt)
    target_link_libraries(ogsdiamswx rt)
    target_link_libraries(ogsdiamgy rt)
    target_link_libraries(ogsdiameter rt)
endif()

set_target_properties(ogsdiamonitor ogsdiaminit ogsdiamcommon
    ogsdiams6a ogsdiamgx ogsdiamrx ogsdiamnextranetaaa
    ogsdiams6b ogsdiamswm ogsdiamswx ogsdiamgy ogsdiameter
    PROPERTIES LINKER_LANGUAGE CXX) 