#include <iostream>
#include "number_theory_test.hpp"
#include "np/number_theory.hpp"


// Начальные значения
int carmichael_lambda_test1() {
    NumberTheory nt;
    if (nt.carmichael_lambda(1) != 1) return 1;
    if (nt.carmichael_lambda(2) != 1) return 1;
    if (nt.carmichael_lambda(4) != 2) return 1;
    return 0;
}

// Степени двойки: при степени >= 3
int carmichael_lambda_test2() {
    NumberTheory nt;
    if (nt.carmichael_lambda(8) != 2) return 1;
    if (nt.carmichael_lambda(16) != 4) return 1;
    if (nt.carmichael_lambda(32) != 8) return 1;
    if (nt.carmichael_lambda(65536) != 16384) return 1;
    return 0;
}

// Простые числа
int carmichael_lambda_test3() {
    NumberTheory nt;
    if (nt.carmichael_lambda(3) != 2) return 1;
    if (nt.carmichael_lambda(7) != 6) return 1;
    if (nt.carmichael_lambda(97) != 96) return 1;
    return 0;
}

// Нечётные чсила
int carmichael_lambda_test4() {
    NumberTheory nt;
    if (nt.carmichael_lambda(9) != 6) return 1;
    if (nt.carmichael_lambda(25) != 20) return 1;
    if (nt.carmichael_lambda(27) != 18) return 1;
    if (nt.carmichael_lambda(121) != 110) return 1;
    return 0;
}

// Три различных простых множителя
int carmichael_lambda_test5() {
    NumberTheory nt;
    if (nt.carmichael_lambda(561) != 80) return 1;
    if (nt.carmichael_lambda(1105) != 48) return 1;
    if (nt.carmichael_lambda(1729) != 36) return 1;
    return 0;
}

