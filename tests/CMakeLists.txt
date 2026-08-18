add_executable(polivex_core_tests
    core_tests.cpp
)

target_compile_features(polivex_core_tests PRIVATE cxx_std_20)
target_link_libraries(polivex_core_tests PRIVATE polivex_app)

add_test(NAME polivex_core_tests COMMAND polivex_core_tests)
