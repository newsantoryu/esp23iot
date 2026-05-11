#!/bin/bash

# ESP32 Audio Pitch Engine - Build Script
# Automates the build process with various options

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="release"
TARGET="esp32doit-devkit-v1"
UPLOAD=false
MONITOR=false
CLEAN=false
VERBOSE=false

# Help function
show_help() {
    echo "ESP32 Audio Pitch Engine - Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -t, --target TARGET     Build target (default: esp32doit-devkit-v1)"
    echo "  -b, --build TYPE        Build type: debug|release (default: release)"
    echo "  -u, --upload            Upload firmware after build"
    echo "  -m, --monitor           Start serial monitor after upload"
    echo "  -c, --clean             Clean build directory before building"
    echo "  -v, --verbose           Verbose output"
    echo "  -h, --help              Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                      # Build release version"
    echo "  $0 -u -m                # Build, upload and monitor"
    echo "  $0 -b debug -c          # Clean build debug version"
    echo "  $0 -t esp32dev -u       # Build for ESP32 DevKit and upload"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--target)
            TARGET="$2"
            shift 2
            ;;
        -b|--build)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -u|--upload)
            UPLOAD=true
            shift
            ;;
        -m|--monitor)
            MONITOR=true
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo -e "${RED}Error: Unknown option $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

# Validate build type
if [[ "$BUILD_TYPE" != "debug" && "$BUILD_TYPE" != "release" ]]; then
    echo -e "${RED}Error: Build type must be 'debug' or 'release'${NC}"
    exit 1
fi

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo -e "${RED}Error: PlatformIO is not installed or not in PATH${NC}"
    echo "Please install PlatformIO: pip install platformio"
    exit 1
fi

# Check if we're in the correct directory
if [[ ! -f "platformio.ini" ]]; then
    echo -e "${RED}Error: platformio.ini not found. Please run this script from the project root.${NC}"
    exit 1
fi

echo -e "${BLUE}ESP32 Audio Pitch Engine - Build Script${NC}"
echo -e "${BLUE}======================================${NC}"
echo "Target: $TARGET"
echo "Build Type: $BUILD_TYPE"
echo "Upload: $UPLOAD"
echo "Monitor: $MONITOR"
echo "Clean: $CLEAN"
echo ""

# Clean build directory if requested
if [[ "$CLEAN" == true ]]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    if [[ "$VERBOSE" == true ]]; then
        pio run --target clean --environment "$TARGET"
    else
        pio run --target clean --environment "$TARGET" > /dev/null 2>&1
    fi
    echo -e "${GREEN}Clean completed${NC}"
fi

# Set build flags based on build type
if [[ "$BUILD_TYPE" == "debug" ]]; then
    echo -e "${YELLOW}Building debug version...${NC}"
    export BUILD_FLAGS="-DCORE_DEBUG_LEVEL=3 -DDEBUG -O0 -g"
else
    echo -e "${YELLOW}Building release version...${NC}"
    export BUILD_FLAGS="-DCORE_DEBUG_LEVEL=0 -DNDEBUG -Os"
fi

# Build the project
echo -e "${YELLOW}Compiling firmware...${NC}"
if [[ "$VERBOSE" == true ]]; then
    pio run --environment "$TARGET"
else
    pio run --environment "$TARGET" > /dev/null 2>&1
fi

# Check if build was successful
if [[ $? -eq 0 ]]; then
    echo -e "${GREEN}Build completed successfully!${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

# Upload firmware if requested
if [[ "$UPLOAD" == true ]]; then
    echo -e "${YELLOW}Uploading firmware...${NC}"
    
    # Check if device is connected
    if [[ "$VERBOSE" == true ]]; then
        pio run --target upload --environment "$TARGET"
    else
        pio run --target upload --environment "$TARGET" > /dev/null 2>&1
    fi
    
    if [[ $? -eq 0 ]]; then
        echo -e "${GREEN}Upload completed successfully!${NC}"
    else
        echo -e "${RED}Upload failed! Please check device connection.${NC}"
        exit 1
    fi
fi

# Start serial monitor if requested
if [[ "$MONITOR" == true ]]; then
    echo -e "${YELLOW}Starting serial monitor...${NC}"
    echo -e "${BLUE}Press Ctrl+C to exit monitor${NC}"
    sleep 2  # Give device time to boot
    pio device monitor --environment "$TARGET" --baud 115200
fi

echo -e "${GREEN}Build script completed successfully!${NC}"
