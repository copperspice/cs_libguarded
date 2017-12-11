## libGuarded

[![Build status](https://ci.appveyor.com/api/projects/status/0mo7qmlb227qvtuv?svg=true)](https://ci.appveyor.com/project/janwilmans/libguarded/branch/master)

### Introduction

The libGuarded library is a standalone header-only library to make correct and deadlock free multithreaded programming easier.

The basic idea is to put the knowledge of what locking mechanism belongs to what data into the type system.
Incorrect usage will result in compiler errors instead of runtime errors.

### System Requirements

To use [copperspice/libguarded](https://github.com/copperspice/libguarded) you will need a C++14 compiler. 

However, the [janwilmans/libguarded](https://github.com/janwilmans/libguarded) repository is modified to work with vs2013 and boost 1.65. So far only **guarded.hpp** and **shared_guarded.hpp** are working and tested.

[copperspice/libguarded](https://github.com/copperspice/libguarded) currently uses the Autotools build system for building and running the unit test suite. The library has been tested with clang sanitizer and a major code review. 

[janwilmans/libguarded] is not reviewed yet but CI tests are running on AppVeyor on vs2013


### Documentation

Class level documentation for libGuarded is available on the CopperSpice website:

www.copperspice.com/docs/libguarded/index.html



### Presentations

Multiple videos discussing libGuarded and multithreading can be found on the following pages:

www.youtube.com/copperspice <br>
www.copperspice.com/presentations.html



### Authors / Contributors

* **Ansel Sermersheim**
* **Barbara Geller**
* **Casey Bodley**
* **Jan Wilmans**
* **Eric Lemanissier**

### Support

* **Odin Holmes** - *Promotions and working on getting us in conan.io*


### License

This library is released under the BSD 2-clause license. For more information refer to the LICENSE file provided with this
project.


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
