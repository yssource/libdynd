//
// Copyright (C) 2011-16 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include "../test_memory.hpp"

#include <dynd/array.hpp>
#include <dynd/gtest.hpp>
#include <dynd/types/fixed_bytes_type.hpp>
#include <dynd/types/string_type.hpp>

using namespace std;
using namespace dynd;

template <typename T>
class Array : public Memory<T> {};

TYPED_TEST_CASE_P(Array);

TEST(Array, NullConstructor) {
  nd::array a;

  // Default-constructed nd::array is NULL and will crash if access is attempted
  EXPECT_EQ(NULL, a.get());
}

TEST(Array, FromValueConstructor) {
  nd::array a;
  // Bool
  a = nd::array(true);
  EXPECT_EQ(ndt::make_type<bool1>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  a = nd::array(bool1(true));
  EXPECT_EQ(ndt::make_type<bool1>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  // Signed int
  a = nd::array((int8_t)1);
  EXPECT_EQ(ndt::make_type<int8_t>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  a = nd::array((int16_t)1);
  EXPECT_EQ(ndt::make_type<int16_t>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  a = nd::array((int32_t)1);
  EXPECT_EQ(ndt::make_type<int32_t>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  a = nd::array((int64_t)1);
  EXPECT_EQ(ndt::make_type<int64_t>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  // Unsigned int
  a = nd::array((uint8_t)1);
  EXPECT_EQ(ndt::make_type<uint8_t>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  a = nd::array((uint16_t)1);
  EXPECT_EQ(ndt::make_type<uint16_t>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  a = nd::array((uint32_t)1);
  EXPECT_EQ(ndt::make_type<uint32_t>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  a = nd::array((uint64_t)1);
  EXPECT_EQ(ndt::make_type<uint64_t>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  // Floating point
  a = nd::array(1.0f);
  EXPECT_EQ(ndt::make_type<float>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  a = nd::array(1.0);
  EXPECT_EQ(ndt::make_type<double>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  // Complex
  a = nd::array(dynd::complex<float>(1, 1));
  EXPECT_EQ(ndt::make_type<dynd::complex<float>>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
  a = nd::array(dynd::complex<double>(1, 1));
  EXPECT_EQ(ndt::make_type<dynd::complex<double>>(), a.get_type());
  EXPECT_EQ((uint32_t)nd::read_access_flag | nd::immutable_access_flag, a.get_flags());
}

TYPED_TEST_P(Array, ScalarConstructor) {
  nd::array a;
  // Scalar nd::array
  a = nd::empty(TestFixture::MakeType(ndt::make_type<float>()));
  EXPECT_EQ(TestFixture::MakeType(ndt::make_type<float>()), a.get_type());
  EXPECT_EQ(ndt::make_type<float>(), a.get_type().get_canonical_type());
  EXPECT_EQ(nd::readwrite_access_flags, a.get_flags());
  EXPECT_TRUE(a.is_scalar());
}

TYPED_TEST_P(Array, OneDimConstructor) {
  // One-dimensional strided nd::array with one element
  nd::array a = nd::empty(TestFixture::MakeType(ndt::make_fixed_dim(1, ndt::make_type<float>())));
  EXPECT_EQ(TestFixture::MakeType(ndt::make_fixed_dim(1, ndt::make_type<float>())), a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(1, ndt::make_type<float>()), a.get_type().get_canonical_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(1u, a.get_shape().size());
  EXPECT_EQ(1, a.get_shape()[0]);
  EXPECT_EQ(1, a.get_dim_size(0));
  EXPECT_EQ(1u, a.get_strides().size());
  EXPECT_EQ(static_cast<intptr_t>(sizeof(float)), a.get_strides()[0]);

  // One-dimensional nd::array
  a = nd::empty(TestFixture::MakeType(ndt::make_fixed_dim(3, ndt::make_type<float>())));
  EXPECT_EQ(TestFixture::MakeType(ndt::make_fixed_dim(3, ndt::make_type<float>())), a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(3, ndt::make_type<float>()), a.get_type().get_canonical_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(1u, a.get_shape().size());
  EXPECT_EQ(3, a.get_shape()[0]);
  EXPECT_EQ(3, a.get_dim_size(0));
  EXPECT_EQ(1u, a.get_strides().size());
  EXPECT_EQ(sizeof(float), (unsigned)a.get_strides()[0]);
}

TYPED_TEST_P(Array, TwoDimConstructor) {
  // Two-dimensional nd::array with a size-one dimension
  nd::array a =
      nd::empty(TestFixture::MakeType(ndt::make_fixed_dim(3, ndt::make_fixed_dim(1, ndt::make_type<float>()))));
  EXPECT_EQ(TestFixture::MakeType(ndt::make_fixed_dim(3, ndt::make_fixed_dim(1, ndt::make_type<float>()))),
            a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(3, ndt::make_fixed_dim(1, ndt::make_type<float>())), a.get_type().get_canonical_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(2u, a.get_shape().size());
  EXPECT_EQ(3, a.get_shape()[0]);
  EXPECT_EQ(3, a.get_dim_size(0));
  EXPECT_EQ(1, a.get_shape()[1]);
  EXPECT_EQ(1, a.get_dim_size(1));
  EXPECT_EQ(2u, a.get_strides().size());
  EXPECT_EQ(sizeof(float), (unsigned)a.get_strides()[0]);
  EXPECT_EQ(static_cast<intptr_t>(sizeof(float)), a.get_strides()[1]);

  // Two-dimensional nd::array with a size-one dimension
  a = nd::empty(TestFixture::MakeType(ndt::make_fixed_dim(1, ndt::make_fixed_dim(3, ndt::make_type<float>()))));
  EXPECT_EQ(TestFixture::MakeType(ndt::make_fixed_dim(1, ndt::make_fixed_dim(3, ndt::make_type<float>()))),
            a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(1, ndt::make_fixed_dim(3, ndt::make_type<float>())), a.get_type().get_canonical_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(2u, a.get_shape().size());
  EXPECT_EQ(1, a.get_shape()[0]);
  EXPECT_EQ(1, a.get_dim_size(0));
  EXPECT_EQ(3, a.get_shape()[1]);
  EXPECT_EQ(3, a.get_dim_size(1));
  EXPECT_EQ(2u, a.get_strides().size());
  EXPECT_EQ(static_cast<intptr_t>(3 * sizeof(float)), a.get_strides()[0]);
  EXPECT_EQ(sizeof(float), (unsigned)a.get_strides()[1]);

  // Two-dimensional nd::array
  a = nd::empty(TestFixture::MakeType(ndt::make_fixed_dim(3, ndt::make_fixed_dim(5, ndt::make_type<float>()))));
  EXPECT_EQ(TestFixture::MakeType(ndt::make_fixed_dim(3, ndt::make_fixed_dim(5, ndt::make_type<float>()))),
            a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(3, ndt::make_fixed_dim(5, ndt::make_type<float>())), a.get_type().get_canonical_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(2u, a.get_shape().size());
  EXPECT_EQ(3, a.get_shape()[0]);
  EXPECT_EQ(3, a.get_dim_size(0));
  EXPECT_EQ(5, a.get_shape()[1]);
  EXPECT_EQ(5, a.get_dim_size(1));
  EXPECT_EQ(2u, a.get_strides().size());
  EXPECT_EQ(5 * sizeof(float), (unsigned)a.get_strides()[0]);
  EXPECT_EQ(sizeof(float), (unsigned)a.get_strides()[1]);
}

TYPED_TEST_P(Array, ThreeDimConstructor) {
  // Three-dimensional nd::array with size-one dimension
  nd::array a = nd::empty(TestFixture::MakeType(
      ndt::make_fixed_dim(1, ndt::make_fixed_dim(5, ndt::make_fixed_dim(4, ndt::make_type<float>())))));
  EXPECT_EQ(TestFixture::MakeType(
                ndt::make_fixed_dim(1, ndt::make_fixed_dim(5, ndt::make_fixed_dim(4, ndt::make_type<float>())))),
            a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(1, ndt::make_fixed_dim(5, ndt::make_fixed_dim(4, ndt::make_type<float>()))),
            a.get_type().get_canonical_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(3u, a.get_shape().size());
  EXPECT_EQ(1, a.get_shape()[0]);
  EXPECT_EQ(1, a.get_dim_size(0));
  EXPECT_EQ(5, a.get_shape()[1]);
  EXPECT_EQ(5, a.get_dim_size(1));
  EXPECT_EQ(4, a.get_shape()[2]);
  EXPECT_EQ(4, a.get_dim_size(2));
  EXPECT_EQ(3u, a.get_strides().size());
  EXPECT_EQ(static_cast<intptr_t>(20 * sizeof(float)), a.get_strides()[0]);
  EXPECT_EQ(4 * sizeof(float), (unsigned)a.get_strides()[1]);
  EXPECT_EQ(sizeof(float), (unsigned)a.get_strides()[2]);

  // Three-dimensional nd::array with size-one dimension
  a = nd::empty(TestFixture::MakeType(
      ndt::make_fixed_dim(3, ndt::make_fixed_dim(1, ndt::make_fixed_dim(4, ndt::make_type<float>())))));
  EXPECT_EQ(TestFixture::MakeType(
                ndt::make_fixed_dim(3, ndt::make_fixed_dim(1, ndt::make_fixed_dim(4, ndt::make_type<float>())))),
            a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(3, ndt::make_fixed_dim(1, ndt::make_fixed_dim(4, ndt::make_type<float>()))),
            a.get_type().get_canonical_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(3u, a.get_shape().size());
  EXPECT_EQ(3, a.get_shape()[0]);
  EXPECT_EQ(3, a.get_dim_size(0));
  EXPECT_EQ(1, a.get_shape()[1]);
  EXPECT_EQ(1, a.get_dim_size(1));
  EXPECT_EQ(4, a.get_shape()[2]);
  EXPECT_EQ(4, a.get_dim_size(2));
  EXPECT_EQ(3u, a.get_strides().size());
  EXPECT_EQ(4 * sizeof(float), (unsigned)a.get_strides()[0]);
  EXPECT_EQ(static_cast<intptr_t>(4 * sizeof(float)), a.get_strides()[1]);
  EXPECT_EQ(sizeof(float), (unsigned)a.get_strides()[2]);

  // Three-dimensional nd::array with size-one dimension
  a = nd::empty(TestFixture::MakeType(
      ndt::make_fixed_dim(3, ndt::make_fixed_dim(5, ndt::make_fixed_dim(1, ndt::make_type<float>())))));
  EXPECT_EQ(TestFixture::MakeType(
                ndt::make_fixed_dim(3, ndt::make_fixed_dim(5, ndt::make_fixed_dim(1, ndt::make_type<float>())))),
            a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(3, ndt::make_fixed_dim(5, ndt::make_fixed_dim(1, ndt::make_type<float>()))),
            a.get_type().get_canonical_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(3u, a.get_shape().size());
  EXPECT_EQ(3, a.get_shape()[0]);
  EXPECT_EQ(3, a.get_dim_size(0));
  EXPECT_EQ(5, a.get_shape()[1]);
  EXPECT_EQ(5, a.get_dim_size(1));
  EXPECT_EQ(1, a.get_shape()[2]);
  EXPECT_EQ(1, a.get_dim_size(2));
  EXPECT_EQ(3u, a.get_strides().size());
  EXPECT_EQ(5 * sizeof(float), (unsigned)a.get_strides()[0]);
  EXPECT_EQ(sizeof(float), (unsigned)a.get_strides()[1]);
  EXPECT_EQ(static_cast<intptr_t>(sizeof(float)), a.get_strides()[2]);

  // Three-dimensional nd::array
  a = nd::empty(TestFixture::MakeType(
      ndt::make_fixed_dim(3, ndt::make_fixed_dim(5, ndt::make_fixed_dim(4, ndt::make_type<float>())))));
  EXPECT_EQ(TestFixture::MakeType(
                ndt::make_fixed_dim(3, ndt::make_fixed_dim(5, ndt::make_fixed_dim(4, ndt::make_type<float>())))),
            a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(3, ndt::make_fixed_dim(5, ndt::make_fixed_dim(4, ndt::make_type<float>()))),
            a.get_type().get_canonical_type());
  EXPECT_FALSE(a.is_scalar());
  EXPECT_EQ(3u, a.get_shape().size());
  EXPECT_EQ(3, a.get_shape()[0]);
  EXPECT_EQ(3, a.get_dim_size(0));
  EXPECT_EQ(5, a.get_shape()[1]);
  EXPECT_EQ(5, a.get_dim_size(1));
  EXPECT_EQ(4, a.get_shape()[2]);
  EXPECT_EQ(4, a.get_dim_size(2));
  EXPECT_EQ(3u, a.get_strides().size());
  EXPECT_EQ(5 * 4 * sizeof(float), (unsigned)a.get_strides()[0]);
  EXPECT_EQ(4 * sizeof(float), (unsigned)a.get_strides()[1]);
  EXPECT_EQ(sizeof(float), (unsigned)a.get_strides()[2]);
}

TEST(Array, IntScalarConstructor) {
  stringstream ss;

  nd::array a = 3;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<int>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(3,\n      type=\"int32\")", ss.str());

  a = (int8_t)1;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<int8_t>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(1,\n      type=\"int8\")", ss.str());

  a = (int16_t)2;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<int16_t>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(2,\n      type=\"int16\")", ss.str());

  a = (int32_t)3;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<int32_t>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(3,\n      type=\"int32\")", ss.str());

  a = (int64_t)4;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<int64_t>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(4,\n      type=\"int64\")", ss.str());
}

TEST(Array, UIntScalarConstructor) {
  stringstream ss;

  nd::array a = (uint8_t)5;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<uint8_t>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(5,\n      type=\"uint8\")", ss.str());

  a = (uint16_t)6;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<uint16_t>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(6,\n      type=\"uint16\")", ss.str());

  a = (uint32_t)7;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<uint32_t>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(7,\n      type=\"uint32\")", ss.str());

  a = (uint64_t)8;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<uint64_t>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(8,\n      type=\"uint64\")", ss.str());
}

TEST(Array, FloatScalarConstructor) {
  stringstream ss;

  nd::array a = 3.25f;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<float>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(3.25,\n      type=\"float32\")", ss.str());

  a = 3.5;
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<double>(), a.get_type());
  ss.str("");
  ss << a;
  EXPECT_EQ("array(3.5,\n      type=\"float64\")", ss.str());

  a = dynd::complex<float>(3.14f, 1.0f);
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<dynd::complex<float>>(), a.get_type());

  a = dynd::complex<double>(3.14, 1.0);
  EXPECT_TRUE(a.is_scalar());
  EXPECT_EQ(ndt::make_type<dynd::complex<double>>(), a.get_type());
}

TEST(Array, StdVectorConstructor) {
  nd::array a;
  std::vector<float> v;

  // Empty vector
  a = v;
  EXPECT_EQ(ndt::make_fixed_dim(0, ndt::make_type<float>()), a.get_type());
  EXPECT_EQ(1, a.get_type().get_ndim());
  EXPECT_EQ(1u, a.get_shape().size());
  EXPECT_EQ(0, a.get_shape()[0]);
  EXPECT_EQ(0, a.get_dim_size(0));
  EXPECT_EQ(1u, a.get_strides().size());
  EXPECT_EQ(static_cast<intptr_t>(sizeof(float)), a.get_strides()[0]);

  // Size-10 vector
  for (int i = 0; i < 10; ++i) {
    v.push_back(i / 0.5f);
  }
  a = v;
  EXPECT_EQ(ndt::make_fixed_dim(10, ndt::make_type<float>()), a.get_type());
  EXPECT_EQ(1, a.get_type().get_ndim());
  EXPECT_EQ(1u, a.get_shape().size());
  EXPECT_EQ(10, a.get_shape()[0]);
  EXPECT_EQ(10, a.get_dim_size(0));
  EXPECT_EQ(1u, a.get_strides().size());
  EXPECT_EQ((int)sizeof(float), a.get_strides()[0]);
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(i / 0.5f, a(i).as<float>());
  }
}

TEST(Array, StdVectorStringConstructor) {
  nd::array a;
  std::vector<std::string> v;

  // Empty vector
  a = v;
  EXPECT_EQ(ndt::type("0 * string"), a.get_type());
  EXPECT_EQ(1, a.get_type().get_ndim());
  EXPECT_EQ(1u, a.get_shape().size());
  EXPECT_EQ(0, a.get_shape()[0]);
  EXPECT_EQ(0, a.get_dim_size(0));
  EXPECT_EQ(1u, a.get_strides().size());
  EXPECT_EQ(static_cast<intptr_t>(sizeof(dynd::string)), a.get_strides()[0]);

  // Size-5 vector
  v.push_back("this");
  v.push_back("is a test of");
  v.push_back("string");
  v.push_back("vectors");
  v.push_back("testing testing testing testing testing testing testing testing testing");
  a = v;
  EXPECT_EQ(ndt::type("5 * string"), a.get_type());
  EXPECT_EQ(1, a.get_type().get_ndim());
  EXPECT_EQ(1u, a.get_shape().size());
  EXPECT_EQ(5, a.get_shape()[0]);
  EXPECT_EQ(5, a.get_dim_size(0));
  EXPECT_EQ(1u, a.get_strides().size());
  EXPECT_EQ((intptr_t)a.get_type().at(0).get_data_size(), a.get_strides()[0]);
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(v[i], a(i).as<std::string>());
  }
}

TYPED_TEST_P(Array, AsScalar) {
  nd::array a;

  a = nd::empty(TestFixture::MakeType(ndt::make_type<float>()));
  a.assign(3.14f);
  EXPECT_EQ(3.14f, a.as<float>());
  EXPECT_EQ(3.14f, a.as<double>());
  if (!TestFixture::IsTypeID(cuda_device_id)) {
    EXPECT_THROW(a.as<int64_t>(), runtime_error);
  }
  EXPECT_EQ(3, a.as<int64_t>(assign_error_overflow));
  if (!TestFixture::IsTypeID(cuda_device_id)) {
    EXPECT_THROW(a.as<bool1>(), runtime_error);
    EXPECT_THROW(a.as<bool1>(assign_error_overflow), runtime_error);
  }
  EXPECT_EQ(true, a.as<bool1>(assign_error_nocheck));
  if (!TestFixture::IsTypeID(cuda_device_id)) {
    EXPECT_THROW(a.as<bool>(), runtime_error);
    EXPECT_THROW(a.as<bool>(assign_error_overflow), runtime_error);
  }
  EXPECT_EQ(true, a.as<bool>(assign_error_nocheck));

  a = nd::empty(TestFixture::MakeType(ndt::make_type<double>()));
  a.assign(3.141592653589);
  EXPECT_EQ(3.141592653589, a.as<double>());
  if (!TestFixture::IsTypeID(cuda_device_id)) {
    EXPECT_THROW(a.as<float>(assign_error_inexact), runtime_error);
  }
  EXPECT_EQ(3.141592653589f, a.as<float>());
}

TEST(Array, InitFromInitializerLists) {
  nd::array a = {1, 2, 3, 4, 5};
  EXPECT_EQ(ndt::make_type<int>(), a.get_dtype());
  ASSERT_EQ(1, a.get_ndim());
  ASSERT_EQ(5, a.get_shape()[0]);
  ASSERT_EQ(5, a.get_dim_size(0));
  EXPECT_EQ((int)sizeof(int), a.get_strides()[0]);
  const int *ptr_i = (const int *)a.cdata();
  EXPECT_EQ(1, ptr_i[0]);
  EXPECT_EQ(2, ptr_i[1]);
  EXPECT_EQ(3, ptr_i[2]);
  EXPECT_EQ(4, ptr_i[3]);
  EXPECT_EQ(5, ptr_i[4]);

#ifndef DYND_NESTED_INIT_LIST_BUG
  nd::array b = {{1., 2., 3.}, {4., 5., 6.25}};
  EXPECT_EQ(ndt::make_type<double>(), b.get_dtype());
  ASSERT_EQ(2, b.get_ndim());
  ASSERT_EQ(2, b.get_shape()[0]);
  ASSERT_EQ(2, b.get_dim_size(0));
  ASSERT_EQ(3, b.get_shape()[1]);
  ASSERT_EQ(3, b.get_dim_size(1));
  EXPECT_EQ(3 * (int)sizeof(double), b.get_strides()[0]);
  EXPECT_EQ((int)sizeof(double), b.get_strides()[1]);
  const double *ptr_d = (const double *)b.cdata();
  EXPECT_EQ(1, ptr_d[0]);
  EXPECT_EQ(2, ptr_d[1]);
  EXPECT_EQ(3, ptr_d[2]);
  EXPECT_EQ(4, ptr_d[3]);
  EXPECT_EQ(5, ptr_d[4]);
  EXPECT_EQ(6.25, ptr_d[5]);

  // Testing assignment operator with initializer list (and 3D nested list)
  a = {{{1LL, 2LL}, {-1LL, -2LL}}, {{4LL, 5LL}, {6LL, 1LL}}};
  EXPECT_EQ(ndt::make_type<long long>(), a.get_dtype());
  ASSERT_EQ(3, a.get_ndim());
  ASSERT_EQ(2, a.get_shape()[0]);
  ASSERT_EQ(2, a.get_dim_size(0));
  ASSERT_EQ(2, a.get_shape()[1]);
  ASSERT_EQ(2, a.get_dim_size(1));
  ASSERT_EQ(2, a.get_shape()[2]);
  ASSERT_EQ(2, a.get_dim_size(2));
  EXPECT_EQ(4 * (int)sizeof(long long), a.get_strides()[0]);
  EXPECT_EQ(2 * (int)sizeof(long long), a.get_strides()[1]);
  EXPECT_EQ((int)sizeof(long long), a.get_strides()[2]);
  const long long *ptr_ll = (const long long *)a.cdata();
  EXPECT_EQ(1, ptr_ll[0]);
  EXPECT_EQ(2, ptr_ll[1]);
  EXPECT_EQ(-1, ptr_ll[2]);
  EXPECT_EQ(-2, ptr_ll[3]);
  EXPECT_EQ(4, ptr_ll[4]);
  EXPECT_EQ(5, ptr_ll[5]);
  EXPECT_EQ(6, ptr_ll[6]);
  EXPECT_EQ(1, ptr_ll[7]);

// If the shape is jagged, should throw an error
//  EXPECT_THROW((a = {{1, 2, 3}, {1, 2}}), runtime_error);
// EXPECT_THROW((a = {{{1}, {2}, {3}}, {{1}, {2}, {3, 4}}}), runtime_error);
#endif // DYND_NESTED_INIT_LIST_BUG
}

TEST(Array, InitFromNestedCArray) {
  int i0[2][3] = {{1, 2, 3}, {4, 5, 6}};
  nd::array a = i0;
  EXPECT_EQ(ndt::type("2 * 3 * int"), a.get_type());
  EXPECT_JSON_EQ_ARR("[[1,2,3], [4,5,6]]", a);

  float i1[2][2][3] = {{{1, 2, 3}, {1.5f, 2.5f, 3.5f}}, {{-10, 0, -3.1f}, {9, 8, 7}}};
  a = i1;
  EXPECT_EQ(ndt::type("2 * 2 * 3 * float32"), a.get_type());
  EXPECT_JSON_EQ_ARR("[[[1,2,3], [1.5,2.5,3.5]], [[-10,0,-3.1], [9,8,7]]]", a);
}

TEST(Array, Storage) {
  int i0[2][3] = {{1, 2, 3}, {4, 5, 6}};
  nd::array a = i0;

  nd::array b = a.storage();
  EXPECT_EQ(ndt::make_fixed_dim(2, ndt::make_fixed_dim(3, ndt::make_type<int>())), a.get_type());
  EXPECT_EQ(ndt::make_fixed_dim(2, ndt::make_fixed_dim(3, ndt::make_type<ndt::fixed_bytes_type>(4, 4))), b.get_type());
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(a.get_shape(), b.get_shape());
  EXPECT_EQ(a.get_strides(), b.get_strides());
}

TEST(Array, SimplePrint) {
  int vals[3] = {1, 2, 3};
  nd::array a = vals;
  stringstream ss;
  ss << a;
  EXPECT_EQ("array([1, 2, 3],\n      type=\"3 * int32\")", ss.str());
}

TEST(Array, SimplePrintEmpty) {
  std::vector<float> v;
  nd::array a = v;
  stringstream ss;
  ss << a;
  EXPECT_EQ("array([],\n      type=\"0 * float32\")", ss.str());
}

TEST(Array, PrintBoolScalar) {
  nd::array a(true);
  stringstream ss;
  ss << a;
  EXPECT_EQ("array(True,\n      type=\"bool\")", ss.str());
}

TEST(Array, PrintBoolVector) {
  nd::array a = nd::empty(3, "bool");
  a.vals() = true;
  stringstream ss;
  ss << a;
  EXPECT_EQ("array([True, True, True],\n      type=\"3 * bool\")", ss.str());
}

TEST(Array, CArrayConstructor) {
  const int vals[5] = {0, 1, 2, 3, 4};

  nd::array a(vals);
  EXPECT_EQ(ndt::make_type<ndt::fixed_dim_type>(5, ndt::make_type<int>()), a.get_type());
  const size_stride_t *size_stride = reinterpret_cast<const size_stride_t *>(a->metadata());
  EXPECT_EQ(0, *reinterpret_cast<const int *>(a.cdata() + 0 * size_stride->stride));
  EXPECT_EQ(1, *reinterpret_cast<const int *>(a.cdata() + 1 * size_stride->stride));
  EXPECT_EQ(2, *reinterpret_cast<const int *>(a.cdata() + 2 * size_stride->stride));
  EXPECT_EQ(3, *reinterpret_cast<const int *>(a.cdata() + 3 * size_stride->stride));
  EXPECT_EQ(4, *reinterpret_cast<const int *>(a.cdata() + 4 * size_stride->stride));
}

TEST(Array, STLArrayConstructor) {
  nd::array a(array<int, 5>{0, 1, 2, 3, 4});
  EXPECT_EQ(ndt::make_type<ndt::fixed_dim_type>(5, ndt::make_type<int>()), a.get_type());
  const size_stride_t *size_stride = reinterpret_cast<const size_stride_t *>(a->metadata());
  EXPECT_EQ(0, *reinterpret_cast<const int *>(a.cdata() + 0 * size_stride->stride));
  EXPECT_EQ(1, *reinterpret_cast<const int *>(a.cdata() + 1 * size_stride->stride));
  EXPECT_EQ(2, *reinterpret_cast<const int *>(a.cdata() + 2 * size_stride->stride));
  EXPECT_EQ(3, *reinterpret_cast<const int *>(a.cdata() + 3 * size_stride->stride));
  EXPECT_EQ(4, *reinterpret_cast<const int *>(a.cdata() + 4 * size_stride->stride));

  a = array<array<int, 2>, 2>{{{0, 1}, {2, 3}}};
  EXPECT_EQ(ndt::make_type<ndt::fixed_dim_type>(2, ndt::make_type<ndt::fixed_dim_type>(2, ndt::make_type<int>())),
            a.get_type());
  size_stride = reinterpret_cast<const size_stride_t *>(a->metadata());
  EXPECT_EQ(0, *reinterpret_cast<const int *>(a.cdata() + 0 * size_stride[0].stride + 0 * size_stride[1].stride));
  EXPECT_EQ(1, *reinterpret_cast<const int *>(a.cdata() + 0 * size_stride[0].stride + 1 * size_stride[1].stride));
  EXPECT_EQ(2, *reinterpret_cast<const int *>(a.cdata() + 1 * size_stride[0].stride + 0 * size_stride[1].stride));
  EXPECT_EQ(3, *reinterpret_cast<const int *>(a.cdata() + 1 * size_stride[0].stride + 1 * size_stride[1].stride));
}

TEST(Array, STLTupleConstructor) {
  nd::array a(make_tuple(1, 2.5));
  EXPECT_EQ(ndt::make_type<ndt::tuple_type>({ndt::make_type<int>(), ndt::make_type<double>()}), a.get_type());
  const offset_t *offsets = reinterpret_cast<const offset_t *>(a->metadata());
  EXPECT_EQ(1, *reinterpret_cast<const int *>(a.cdata() + offsets[0]));
  EXPECT_EQ(2.5, *reinterpret_cast<const double *>(a.cdata() + offsets[1]));

  a = make_tuple(array<int, 5>{0, 1, 2, 3, 4}, 11.5);
  EXPECT_EQ(ndt::make_type<ndt::tuple_type>({ndt::make_type<array<int, 5>>(), ndt::make_type<double>()}), a.get_type());
  offsets = reinterpret_cast<const offset_t *>(a->metadata());
  const size_stride_t *size_stride = reinterpret_cast<const size_stride_t *>(a->metadata() + 2 * sizeof(offset_t));
  EXPECT_EQ(0, *reinterpret_cast<const int *>(a.cdata() + offsets[0] + 0 * size_stride->stride));
  EXPECT_EQ(1, *reinterpret_cast<const int *>(a.cdata() + offsets[0] + 1 * size_stride->stride));
  EXPECT_EQ(2, *reinterpret_cast<const int *>(a.cdata() + offsets[0] + 2 * size_stride->stride));
  EXPECT_EQ(3, *reinterpret_cast<const int *>(a.cdata() + offsets[0] + 3 * size_stride->stride));
  EXPECT_EQ(4, *reinterpret_cast<const int *>(a.cdata() + offsets[0] + 4 * size_stride->stride));
  EXPECT_EQ(11.5, *reinterpret_cast<const double *>(a.cdata() + offsets[1]));

  a = make_tuple(array<array<int, 2>, 2>{{{0, 1}, {2, 3}}}, 11.5);
  EXPECT_EQ(ndt::make_type<ndt::tuple_type>(
                {ndt::make_type<ndt::fixed_dim_type>(2, ndt::make_type<ndt::fixed_dim_type>(2, ndt::make_type<int>())),
                 ndt::make_type<double>()}),
            a.get_type());
  offsets = reinterpret_cast<const offset_t *>(a->metadata());
  size_stride = reinterpret_cast<const size_stride_t *>(a->metadata() + 2 * sizeof(offset_t));
  EXPECT_EQ(0, *reinterpret_cast<const int *>(a.cdata() + offsets[0] + 0 * size_stride[0].stride +
                                              0 * size_stride[1].stride));
  EXPECT_EQ(1, *reinterpret_cast<const int *>(a.cdata() + offsets[0] + 0 * size_stride[0].stride +
                                              1 * size_stride[1].stride));
  EXPECT_EQ(2, *reinterpret_cast<const int *>(a.cdata() + offsets[0] + 1 * size_stride[0].stride +
                                              0 * size_stride[1].stride));
  EXPECT_EQ(3, *reinterpret_cast<const int *>(a.cdata() + offsets[0] + 1 * size_stride[0].stride +
                                              1 * size_stride[1].stride));
  EXPECT_EQ(11.5, *reinterpret_cast<const double *>(a.cdata() + offsets[1]));
}

TEST(Array, ConstructAssign) {
  using dynd::fixed;

  nd::array a0({0, 1, 2, 3, 4}, ndt::make_type<ndt::fixed_dim_type>(5, ndt::make_type<double>()));
  EXPECT_EQ(ndt::make_type<ndt::fixed_dim_type>(5, ndt::make_type<double>()), a0.get_type());
  const auto &v0 = a0.view<fixed<double>>();
  EXPECT_EQ(0.0, v0[0]);
  EXPECT_EQ(1.0, v0[1]);
  EXPECT_EQ(2.0, v0[2]);
  EXPECT_EQ(3.0, v0[3]);
  EXPECT_EQ(4.0, v0[4]);

  nd::array a1({0, ndt::traits<int>::na(), 2, ndt::traits<int>::na(), 4},
               ndt::make_type<ndt::fixed_dim_type>(5, ndt::make_type<ndt::option_type>(ndt::make_type<int>())));
  EXPECT_EQ(ndt::make_type<ndt::fixed_dim_type>(5, ndt::make_type<ndt::option_type>(ndt::make_type<int>())),
            a1.get_type());
  const auto &v1 = a1.view<fixed<optional<int>>>();
  EXPECT_EQ(0, v1[0].value());
  EXPECT_TRUE(v1[1].is_na());
  EXPECT_EQ(2, v1[2].value());
  EXPECT_TRUE(v1[3].is_na());
  EXPECT_EQ(4, v1[4].value());

  nd::array a2({0, ndt::traits<int>::na(), 2, ndt::traits<int>::na(), 4},
               ndt::make_type<ndt::option_type>(ndt::make_type<int>()));
  EXPECT_EQ(ndt::make_type<ndt::fixed_dim_type>(5, ndt::make_type<ndt::option_type>(ndt::make_type<int>())),
            a2.get_type());
  const auto &v2 = a2.view<fixed<optional<int>>>();
  EXPECT_EQ(0, v2[0].value());
  EXPECT_TRUE(v2[1].is_na());
  EXPECT_EQ(2, v2[2].value());
  EXPECT_TRUE(v2[3].is_na());
  EXPECT_EQ(4, v2[4].value());
}

REGISTER_TYPED_TEST_CASE_P(Array, ScalarConstructor, OneDimConstructor, TwoDimConstructor, ThreeDimConstructor,
                           AsScalar);

INSTANTIATE_TYPED_TEST_CASE_P(Default, Array, DefaultMemory);
#ifdef DYND_CUDA
INSTANTIATE_TYPED_TEST_CASE_P(CUDA, Array, CUDAMemory);
#endif // DYND_CUDA
