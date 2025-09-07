# CMake Build Script for Fastfetch
# Save this file as: build_script.cmake
# Usage: cmake -P build_script.cmake

# Configuration variables
set(PROJECT_NAME "fastfetch")
set(BUILD_DIR "build")
set(SOURCE_DIR ".")
set(BUILD_TYPE "Release")
set(INSTALL_PREFIX "/usr/local")

# Get number of processors for parallel builds
include(ProcessorCount)
ProcessorCount(N_CORES)
if(NOT N_CORES EQUAL 0)
    set(PARALLEL_JOBS ${N_CORES})
else()
    set(PARALLEL_JOBS 4)
endif()

# Color codes for output
string(ASCII 27 ESC)
set(COLOR_RESET "${ESC}[0m")
set(COLOR_RED "${ESC}[31m")
set(COLOR_GREEN "${ESC}[32m")
set(COLOR_YELLOW "${ESC}[33m")
set(COLOR_BLUE "${ESC}[34m")

# Logging functions
function(log_info MESSAGE)
    message(STATUS "${COLOR_BLUE}[INFO]${COLOR_RESET} ${MESSAGE}")
endfunction()

function(log_success MESSAGE)
    message(STATUS "${COLOR_GREEN}[SUCCESS]${COLOR_RESET} ${MESSAGE}")
endfunction()

function(log_warning MESSAGE)
    message(STATUS "${COLOR_YELLOW}[WARNING]${COLOR_RESET} ${MESSAGE}")
endfunction()

function(log_error MESSAGE)
    message(FATAL_ERROR "${COLOR_RED}[ERROR]${COLOR_RESET} ${MESSAGE}")
endfunction()

# Function to check if a command exists
function(check_command COMMAND RESULT_VAR)
    find_program(${RESULT_VAR} ${COMMAND})
    if(NOT ${RESULT_VAR})
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
    else()
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
    endif()
endfunction()

# Function to check dependencies
function(check_dependencies)
    log_info("Checking build dependencies...")
    
    set(REQUIRED_COMMANDS cmake make gcc git)
    set(MISSING_DEPS "")
    
    foreach(CMD IN LISTS REQUIRED_COMMANDS)
        check_command(${CMD} CMD_FOUND)
        if(NOT CMD_FOUND)
            list(APPEND MISSING_DEPS ${CMD})
        endif()
    endforeach()
    
    if(MISSING_DEPS)
        log_error("Missing dependencies: ${MISSING_DEPS}")
    else()
        log_success("All dependencies are available")
    endif()
endfunction()

# Function to clean build directory
function(clean_build_dir)
    log_info("Cleaning build directory...")
    if(EXISTS ${BUILD_DIR})
        file(REMOVE_RECURSE ${BUILD_DIR})
        log_success("Build directory cleaned")
    endif()
endfunction()

# Function to configure the build
function(configure_build)
    log_info("Configuring build with CMake...")
    
    file(MAKE_DIRECTORY ${BUILD_DIR})
    
    execute_process(
        COMMAND ${CMAKE_COMMAND} 
            -S ${SOURCE_DIR}
            -B ${BUILD_DIR}
            -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        RESULT_VARIABLE CONFIGURE_RESULT
        OUTPUT_VARIABLE CONFIGURE_OUTPUT
        ERROR_VARIABLE CONFIGURE_ERROR
    )
    
    if(NOT CONFIGURE_RESULT EQUAL 0)
        log_error("Configuration failed: ${CONFIGURE_ERROR}")
    else()
        log_success("Configuration completed")
    endif()
endfunction()

# Function to build the project
function(build_project)
    log_info("Building ${PROJECT_NAME}...")
    
    execute_process(
        COMMAND ${CMAKE_COMMAND} 
            --build ${BUILD_DIR} 
            --parallel ${PARALLEL_JOBS}
            --config ${BUILD_TYPE}
        RESULT_VARIABLE BUILD_RESULT
        OUTPUT_VARIABLE BUILD_OUTPUT
        ERROR_VARIABLE BUILD_ERROR
    )
    
    if(NOT BUILD_RESULT EQUAL 0)
        log_error("Build failed: ${BUILD_ERROR}")
    else()
        log_success("Build completed successfully")
    endif()
endfunction()

