#!/bin/bash

set -e

DOWNLOAD_URL="https://github.com/6over3/libai/releases/latest/download/momo.tar.gz"
TEMP_DIR=$(mktemp -d)

echo "Installing momo..."

# Detect architecture
ARCH=$(uname -m)
case "$ARCH" in
    "arm64")
        MOMO_ARCH="arm64"
        ;;
    "x86_64")
        MOMO_ARCH="x86_64"
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

echo "Detected architecture: $MOMO_ARCH"

# Download and extract
echo "Downloading momo.tar.gz..."
curl -L "$DOWNLOAD_URL" -o "$TEMP_DIR/momo.tar.gz"

echo "Extracting..."
cd "$TEMP_DIR"
tar -xzf momo.tar.gz

# Find the momo binary
MOMO_BINARY=$(find . -name "momo" -path "*/apple/$MOMO_ARCH/momo" | head -1)

if [ -z "$MOMO_BINARY" ]; then
    echo "Could not find momo binary for architecture $MOMO_ARCH"
    exit 1
fi

# Determine install location
if [ -w "/usr/local/bin" ]; then
    INSTALL_DIR="/usr/local/bin"
elif [ -d "$HOME/.local/bin" ]; then
    INSTALL_DIR="$HOME/.local/bin"
    mkdir -p "$INSTALL_DIR"
else
    INSTALL_DIR="$HOME/bin"
    mkdir -p "$INSTALL_DIR"
fi

echo "Installing momo to $INSTALL_DIR..."
cp "$MOMO_BINARY" "$INSTALL_DIR/momo"
chmod +x "$INSTALL_DIR/momo"

# Clean up
rm -rf "$TEMP_DIR"

echo "Installation complete!"
echo "momo installed to: $INSTALL_DIR/momo"

# Check if install dir is in PATH
case ":$PATH:" in
    *":$INSTALL_DIR:"*)
        echo "You can now run: momo"
        ;;
    *)
        echo "Note: $INSTALL_DIR is not in your PATH"
        echo "Add this line to your shell profile (.bashrc, .zshrc, etc.):"
        echo "export PATH=\"$INSTALL_DIR:\$PATH\""
        ;;
esac