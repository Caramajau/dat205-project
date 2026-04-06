cmake_minimum_required ( VERSION 3.10 )

project ( project )

# Find *all* shaders.
file(GLOB_RECURSE SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.frag"
)
# Separate filter for shaders.
source_group("Shaders" FILES ${SHADERS})

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# Build and link executable.
add_executable ( ${PROJECT_NAME}
    ${SOURCES}
    ${SHADERS}
    )

target_link_libraries ( ${PROJECT_NAME} labhelper )
config_build_output()
