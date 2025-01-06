add_library(CsLibGuarded INTERFACE)
add_library(CsLibGuarded::CsLibGuarded ALIAS CsLibGuarded)

target_compile_features(CsLibGuarded
   INTERFACE
   cxx_std_20
)

target_include_directories(CsLibGuarded
   INTERFACE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CsLibGuarded>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(CS_LIBGUARDED_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_cow_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_deferred_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_plain_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_lock_guards.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_lr_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_ordered_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_rcu_guarded.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_rcu_list.h
   ${CMAKE_CURRENT_SOURCE_DIR}/src/cs_shared_guarded.h
)

install(
   TARGETS CsLibGuarded
   EXPORT CsLibGuardedLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
   FILES ${CS_LIBGUARDED_INCLUDES}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CsLibGuarded
   COMPONENT CsLibGuarded
)

install(
   EXPORT CsLibGuardedLibraryTargets
   NAMESPACE CsLibGuarded::
   FILE CsLibGuardedLibraryTargets.cmake
   DESTINATION ${PKG_PREFIX}
)
