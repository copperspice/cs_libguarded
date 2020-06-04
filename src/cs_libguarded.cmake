add_library(CsLibGuarded INTERFACE)

target_include_directories(
   CsLibGuarded
   INTERFACE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CsLibGuarded>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(
   CsLibGuarded
   INTERFACE
   cxx_std_17
)

set(CS_LIBGUARDED_INCLUDE
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_cow_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_deferred_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_plain_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_lr_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_ordered_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_rcu_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_rcu_list.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_shared_guarded.h
)


install(
   FILES ${CS_LIBGUARDED_INCLUDE}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CsLibGuarded
   COMPONENT CsLibGuarded
)

install(
   TARGETS CsLibGuarded
   EXPORT CsLibGuarded-export
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT CsLibGuarded
)

install(
   EXPORT CsLibGuarded-export
   FILE CsLibGuarded.cmake
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CsLibGuarded
)
