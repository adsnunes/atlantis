#!/bin/bash

set -e

# Get versions for package names
LIB_VERSION=$(make print-version)
MOMO_VERSION=$(make print-momo-version)

echo "Building all architectures..."

# Clean previous builds
make clean

# Build native arm64 (default)
echo "Building arm64 static release..."
make static-rel

echo "Building arm64 dynamic release..."
make dynamic-rel

# Build x86_64
echo "Building x86_64 static release..."
CROSS_COMPILE=x86_64 make static-rel

echo "Building x86_64 dynamic release..."
CROSS_COMPILE=x86_64 make dynamic-rel

# Build arm64e
echo "Building arm64e static release..."
CROSS_COMPILE=arm64e make static-rel

echo "Building arm64e dynamic release..."
CROSS_COMPILE=arm64e make dynamic-rel

# Create package directories
mkdir -p dist
rm -rf "dist/libai-${LIB_VERSION}"
rm -rf "dist/momo-${MOMO_VERSION}"
mkdir -p "dist/libai-${LIB_VERSION}/apple"
mkdir -p "dist/momo-${MOMO_VERSION}/apple"

# Copy libai libraries (both static and dynamic)
echo "Packaging libai libraries..."
for arch in arm64 x86_64 arm64e; do
    mkdir -p "dist/libai-${LIB_VERSION}/apple/${arch}"
    cp build/static/${arch}/release/libai.a "dist/libai-${LIB_VERSION}/apple/${arch}/"
    cp build/dynamic/${arch}/release/libai.dylib "dist/libai-${LIB_VERSION}/apple/${arch}/"
    cp build/dynamic/${arch}/release/libaibridge.dylib "dist/libai-${LIB_VERSION}/apple/${arch}/"
done

# Copy momo executables (static only)
echo "Packaging momo executables..."
for arch in arm64 x86_64 arm64e; do
    mkdir -p "dist/momo-${MOMO_VERSION}/apple/${arch}"
    cp build/static/${arch}/release/momo "dist/momo-${MOMO_VERSION}/apple/${arch}/momo"
done

# Create tarballs
echo "Creating tarballs..."
cd dist
tar -czf "libai.tar.gz" "libai-${LIB_VERSION}"
tar -czf "momo.tar.gz" "momo-${MOMO_VERSION}"
cd ..

echo "Build complete:"
echo "  Libraries: dist/libai.tar.gz"
echo "  Executables: dist/momo.tar.gz"