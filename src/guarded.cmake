add_library(CsLibGuarded INTERFACE)

target_include_directories(
   CsLibGuarded
   INTERFACE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_compile_features(
   CsLibGuarded
   INTERFACE
   cxx_std_14
)

set(CS_LIBGUARDED_INCLUDE
   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_cow.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_deferred.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_guarded.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_lr.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_ordered.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_rcu.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/test_shared.hpp
)

install(
   FILES ${CS_LIBGUARDED_INCLUDE}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT Devel
)

