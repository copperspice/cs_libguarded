## libGuarded

[![Build status](https://ci.appveyor.com/api/projects/status/0mo7qmlb227qvtuv?svg=true)](https://ci.appveyor.com/project/janwilmans/libguarded/branch/master)

### Introduction

The libGuarded library is a standalone header-only library to make correct and deadlock free multithreaded programming easier.

The basic idea is to put the knowledge of what locking mechanism belongs to what data into the type system.
Incorrect usage will result in compiler errors instead of runtime errors.

### System Requirements

To use [libGuarded](https://github.com/copperspice/libguarded) you will need a C++14 compiler. 

Currently uses the Autotools build system for building and running the unit test suite. 
The library has been tested with clang sanitizer and a major code review. 

Addendum: the [janwilmans/libguarded](https://github.com/janwilmans/libguarded) repository is modified to work with vs2013 and boost 1.65. So far only **guarded.hpp** and **shared_guarded.hpp** are working and tested.

### Documentation

Class level documentation for libGuarded is available on the CopperSpice website:

www.copperspice.com/docs/libguarded/index.html


### Presentations

For additional information on multithreading refer to my presentations:

www.copperspice.com/presentations.html



### Authors

* **Ansel Sermersheim** - *Initial work*
* **Barbara Geller** - *Testing, Documentation*
* **Jan Wilmans** - *Modifications for vs2013 and msvc testing*
* **Odin Holmes** - *Promotions and working on getting us in conan.io*


### License

This library is released under the BSD 2-clause license. For more information refer to the
LICENSE file provided with this project. 


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
