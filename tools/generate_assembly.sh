#!/bin/bash

echo "Huginn Chess Engine - Assembly Generation"
echo

CONFIG="${1:-gcc-x64-release-asm}"
echo "Configuration: $CONFIG"
echo

# Configure with assembly generation
echo "Configuring with assembly generation..."
cmake --preset "$CONFIG"

if [ $? -ne 0 ]; then
    echo "Configuration failed!"
    exit 1
fi

# Build the project
echo
echo "Building project..."
cmake --build --preset "$CONFIG"

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Generate assembly files
echo
echo "Generating assembly files..."
cmake --build "build/$CONFIG" --target generate_assembly

if [ $? -ne 0 ]; then
    echo "Assembly generation failed!"
    exit 1
fi

# Create organized directory structure for comparison
echo
echo "Organizing files for side-by-side comparison..."

BUILD_DIR="build/$CONFIG"
ANALYSIS_DIR="$BUILD_DIR/analysis"

mkdir -p "$ANALYSIS_DIR/cpp"
mkdir -p "$ANALYSIS_DIR/asm"

# Copy relevant C++ source files
echo "Copying C++ source files..."
cp src/bitboard.cpp "$ANALYSIS_DIR/cpp/" 2>/dev/null
cp src/movegen_enhanced.cpp "$ANALYSIS_DIR/cpp/" 2>/dev/null
cp src/attack_detection.cpp "$ANALYSIS_DIR/cpp/" 2>/dev/null
cp src/position.cpp "$ANALYSIS_DIR/cpp/" 2>/dev/null
cp src/search.cpp "$ANALYSIS_DIR/cpp/" 2>/dev/null
cp src/hybrid_evaluation.cpp "$ANALYSIS_DIR/cpp/" 2>/dev/null
cp src/simple_search.cpp "$ANALYSIS_DIR/cpp/" 2>/dev/null

# Copy assembly files
echo "Copying assembly files..."
if ls "$BUILD_DIR/asm"/*.asm 1> /dev/null 2>&1; then
    cp "$BUILD_DIR/asm"/*.asm "$ANALYSIS_DIR/asm/" 2>/dev/null
    echo "MSVC assembly files (.asm) copied."
fi
if ls "$BUILD_DIR/asm"/*.s 1> /dev/null 2>&1; then
    cp "$BUILD_DIR/asm"/*.s "$ANALYSIS_DIR/asm/" 2>/dev/null
    echo "GCC assembly files (.s) copied."
fi

echo
echo "========================================"
echo "Assembly generation completed!"
echo
echo "Files organized in: $ANALYSIS_DIR"
echo
echo "C++ source files: $ANALYSIS_DIR/cpp/"
echo "Assembly files:   $ANALYSIS_DIR/asm/"
echo
echo "Key files for performance analysis:"
echo "- bitboard.cpp/.s           (Bit manipulation)"
echo "- movegen_enhanced.cpp/.s   (Move generation)"
echo "- attack_detection.cpp/.s   (Attack calculations)"
echo "- position.cpp/.s           (Position management)"
echo "- search.cpp/.s             (Search algorithm)"
echo "- hybrid_evaluation.cpp/.s  (Advanced evaluation)"
echo
echo "You can now open both .cpp and .s files side by side"
echo "in your editor for performance analysis."
echo "========================================"
echo

# List generated assembly files
echo "Generated assembly files:"
ls -1 "$ANALYSIS_DIR/asm/"* 2>/dev/null | xargs -I {} basename {}

echo
