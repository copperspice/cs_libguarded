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

Our YouTube channel contains over 65 videos about C++, programming fundamentals, Unicode/Strings, multithreading,
graphics, CopperSpice, DoxyPress, and other software development topics.

https://www.youtube.com/copperspice


Links to additional vidoes can be found on our website.

https://www.copperspice.com/presentations.html


### Authors / Contributors

* **Ansel Sermersheim**
* **Barbara Geller**
* **Casey Bodley**
* **Jan Wilmans**
* **Eric Lemanissier**


### License

This library is released under the BSD 2-clause license. For more information refer to the LICENSE file provided with
this project.


### References

* Website:  https://www.copperspice.com
* Twitter:  https://twitter.com/copperspice_cpp
* Email:    info@copperspice.com

* Github:   https://github.com/copperspice

* Forum:    https://forum.copperspice.com
* Journal:  https://journal.copperspice.com
