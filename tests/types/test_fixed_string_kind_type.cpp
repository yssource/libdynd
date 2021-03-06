//
// Copyright (C) 2011-16 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <dynd/types/fixed_string_kind_type.hpp>
#include <dynd/gtest.hpp>

using namespace std;
using namespace dynd;

TEST(FixedStringKindType, Construction) {
  ndt::type fixed_string_kind_tp = ndt::make_type<ndt::fixed_string_kind_type>();
  EXPECT_EQ(fixed_string_kind_id, fixed_string_kind_tp.get_id());
  EXPECT_EQ(string_kind_id, fixed_string_kind_tp.get_base_id());
  EXPECT_EQ(0u, fixed_string_kind_tp.get_data_alignment());
  EXPECT_EQ(0u, fixed_string_kind_tp.get_data_size());
  EXPECT_FALSE(fixed_string_kind_tp.is_expression());
  EXPECT_TRUE(fixed_string_kind_tp.is_scalar());
  EXPECT_TRUE(fixed_string_kind_tp.is_symbolic());
  // Roundtripping through a string
  EXPECT_EQ(fixed_string_kind_tp, ndt::type(fixed_string_kind_tp.str()));
}
