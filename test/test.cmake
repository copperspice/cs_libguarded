find_package(Catch2)
find_package(Threads)

include(CTest)

if (Catch2_FOUND AND Threads_FOUND)

   add_executable(CsLibGuardedTest "")

   add_test(unit_test bin/CsLibGuardedTest)

   target_link_libraries(CsLibGuardedTest
      PUBLIC
      CsLibGuarded
      Catch2::Catch2
      Threads::Threads
   )

   target_sources(CsLibGuardedTest
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_cow.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_deferred.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_lock.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_read_lock.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_lr.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_ordered.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_rcu.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_shared.cpp
      )

    include(ParseAndAddCatchTests)
    ParseAndAddCatchTests(CsLibGuardedTest)
endif()
