class Atlantis < Formula
  desc "Distraction-free terminal writing environment with live markdown rendering"
  homepage "https://github.com/adsnunes/atlantis"
  version "VERSION"
  license "MIT"

  on_macos do
    on_arm do
      url "https://github.com/adsnunes/atlantis/releases/download/vVERSION/atlantis-macos-arm64.tar.gz"
      sha256 "SHA256_MACOS_ARM64"
    end
    on_intel do
      url "https://github.com/adsnunes/atlantis/releases/download/vVERSION/atlantis-macos-x64.tar.gz"
      sha256 "SHA256_MACOS_X64"
    end
  end

  on_linux do
    on_arm do
      url "https://github.com/adsnunes/atlantis/releases/download/vVERSION/atlantis-linux-arm64.tar.gz"
      sha256 "SHA256_LINUX_ARM64"
    end
    on_intel do
      url "https://github.com/adsnunes/atlantis/releases/download/vVERSION/atlantis-linux-x64.tar.gz"
      sha256 "SHA256_LINUX_X64"
    end
  end

  def install
    bin.install "atlantis"
  end

  test do
    assert_match "atlantis", shell_output("#{bin}/atlantis -h")
  end
end
