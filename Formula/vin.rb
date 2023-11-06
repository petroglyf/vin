require "formula"

class Vin < Formula
  desc "Executable for managing filtergraphs for vision"
  homepage "https://github.com/petrogly-ph/vin"
  url "https://github.com/petrogly-ph/vin/archive/refs/tags/v0.5saccade.tar.gz"
  # url "file:///Users/drobotnik/projects/vin/"
  sha256 "0a95d556cc1768e90a96ee87cc5b1a8a2423d353a30a8c5b10efa6c7f72325bf"
  license "MIT"
  version "0.7"

  # bottle do
  #   cellar :any
  #   sha1 "5466fbee57b366a41bbcec814614ee236e39bed8" => :yosemite
  #   sha1 "bde270764522e4a1d99767ca759574a99485e5ac" => :mavericks
  #   sha1 "e77d0e5f516cb41ac061e1050c8f37d0fb65b796" => :mountain_lion
  # end

  depends_on "cmake" => :build
  depends_on "qt@6" => :build
  depends_on "functional-dag@0.5" => :build

  def install
    # ENV.cxx20 if build.cxx20?
    mkdir "vin-build" do
      args = std_cmake_args
      
      system "cmake", "..", *args
      system "make", "install"
    end


    # ENV.deparallelize
    # system "./configure", *std_configure_args, "--disable-silent-rules"
    # system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    # system "make", "install"
  end

  # test do
  #   system "false"
  # end
end
