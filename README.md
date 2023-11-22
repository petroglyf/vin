# vin : The Video INput library
![image](https://github.com/petrogly-ph/vin/assets/3150543/2e1181e7-7a8a-466b-979c-ecb515c77722)

The vin library is minimal (Qt only) support for media capture and visualization. It is meant to bookend the [functional-dag](https://github.com/petrogly-ph/functional-dag/) so that other cognitive components can rely on these pieces and build the real architecture. 

## Why would you want this?
The commitment being made with this cognitive architecture is that tensors are the key data structure passed between components of the system rather than CvMats. This system does _not_ support linear algebra routines and will do as much as possible to support native tensors into and out of pytorch and to do it in the most minimal way possible that is compatible with the package managers. 

### Build and install
The build is CMake based and is built for cxx20 standards and we currently only support [homebrew](https://brew.sh/). 

``` bash
$ git clone https://github.com/petrogly-ph/vin
$ cd vin
$ brew install --build-from-source Formula/vin.rb
```

## User docs
TBA

### Build dependencies
This project tries to minimize dependencies so as to not stack dependencies across larger projects and to make it easier to build simple layers to other languages like python. 

Build dependencies:
* Qt6 (Gstreamer doesn't always support native libraries. Qt is a great library to support basic multimedia and OpenGL)
* Cmake
* ctest (if you want to unit test)
* clang-tidy (if you want to lint the code as well)

Dependencies during testing:
* Catch2 (for unit testing)
* Qt6 (Gstreamer doesn't always support native libraries. Qt is a great library to support basic multimedia and OpenGL)
  
Runtime dependencies
* Qt6 (Gstreamer doesn't always support native libraries. Qt is a great library to support basic multimedia and OpenGL)

### How to contribute to the code
We work on issues. If you'd like to help out, take a look at the [issues](https://github.com/petrogly-ph/vin/issues), assign a free one and start coding! 

You're also welcome to perform unsolicited push requests to main and ask for a review. We'll be happy to take a look and provide feedback.

You can also help out by simply providing bug reports! 
