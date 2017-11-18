## libGuarded

### Introduction

The libGuarded library is a standalone header-only library to make correct and deadlock free multithreaded programming easier.

The basic idea is to put the knowledge of what locking mechanism belongs to what data into the type system.
Incorrect usage will result in compiler errors instead of runtime errors.

### System Requirements

To use libGuarded you will need a C++11 compiler and a C++11 standard library. Some of 
the advanced features of libGuarded leverage the C++14 standard or equivalent functionality 
in Boost.

Currently uses the Autotools build system for building and running the unit test suite. 
The library has been tested with clang sanitizer and a major code review. 


### Documentation

Class level documentation for libGuarded is available on the CopperSpice website:

www.copperspice.com/docs/libguarded/index.html


### Presentations

For additional information on multithreading refer to my presentations:

www.copperspice.com/presentations.html



### Authors

* **Ansel Sermersheim** - *Initial work*
* **Barbara Geller** - *Testing, Documentation*


### License

This library is released under the BSD 2-clause license. For more information refer to the
LICENSE file provided with this project. 


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
