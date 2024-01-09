find_package(Threads REQUIRED)

include(CTest)
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS on)

add_executable(CsLibGuardedTest "")

target_link_libraries(CsLibGuardedTest
   PUBLIC
   CsLibGuarded
   Threads::Threads
)

target_sources(CsLibGuardedTest
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/catch2/catch.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/test_cow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/test_deferred.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/test_lock.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/test_read_lock.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/test_lr.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/test_ordered.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/test_rcu.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/test_shared.cpp
)

include(ParseAndAddCatchTests)
ParseAndAddCatchTests(CsLibGuardedTest)
