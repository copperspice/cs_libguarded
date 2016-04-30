AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS=-I autotools/m4

AM_CFLAGS = -Wall -I$(srcdir)/include
AM_CXXFLAGS = -Wall -I$(srcdir)/include


libguardeddir = $(includedir)/libguarded

libguarded_HEADERS = \
	include/libguarded/cow_guarded.hpp \
	include/libguarded/deferred_guarded.hpp \
	include/libguarded/guarded.hpp \
	include/libguarded/lr_guarded.hpp \
	include/libguarded/ordered_guarded.hpp \
	include/libguarded/shared_guarded.hpp


TESTS=bin/unittest

check_PROGRAMS=bin/unittest

bin_unittest_CXXFLAGS=${AM_CXXFLAGS} ${PTHREAD_CFLAGS}
bin_unittest_LIBS=${PTHREAD_LIBS}
bin_unittest_SOURCES = \
	test/cow_guarded.cpp \
	test/guarded.cpp \
	test/lr_guarded.cpp 

if CXX14
bin_unittest_SOURCES += \
	test/deferred_guarded.cpp \
	test/ordered_guarded.cpp \
	test/shared_guarded.cpp
endif

