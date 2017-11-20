## libGuarded

### Introduction

The libGuarded library is a standalone header-only library for multithreaded programming.

This library provides templated classes which prevent race conditions by controlling access
to shared data. Existing multithreading primitives like mutexes and locks are only bound
to the protected data by conventions. This makes it very easy to introduce bugs in your code
by forgetting to use the right locks before accessing a block of data. The idea of this
library is to tie the data and the locks in a type safe interface that only allows correct usage.


### System Requirements

To use libGuarded you will need a C++14 compiler and a C++14 standard library.

Currently uses the Autotools build system for building and running the unit test suite.
The library has been tested with clang thread sanitizer, multiple code reviews, and production software.


### Documentation

Class level documentation for libGuarded is available on the CopperSpice website:

www.copperspice.com/docs/libguarded/index.html


### Presentations

Multiple videos discussing libGuarded and multithreading can be found on the following pages:

www.copperspice.com/presentations.html <br>
www.youtube.com/copperspice



### Authors

* **Ansel Sermersheim** - *Initial work*
* **Barbara Geller** - *Testing, Documentation*


### License

This library is released under the BSD 2-clause license. For more information refer to the
LICENSE file provided with this project.


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
