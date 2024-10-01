# Tests to run
set(EXAMPLE_SOURCES HelloSO3)
find_package(Ceres 1.6.0 QUIET)

foreach(example_src ${EXAMPLE_SOURCES})
    add_executable(${example_src} ${example_src}.cpp)
    target_link_libraries(${example_src} sophus)
endforeach(example_src)
