# Add include directories
include_directories(include)
include_directories(../lib/include)
#find_package(tcndnpu REQUIRED)

# Add source files
add_executable(tcnputestapp
    src/npu_test.c
    src/main.c
)

# Link libraries
target_link_libraries(tcnputestapp PRIVATE pthread dl tcndnpu)

# Install
install(TARGETS tcnputestapp DESTINATION bin)
