AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS=-I autotools/m4

AM_CFLAGS = -Wall -I$(srcdir)/src
AM_CXXFLAGS = -Wall -I$(srcdir)/src


libguardeddir = $(srcdir)/libguarded

libguarded_HEADERS = \
	src/libguarded/cow_guarded.hpp \
	src/libguarded/deferred_guarded.hpp \
	src/libguarded/guarded.hpp \
	src/libguarded/lr_guarded.hpp \
	src/libguarded/ordered_guarded.hpp \
	src/libguarded/rcu_guarded.hpp \
	src/libguarded/rcu_list.hpp \
	src/libguarded/shared_guarded.hpp

EXTRA_DIST= LICENSE README.md

TESTS=bin/unittest

check_PROGRAMS=bin/unittest

bin_unittest_CXXFLAGS=${AM_CXXFLAGS} ${PTHREAD_CFLAGS}
bin_unittest_LIBS=${PTHREAD_LIBS}
bin_unittest_SOURCES = \
	test/test_cow.cpp \
	test/test_guarded.cpp \
	test/test_lr.cpp \
	test/test_rcu.cpp

if CXX14
bin_unittest_SOURCES += \
	test/test_deferred.cpp \
	test/test_ordered.cpp \
	test/test_shared.cpp
endif

