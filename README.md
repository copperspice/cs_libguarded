## CsLibGuarded

### Introduction

The CsLibGuarded library is a standalone header only library for multithreaded programming.

This library provides templated classes which prevent race conditions by controlling access to shared data. Existing
multithreading primitives like mutexes and locks are only bound to the protected data by conventions. This makes it
very easy to introduce bugs in your code by forgetting to use the right locks before accessing a block of data. The
idea of this library is to tie the data and the locks in a type safe interface that only allows correct usage.


### System Requirements

To use CsLibGuarded you will need a C++17 compiler which fully supports the C++17 standard library.

CMake is only for building and running the unit test suite. This library has been tested with clang thread sanitizer,
multiple code reviews, and production software.


### Documentation

Class level documentation for CsLibGuarded is available on the CopperSpice website:

www.copperspice.com/docs/cs_libguarded/index.html



### Presentations

Our YouTube channel contains videos about modern C++, graphics, build systems, CopperSpice, DoxyPress, and other
topics related to software development.

https://www.youtube.com/copperspice


Multiple videos discussing CsLibGuarded and multithreading can be found on the following page:

www.copperspice.com/presentations.html



### Authors / Contributors

* **Ansel Sermersheim**
* **Barbara Geller**
* **Casey Bodley**
* **Jan Wilmans**
* **Eric Lemanissier**


### License

This library is released under the BSD 2-clause license. For more information refer to the LICENSE file provided with this
project.


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
