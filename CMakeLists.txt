cmake_minimum_required(VERSION 3.10.2)
project(TypedView VERSION 1.0.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Create library
add_library(${PROJECT_NAME} INTERFACE)

# Collect header files in include directory
file(GLOB H_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

target_sources(${PROJECT_NAME} INTERFACE "$<BUILD_INTERFACE:${H_FILES}>")
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>)
target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)

# Define install path
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${PROJECT_NAME} )
set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/${PROJECT_NAME} )

# Print install path
message(VERBOSE "Install includes in: " ${INCLUDE_INSTALL_DIR})
message(VERBOSE "Install cmake file in: " ${CMAKE_INSTALL_DIR})

# Create configuration files
configure_package_config_file( "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
                               "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                               INSTALL_DESTINATION
                               ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
                               PATH_VARS INCLUDE_INSTALL_DIR CMAKE_INSTALL_DIR
                               )

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

# Install configuration files
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DIR})

# Install target FROM to DESTINATION
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/view.h" DESTINATION ${INCLUDE_INSTALL_DIR})
