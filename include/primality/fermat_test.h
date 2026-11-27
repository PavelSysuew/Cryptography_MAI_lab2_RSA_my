#pragma once

#include "primality/primality_test.h"


class FermatTest : public PrimalityTest {
protected:
    bool single_iteration(const BigInt& n, const BigInt& a) const override;
    long double log_error_bound(const BigInt& n) const override;
};
