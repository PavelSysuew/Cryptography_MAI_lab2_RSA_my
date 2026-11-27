cmake_minimum_required(VERSION 3.25)
project(RSA LANGUAGES CXX)

find_package(Boost REQUIRED)

set(PRIMALITY_SOURCES
    src/primality/primality_test.cpp
    src/primality/fermat_test.cpp
    src/primality/solovay_strassen_test.cpp
    src/primality/miller_rabin_test.cpp
)

add_executable(RSA
    main.cpp
    src/function.cpp
    src/np/number_theory.cpp
    src/primality/primality_menu.cpp
    src/rsa/rsa.cpp
    src/rsa/rsa_file.cpp
    src/rsa/rsa_menu.cpp
    ${PRIMALITY_SOURCES}
)
target_compile_features(RSA PRIVATE cxx_std_20)
target_include_directories(RSA PRIVATE include)
target_link_libraries(RSA PRIVATE Boost::headers)


add_executable(test_1_RSA
    tests/np/main.cpp

    tests/np/src/number_theory_test.cpp
    src/np/number_theory.cpp


)
target_compile_features(test_1_RSA PRIVATE cxx_std_20)
target_include_directories(test_1_RSA PRIVATE include tests/np/include)
target_link_libraries(test_1_RSA PRIVATE Boost::headers)

add_executable(test_2_RSA
    tests/primality/main.cpp
    tests/primality/src/primality_test_cases.cpp
    src/np/number_theory.cpp
    ${PRIMALITY_SOURCES}
)
target_compile_features(test_2_RSA PRIVATE cxx_std_20)
target_include_directories(test_2_RSA PRIVATE include tests/primality/include)
target_link_libraries(test_2_RSA PRIVATE Boost::headers)

add_executable(test_3_RSA
    tests/rsa/main.cpp
    tests/rsa/src/rsa_test.cpp
    src/np/number_theory.cpp
    src/rsa/rsa.cpp
    src/rsa/rsa_file.cpp
    ${PRIMALITY_SOURCES}
)
target_compile_features(test_3_RSA PRIVATE cxx_std_20)
target_include_directories(test_3_RSA PRIVATE include tests/rsa/include)
target_link_libraries(test_3_RSA PRIVATE Boost::headers)

enable_testing()
add_test(NAME test_1_RSA COMMAND test_1_RSA)
add_test(NAME test_2_RSA COMMAND test_2_RSA)
add_test(NAME test_3_RSA COMMAND test_3_RSA)
