require "formula"

class Vin < Formula
  desc "Executable for managing filtergraphs for vision"
  homepage "https://github.com/petrogly-ph/vin"
  url "https://github.com/petroglyf/vin/archive/refs/tags/brew-v1.0.tar.gz"
  sha256 "bd6b97311ccb1cad7556291fb2e8014982f5bd32249af2a6d257508c6dc39817"
  license "MIT"
  version "1.0rc1"

  depends_on "cmake" => :build
  depends_on "meson" => :build
  depends_on "catch2" => :test
  depends_on "ninja" => :build
  depends_on "qt@6" => :build
  depends_on "flatbuffers" => :build
  depends_on "apache-arrow" => :build
  depends_on "glog" => :build
  depends_on "functional-dag" => :build
  depends_on "clang-tidy" => :test

  
  
  def install
    system "meson", "setup", "build", *std_meson_args
    system "ninja", "-C", "build"
    system "meson", "install", "-C", "build"
  end

  test do
    system "false"
  end
end
