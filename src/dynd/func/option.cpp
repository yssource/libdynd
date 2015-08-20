//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/func/option.hpp>
#include <dynd/kernels/assign_na_kernel.hpp>
#include <dynd/kernels/is_avail_kernel.hpp>
#include <dynd/math.hpp>

using namespace std;
using namespace dynd;

//      const callable self = functional::call<F>(ndt::type("(?Any) ->
//      bool"));

/*
      for (type_id_t i0 : dim_type_ids::vals()) {
        const ndt::type child_tp = ndt::callable::make(
self.get_type()->get_return_type(),
            {ndt::type(i0)});
        children[i0] = functional::elwise(child_tp, self);
      }

      return functional::multidispatch(self.get_array_type(), children,
                                       default_child);
*/

nd::callable nd::is_avail::children[DYND_TYPE_ID_MAX + 1];

nd::callable nd::is_avail::make()
{
  typedef type_id_sequence<
      bool_type_id, int8_type_id, int16_type_id, int32_type_id, int64_type_id,
      int128_type_id, float32_type_id, float64_type_id, complex_float32_type_id,
      complex_float64_type_id, void_type_id, string_type_id, fixed_dim_type_id,
      date_type_id, time_type_id, datetime_type_id> type_ids;

  for (const std::pair<const type_id_t, callable> &pair :
       callable::make_all<is_avail_kernel, type_ids>(0)) {
    children[pair.first] = pair.second;
  }

  // ...

  return callable();
}

// underlying_type<type_id>::type
// type_kind<type_id>::value
// type_id<type>::value

struct nd::is_avail nd::is_avail;

nd::callable nd::assign_na_decl::children[DYND_TYPE_ID_MAX + 1];

nd::callable nd::assign_na_decl::make()
{
  typedef type_id_sequence<
      bool_type_id, int8_type_id, int16_type_id, int32_type_id, int64_type_id,
      int128_type_id, float32_type_id, float64_type_id, complex_float32_type_id,
      complex_float64_type_id, void_type_id, string_type_id, fixed_dim_type_id,
      date_type_id, time_type_id, datetime_type_id> type_ids;

  for (const std::pair<const type_id_t, callable> &pair :
       callable::make_all<assign_na_kernel, type_ids>(0)) {
    children[pair.first] = pair.second;
  }

  // ...

  return callable();
}

struct nd::assign_na_decl nd::assign_na_decl;
