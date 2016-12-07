
#
# External dependencies
#

find_package(cpplocate REQUIRED)
find_package(glm REQUIRED)
find_package(glbinding REQUIRED)
find_package(globjects REQUIRED)
find_package(glfw REQUIRED)


# 
# Executable name and options
# 

# Target name
set(target point-based-example)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target}")


#
# Sources
#

set(sources
    main.cpp
)


#
# Create executable
#

# Build library
add_executable(${target}
    ${sources}
)

# Create namespaced alias
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

#
# Project options
#

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
)


#
# Include directories
#

target_include_directories(${target}
    PRIVATE
    ${DEFAULT_INCLUDE_DIRECTORIES}
    ${PROJECT_BINARY_DIR}/source/include
    SYSTEM
    ${GLM_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
)


#
# Libraries
#

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LIBRARIES}
    ${META_PROJECT_NAME}::openll
    cpplocate::cpplocate
    glbinding::glbinding
    globjects::globjects
    ${GLFW_LIBRARIES}
)


#
# Compile definitions
#

target_compile_definitions(${target}
    PRIVATE
    ${DEFAULT_COMPILE_DEFINITIONS}
    GLFW_INCLUDE_NONE
    GLM_FORCE_RADIANS
)


#
# Compile options
#

target_compile_options(${target}
    PRIVATE
    ${DEFAULT_COMPILE_OPTIONS}
)


#
# Linker options
#

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LINKER_OPTIONS}
)


#
# Deployment
#

# Plugin library
install(TARGETS ${target}
    RUNTIME DESTINATION ${INSTALL_EXAMPLES} COMPONENT examples
    LIBRARY DESTINATION ${INSTALL_EXAMPLES} COMPONENT examples
)