# Function to run tests
function(run_tests)
    log_info("Running tests...")
    
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --build ${BUILD_DIR}
            --target test
        RESULT_VARIABLE TEST_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT TEST_RESULT EQUAL 0)
        log_warning("No tests found or tests failed")
    else()
        log_success("All tests passed")
    endif()
endfunction()

# Function to install the project
function(install_project)
    log_info("Installing ${PROJECT_NAME}...")
    
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --install ${BUILD_DIR}
            --prefix ${INSTALL_PREFIX}
        RESULT_VARIABLE INSTALL_RESULT
        OUTPUT_VARIABLE INSTALL_OUTPUT
        ERROR_VARIABLE INSTALL_ERROR
    )
    
    if(NOT INSTALL_RESULT EQUAL 0)
        log_error("Installation failed: ${INSTALL_ERROR}")
    else()
        log_success("Installation completed")
    endif()
endfunction()

# Function to create package
function(create_package)
    log_info("Creating distribution package...")
    
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            --build ${BUILD_DIR}
            --target package
        RESULT_VARIABLE PACKAGE_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT PACKAGE_RESULT EQUAL 0)
        log_warning("Package creation not available or failed")
    else()
        log_success("Package created successfully")
        file(GLOB PACKAGES "${BUILD_DIR}/*.tar.gz" "${BUILD_DIR}/*.deb" "${BUILD_DIR}/*.rpm")
        if(PACKAGES)
            log_info("Generated packages:")
            foreach(PACKAGE IN LISTS PACKAGES)
                get_filename_component(PACKAGE_NAME ${PACKAGE} NAME)
                file(SIZE ${PACKAGE} PACKAGE_SIZE)
                math(EXPR PACKAGE_SIZE_KB "${PACKAGE_SIZE} / 1024")
                message("  - ${PACKAGE_NAME} (${PACKAGE_SIZE_KB} KB)")
            endforeach()
        endif()
    endif()
endfunction()

# Function to show build report
function(show_build_report)
    log_info("Build Report:")
    message("===============================================")
    message("Project: ${PROJECT_NAME}")
    message("Build Type: ${BUILD_TYPE}")
    message("Install Prefix: ${INSTALL_PREFIX}")
    message("Build Directory: ${BUILD_DIR}")
    message("Parallel Jobs: ${PARALLEL_JOBS}")
    message("===============================================")
    
    set(BINARY_PATH "${BUILD_DIR}/fastfetch")
    if(EXISTS ${BINARY_PATH})
        file(SIZE ${BINARY_PATH} BINARY_SIZE)
        math(EXPR BINARY_SIZE_KB "${BINARY_SIZE} / 1024")
        message("Binary Location: ${BINARY_PATH}")
        message("Binary Size: ${BINARY_SIZE_KB} KB")
    endif()
    
    set(INSTALLED_BINARY "${INSTALL_PREFIX}/bin/fastfetch")
    if(EXISTS ${INSTALLED_BINARY})
        message("Installed Binary: ${INSTALLED_BINARY}")
    endif()
endfunction()

# Parse command line arguments
if(DEFINED CMAKE_ARGV3)
    set(ACTION ${CMAKE_ARGV3})
else()
    set(ACTION "build")
endif()

# Handle different build types from arguments
if(CMAKE_ARGV4 STREQUAL "debug")
    set(BUILD_TYPE "Debug")
elseif(CMAKE_ARGV4 STREQUAL "release")
    set(BUILD_TYPE "Release")
elseif(CMAKE_ARGV4 STREQUAL "relwithdebinfo")
    set(BUILD_TYPE "RelWithDebInfo")
endif()

# Main execution logic
log_info("Starting ${PROJECT_NAME} build automation...")

if(ACTION STREQUAL "clean")
    clean_build_dir()
    log_success("Clean completed")
    return()
elseif(ACTION STREQUAL "check-deps")
    check_dependencies()
    return()
endif()

# Main build process
check_dependencies()

if(NOT ACTION STREQUAL "configure-only")
    clean_build_dir()
endif()

configure_build()

if(ACTION STREQUAL "configure-only")
    return()
endif()

build_project()

if(ACTION STREQUAL "test" OR ACTION STREQUAL "full")
    run_tests()
endif()

if(ACTION STREQUAL "install" OR ACTION STREQUAL "full")
    install_project()
endif()

if(ACTION STREQUAL "package" OR ACTION STREQUAL "full")
    create_package()
endif()

show_build_report()
log_success("${PROJECT_NAME} build automation completed!")
