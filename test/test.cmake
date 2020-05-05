find_package(Boost COMPONENTS unit_test_framework)
find_package(Threads)

if (Boost_UNIT_TEST_FRAMEWORK_FOUND AND Threads_FOUND)
   add_executable(CsLibGuardedTest "")

   add_test(unit_test bin/CsLibGuardedTest)

   target_link_libraries(CsLibGuardedTest
      PUBLIC
      CsLibGuarded
      Boost::unit_test_framework
      Threads::Threads
   )

   target_sources(CsLibGuardedTest
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_cow.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_deferred.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_plain.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_lr.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_ordered.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_rcu.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_shared.cpp
   )
endif()
