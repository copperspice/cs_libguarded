# libguarded

This is a header only library that implements data guards for C++11 and C++14.

## Motivation

Existing multithreading primitives like mutexes and locks are only bound to the protected data by conventions. 
This makes it very easy to create bugs by forgetting to use the right locks before accessing some data.
The idea of this library is to connect the data and the locks in a type safe interface, that only allows correct usage.

## State

This is a new library. It has been tested with unit tests, sanitizers and code review. 
Multithreaded programming is hard, so we might have missed some issue.
Please try it in your projects and report any issues or suggestions.

## Contents

All interface headers are in `include/libguarded`.

* `class guarded<T>`
  * exclusive locks
  * C++11
* `class shared_guarded<T>`
  * exclusive locks
  * shared locks
  * C++14 or boost::thread (requires a shared mutex)
* `class ordered_guarded<T>`
  * shared locks
  * blocking modification to shared data (via lambda)
  * C++14 or boost::thread (requires a shared mutex)
* `class deferred_guarded<T>`
  * shared locks
  * nonblocking modification to shared data (via lambda) - Defers modification until no shared locks are active.
  * deadlock free eventual consistency - Will block the next shared lock until all writes are done.
  * C++14 or boost::thread (requires a shared mutex)
* `class lr_guarded<T>`
  * shared access without locks
  * blocking modification to shared data (via lambda)
  * readers block writers (but readers never block readers)
  * readers never see data older than the previous write
  * C++11
* `class cow_guarded<T>`
  * shared access without locks
  * blocking modification to shared data (via lambda)
  * only other writers can block writers
  * readers see a snapshot of data
  * unwanted modifications can be discarded
  * C++11

## More

introducing talk: [Andel Sermersheim: Multithreading is the answer. What was the question? Part II](https://www.youtube.com/watch?v=8HBmmHUcZZA)

<a href="http://www.youtube.com/watch?feature=player_embedded&v=8HBmmHUcZZA
" target="_blank"><img src="http://img.youtube.com/vi/8HBmmHUcZZA/0.jpg" 
alt="VIDEO" width="240" height="180" border="10" /></a>

website: http://www.copperspice.com

forum: http://forum.copperspice.com

## Future enhancements

* per element locking for guarded containers
* integration with condition variables
* locking multiple guarded objects simultaneously
* use libGuarded in CsSignal
* integration with a work queue

## License

Libguarded is free software released under the BSD 2-clause license. For more information see the LICENCE file provided with this project.
