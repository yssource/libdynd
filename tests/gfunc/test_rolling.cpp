//
// Copyright (C) 2011-14 Mark Wiebe, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include "inc_gtest.hpp"

#include <dynd/array.hpp>
#include <dynd/kernels/rolling_ckernel_deferred.hpp>
#include <dynd/kernels/reduction_kernels.hpp>
#include <dynd/types/ckernel_deferred_type.hpp>
#include <dynd/kernels/lift_reduction_ckernel_deferred.hpp>
#include <dynd/gfunc/call_callable.hpp>

using namespace std;
using namespace dynd;

TEST(Rolling, BuiltinSum_Kernel) {
    nd::array sum_1d =
        kernels::make_builtin_sum1d_ckernel_deferred(float64_type_id);
    nd::array rolling_sum = make_rolling_ckernel_deferred(
        ndt::type("strided * float64"), ndt::type("strided * float64"), sum_1d, 4);

    double adata[] = {1, 3, 7, 2, 9, 4, -5, 100, 2, -20, 3, 9, 18};
    nd::array a = adata;
    nd::array b = nd::empty_like(a);
    rolling_sum.f("__call__", b, a);
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(DYND_ISNAN(b(i).as<double>()));
    }
    for (int i = 3, i_end = (int)b.get_dim_size(); i < i_end; ++i) {
        double s = 0;
        for (int j = i-3; j <= i; ++j) {
            s += adata[j];
        }
        EXPECT_EQ(s, b(i).as<double>());
    }
}
