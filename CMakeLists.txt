cmake_minimum_required(VERSION 3.25)
project(RSA LANGUAGES CXX)

find_package(Boost REQUIRED)

add_executable(RSA
    main.cpp
    src/np/function.cpp
    src/np/number_theory.cpp
)
target_compile_features(RSA PRIVATE cxx_std_20)
target_include_directories(RSA PRIVATE include)
target_link_libraries(RSA PRIVATE Boost::headers)


add_executable(test_RSA
    tests/main.cpp

    tests/src/number_theory_test.cpp
    src/np/number_theory.cpp


)
target_compile_features(test_RSA PRIVATE cxx_std_20)
target_include_directories(test_RSA PRIVATE include tests/include)
target_link_libraries(test_RSA PRIVATE Boost::headers)


enable_testing()
add_test(NAME test_RSA COMMAND test_RSA)