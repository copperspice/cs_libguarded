#ifndef LIBGUARDED_FEATURE_CHECK_HPP
#define LIBGUARDED_FEATURE_CHECK_HPP

#ifdef HAVE_CXX14
    #include <shared_mutex>
#else
    #include <boost/thread/shared_mutex.hpp>
#endif

namespace libguarded
{

#ifdef HAVE_CXX14
    using shared_timed_mutex = std::shared_timed_mutex;
    namespace chrono         = std::chrono;
#else
    using shared_timed_mutex = boost::shared_timed_mutex;
    namespace chrono         = boost::chrono;
#endif

} // namespace libguarded

#endif
