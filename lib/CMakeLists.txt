# Use standard install variables
include(GNUInstallDirs)

# Include header files
include_directories(include)

# source files
file(GLOB SOURCES src/*.c)
add_library(tcndnpu SHARED ${SOURCES})

# Exports tcndnpu target as tcndnpuConfig
install(TARGETS tcndnpu EXPORT tcndnpuConfig)

# Install tcndnpuConfig.cmake file & make it accessible via find_package()
install(EXPORT tcndnpuConfig FILE tcndnpuConfig.cmake DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/tcndnpu EXPORT_LINK_INTERFACE_LIBRARIES)

# Install library
install(TARGETS tcndnpu DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Install library headers
file(GLOB HEADERS include/*.h)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install pkgconfig env file
configure_file(tcndnpu.pc.in tcndnpu.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/lib/tcndnpu.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

# Add Library dependency
target_link_libraries(tcndnpu PUBLIC dl)

set_target_properties( tcndnpu PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)
