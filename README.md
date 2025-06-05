

<p align="center" style="font-size: 30pt;">
  <img border=0 src="https://github.com/user-attachments/assets/8afe8146-5e0d-49fe-b1e4-70bef9aba2ae" width="30%" height="30%"/> <br />
  vin : The Video INput library
</p>


The vin library is minimal (Qt and Apache Arrow only) support for media capture and visualization. It is meant to bookend the [functional-dag](https://github.com/petrogly-ph/functional-dag/) so that other cognitive components can rely on these pieces and build the real architecture. 

## Why would you want this?
The commitment being made with this cognitive architecture is that tensors are the key data structure passed between components of the system rather than CvMats. This system does _not_ support linear algebra routines and will do as much as possible to support native tensors into and out of pytorch and to do it in the most minimal way possible that is compatible with the package managers. 

### Build and install
OS Support: ![Ubuntu](https://img.shields.io/badge/-Ubuntu-grey?logo=ubuntu) ![macOS](https://img.shields.io/badge/-macOS-grey?logo=macos)

Status: [![Ubuntu Build and Test](https://github.com/petroglyf/vin/actions/workflows/ubuntu-build.yml/badge.svg?branch=main)](https://github.com/petroglyf/vin/actions/workflows/ubuntu-build.yml?query=branch%3Amain)

The build is Meson based and is built for cxx23 standards. First class build support is given to Ubuntu and MacOS through the [homebrew](https://brew.sh/) and aptitude infrastrucure.

#### Installing via homebrew
```bash
$ brew tap petroglyf/functional-dag https://github.com/petroglyf/functional-dag
$ brew tap petroglyf/vin https://github.com/petroglyf/vin
$ brew install petroglyf/vin/vin
```

#### Installing via aptitude
```bash
$ curl -s --compressed "https://petroglyf.github.io/ppa/KEY.gpg" | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/ppa.gpg >/dev/null
$ sudo curl -s --compressed -o /etc/apt/sources.list.d/packages.list "https://petroglyf.github.io/ppa/packages.list"
$ apt update
$ apt install vin
```

### Build dependencies
This project tries to minimize dependencies so as to not stack dependencies across larger projects and to make it easier to build simple layers to other languages like python. 


Build dependencies:
* [Qt6](https://www.qt.io/product/qt6)
* [Arrow](https://arrow.apache.org/)
* [Flatbuffers](https://flatbuffers.dev/)
* [λg](https://github.com/petroglyf/functional-dag)
* [Catch2](https://github.com/catchorg/Catch2) (if you want to unit test)
* [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) (if you want to lint the code as well)
* [ONNX Runtime](https://onnxruntime.ai/) (If you want to build deep learning inference support)

Runtime dependencies
* Qt6 (Gstreamer doesn't always support native libraries. Qt is a great library to support basic multimedia and OpenGL)
* Flatbuffers
* Glog
* ONNX Runtime (optional)

### How to contribute to the code
We work on issues. If you'd like to help out, take a look at the [issues](https://github.com/petrogly-ph/vin/issues), assign a free one and start coding! 

You're also welcome to perform unsolicited push requests to main and ask for a review. We'll be happy to take a look and provide feedback.

You can also help out by simply providing bug reports! 
