//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/func/assignment.hpp>
#include <dynd/kernels/assignment_kernels.hpp>

using namespace std;
using namespace dynd;

map<array<type_id_t, 2>, nd::arrfunc> nd::assign::make_children()
{
  typedef type_id_sequence<bool_type_id, int8_type_id, int16_type_id,
                           int32_type_id, int64_type_id, uint8_type_id,
                           uint16_type_id, uint32_type_id, uint64_type_id,
                           float32_type_id, float64_type_id> numeric_type_ids;

  map<std::array<type_id_t, 2>, arrfunc> children2 =
      arrfunc::make_all<bind<assign_error_mode, assignment_kernel>::type,
                        numeric_type_ids, numeric_type_ids>(0);

  return children2;
}

struct nd::assign nd::assign;

static nd::make_t assign_make[builtin_type_id_count - 2][builtin_type_id_count -
                                                         2][4] = {
#define SINGLE_OPERATION_PAIR_LEVEL(dst_type_id, src_type_id, errmode)         \
  &nd::make<nd::assignment_kernel<dst_type_id, src_type_id, errmode>>

#define ERROR_MODE_LEVEL(dst_type, src_type)                                   \
  {                                                                            \
    SINGLE_OPERATION_PAIR_LEVEL(dst_type, src_type, assign_error_nocheck),     \
        SINGLE_OPERATION_PAIR_LEVEL(dst_type, src_type,                        \
                                    assign_error_overflow),                    \
        SINGLE_OPERATION_PAIR_LEVEL(dst_type, src_type,                        \
                                    assign_error_fractional),                  \
        SINGLE_OPERATION_PAIR_LEVEL(dst_type, src_type, assign_error_inexact)  \
  }

#define SRC_TYPE_LEVEL(dst_type_id)                                            \
  {                                                                            \
    ERROR_MODE_LEVEL(dst_type_id, bool_type_id),                               \
        ERROR_MODE_LEVEL(dst_type_id, int8_type_id),                           \
        ERROR_MODE_LEVEL(dst_type_id, int16_type_id),                          \
        ERROR_MODE_LEVEL(dst_type_id, int32_type_id),                          \
        ERROR_MODE_LEVEL(dst_type_id, int64_type_id),                          \
        ERROR_MODE_LEVEL(dst_type_id, int128_type_id),                         \
        ERROR_MODE_LEVEL(dst_type_id, uint8_type_id),                          \
        ERROR_MODE_LEVEL(dst_type_id, uint16_type_id),                         \
        ERROR_MODE_LEVEL(dst_type_id, uint32_type_id),                         \
        ERROR_MODE_LEVEL(dst_type_id, uint64_type_id),                         \
        ERROR_MODE_LEVEL(dst_type_id, uint128_type_id),                        \
        ERROR_MODE_LEVEL(dst_type_id, float16_type_id),                        \
        ERROR_MODE_LEVEL(dst_type_id, float32_type_id),                        \
        ERROR_MODE_LEVEL(dst_type_id, float64_type_id),                        \
        ERROR_MODE_LEVEL(dst_type_id, float128_type_id),                       \
        ERROR_MODE_LEVEL(dst_type_id, complex_float32_type_id),                \
        ERROR_MODE_LEVEL(dst_type_id, complex_float64_type_id)                 \
  }

    SRC_TYPE_LEVEL(bool_type_id), SRC_TYPE_LEVEL(int8_type_id),
    SRC_TYPE_LEVEL(int16_type_id), SRC_TYPE_LEVEL(int32_type_id),
    SRC_TYPE_LEVEL(int64_type_id), SRC_TYPE_LEVEL(int128_type_id),
    SRC_TYPE_LEVEL(uint8_type_id), SRC_TYPE_LEVEL(uint16_type_id),
    SRC_TYPE_LEVEL(uint32_type_id), SRC_TYPE_LEVEL(uint64_type_id),
    SRC_TYPE_LEVEL(uint128_type_id), SRC_TYPE_LEVEL(float16_type_id),
    SRC_TYPE_LEVEL(float32_type_id), SRC_TYPE_LEVEL(float64_type_id),
    SRC_TYPE_LEVEL(float128_type_id), SRC_TYPE_LEVEL(complex_float32_type_id),
    SRC_TYPE_LEVEL(complex_float64_type_id)
#undef SRC_TYPE_LEVEL
#undef ERROR_MODE_LEVEL
#undef SINGLE_OPERATION_PAIR_LEVEL
};

size_t dynd::make_builtin_type_assignment_kernel(void *ckb, intptr_t ckb_offset,
                                                 type_id_t dst_type_id,
                                                 type_id_t src_type_id,
                                                 kernel_request_t kernreq,
                                                 assign_error_mode errmode)
{
  //  std::cout << nd::assign << std::endl;
  // std::exit(-1);

  // Do a table lookup for the built-in range of dynd types
  if (dst_type_id >= bool_type_id && dst_type_id <= complex_float64_type_id &&
      src_type_id >= bool_type_id && src_type_id <= complex_float64_type_id &&
      errmode != assign_error_default) {
    (*assign_make[dst_type_id - bool_type_id][src_type_id -
                                              bool_type_id][errmode])(
        ckb, kernreq, ckb_offset);
    return ckb_offset;
  } else {
    stringstream ss;
    ss << "Cannot assign from " << ndt::type(src_type_id) << " to "
       << ndt::type(dst_type_id);
    throw runtime_error(ss.str());
  }
}

size_t dynd::make_pod_typed_data_assignment_kernel(void *ckb,
                                                   intptr_t ckb_offset,
                                                   size_t data_size,
                                                   size_t data_alignment,
                                                   kernel_request_t kernreq)
{
  if (data_size == data_alignment) {
    // Aligned specialization tables
    switch (data_size) {
    case 1:
      nd::aligned_fixed_size_copy_assign<1>::make(ckb, kernreq, ckb_offset);
      return ckb_offset;
    case 2:
      nd::aligned_fixed_size_copy_assign<2>::make(ckb, kernreq, ckb_offset);
      return ckb_offset;
    case 4:
      nd::aligned_fixed_size_copy_assign<4>::make(ckb, kernreq, ckb_offset);
      return ckb_offset;
    case 8:
      nd::aligned_fixed_size_copy_assign<8>::make(ckb, kernreq, ckb_offset);
      return ckb_offset;
    default:
      nd::unaligned_copy_ck::make(ckb, kernreq, ckb_offset, data_size);
      return ckb_offset;
    }
  } else {
    // Unaligned specialization tables
    switch (data_size) {
    case 2:
      nd::unaligned_fixed_size_copy_assign<2>::make(ckb, kernreq, ckb_offset);
      return ckb_offset;
    case 4:
      nd::unaligned_fixed_size_copy_assign<4>::make(ckb, kernreq, ckb_offset);
      return ckb_offset;
    case 8:
      nd::unaligned_fixed_size_copy_assign<8>::make(ckb, kernreq, ckb_offset);
      return ckb_offset;
    default:
      nd::unaligned_copy_ck::make(ckb, kernreq, ckb_offset, data_size);
      return ckb_offset;
    }
  }
}

intptr_t dynd::make_assignment_kernel(
    const ndt::arrfunc_type *af_tp, void *ckb, intptr_t ckb_offset,
    const ndt::type &dst_tp, const char *dst_arrmeta, const ndt::type &src_tp,
    const char *src_arrmeta, kernel_request_t kernreq,
    const eval::eval_context *ectx, const nd::array &kwds)
{
  //  std::cout << "(make_assignment_kernel) dst_tp = " << dst_tp << std::endl;
  //  std::cout << "(make_assignment_kernel) src_tp = " << src_tp << std::endl;

  if (dst_tp.is_builtin()) {
    if (src_tp.is_builtin()) {
      if (dst_tp.extended() == src_tp.extended()) {
        return make_pod_typed_data_assignment_kernel(
            ckb, ckb_offset, dst_tp.get_data_size(),
            dst_tp.get_data_alignment(), kernreq);
      } else {
        return make_builtin_type_assignment_kernel(
            ckb, ckb_offset, dst_tp.get_type_id(), src_tp.get_type_id(),
            kernreq, ectx->errmode);
      }
    } else {
      return src_tp.extended()->make_assignment_kernel(
          af_tp, ckb, ckb_offset, dst_tp, dst_arrmeta, src_tp, src_arrmeta,
          kernreq, ectx, kwds);
    }
  } else {
    return dst_tp.extended()->make_assignment_kernel(
        af_tp, ckb, ckb_offset, dst_tp, dst_arrmeta, src_tp, src_arrmeta,
        kernreq, ectx, kwds);
  }
}

namespace {

static const expr_strided_t wrap_single_as_strided_fixedcount[7] = {
    &nd::wrap_single_as_strided_fixedcount_ck<0>::strided,
    &nd::wrap_single_as_strided_fixedcount_ck<1>::strided,
    &nd::wrap_single_as_strided_fixedcount_ck<2>::strided,
    &nd::wrap_single_as_strided_fixedcount_ck<3>::strided,
    &nd::wrap_single_as_strided_fixedcount_ck<4>::strided,
    &nd::wrap_single_as_strided_fixedcount_ck<5>::strided,
    &nd::wrap_single_as_strided_fixedcount_ck<6>::strided,
};

static void simple_wrapper_kernel_destruct(ckernel_prefix *self)
{
  self->destroy_child_ckernel(sizeof(ckernel_prefix));
}

} // anonymous namespace

size_t dynd::make_kernreq_to_single_kernel_adapter(void *ckb,
                                                   intptr_t ckb_offset,
                                                   int nsrc,
                                                   kernel_request_t kernreq)
{
  switch (kernreq) {
  case kernel_request_single: {
    return ckb_offset;
  }
  case kernel_request_strided: {
    if (nsrc >= 0 &&
        nsrc < (int)(sizeof(wrap_single_as_strided_fixedcount) /
                     sizeof(wrap_single_as_strided_fixedcount[0]))) {
      ckernel_prefix *e =
          reinterpret_cast<ckernel_builder<kernel_request_host> *>(ckb)
              ->alloc_ck<ckernel_prefix>(ckb_offset);
      e->set_function<expr_strided_t>(wrap_single_as_strided_fixedcount[nsrc]);
      e->destructor = &simple_wrapper_kernel_destruct;
      return ckb_offset;
    } else {
      nd::wrap_single_as_strided_ck *e =
          reinterpret_cast<ckernel_builder<kernel_request_host> *>(ckb)
              ->alloc_ck<nd::wrap_single_as_strided_ck>(ckb_offset);
      e->base.set_function<expr_strided_t>(
          &nd::wrap_single_as_strided_ck::strided);
      e->base.destructor = &nd::wrap_single_as_strided_ck::destruct;
      e->nsrc = nsrc;
      return ckb_offset;
    }
  }
  default: {
    stringstream ss;
    ss << "make_kernreq_to_single_kernel_adapter: unrecognized request "
       << (int)kernreq;
    throw runtime_error(ss.str());
  }
  }
}

#ifdef DYND_CUDA

intptr_t dynd::make_cuda_device_builtin_type_assignment_kernel(
    const arrfunc_type_data *DYND_UNUSED(self),
    const ndt::arrfunc_type *DYND_UNUSED(af_tp), char *DYND_UNUSED(data),
    void *ckb, intptr_t ckb_offset, const ndt::type &dst_tp,
    const char *DYND_UNUSED(dst_arrmeta), intptr_t DYND_UNUSED(nsrc),
    const ndt::type *src_tp, const char *const *DYND_UNUSED(src_arrmeta),
    kernel_request_t kernreq, const eval::eval_context *ectx,
    const nd::array &kwds, const std::map<nd::string, ndt::type> &tp_vars)
{
  assign_error_mode errmode = ectx->cuda_device_errmode;

  if (errmode != assign_error_nocheck &&
      is_lossless_assignment(dst_tp, *src_tp)) {
    errmode = assign_error_nocheck;
  }

  if (!dst_tp.is_builtin() || !src_tp->is_builtin() ||
      errmode == assign_error_default) {
    stringstream ss;
    ss << "cannot do cuda device assignment from " << *src_tp << " to "
       << dst_tp;
    throw runtime_error(ss.str());
  }

  if ((kernreq & kernel_request_cuda_device) == 0) {
    // Create a trampoline ckernel from host to device
    nd::cuda_launch_ck<1> *self =
        nd::cuda_launch_ck<1>::make(ckb, kernreq, ckb_offset, 1, 1);
    // Make the assignment on the device
    make_cuda_device_builtin_type_assignment_kernel(
        NULL, NULL, NULL, &self->ckb, 0, dst_tp, NULL, 1, src_tp, NULL,
        kernreq | kernel_request_cuda_device, ectx, kwds, tp_vars);
    // Return the offset for the original ckb
    return ckb_offset;
  }

  return make_builtin_type_assignment_kernel(
      ckb, ckb_offset, dst_tp.get_type_id(), src_tp->get_type_id(), kernreq,
      errmode);
}

intptr_t dynd::make_cuda_to_device_builtin_type_assignment_kernel(
    const arrfunc_type_data *DYND_UNUSED(self),
    const ndt::arrfunc_type *DYND_UNUSED(af_tp), char *DYND_UNUSED(data),
    void *ckb, intptr_t ckb_offset, const ndt::type &dst_tp,
    const char *DYND_UNUSED(dst_arrmeta), intptr_t DYND_UNUSED(nsrc),
    const ndt::type *src_tp, const char *const *DYND_UNUSED(src_arrmeta),
    kernel_request_t kernreq, const eval::eval_context *ectx,
    const nd::array &DYND_UNUSED(kwds),
    const std::map<nd::string, ndt::type> &DYND_UNUSED(tp_vars))
{
  assign_error_mode errmode = ectx->errmode;

  if (errmode != assign_error_nocheck &&
      is_lossless_assignment(dst_tp, *src_tp)) {
    errmode = assign_error_nocheck;
  }

  if (!dst_tp.is_builtin() || !src_tp->is_builtin() ||
      errmode == assign_error_default) {
    stringstream ss;
    ss << "cannot assign to CUDA device with types " << *src_tp << " to "
       << dst_tp;
    throw runtime_error(ss.str());
  }

  nd::cuda_host_to_device_assign_ck::make(ckb, kernreq, ckb_offset,
                                          dst_tp.get_data_size());
  return make_builtin_type_assignment_kernel(
      ckb, ckb_offset, dst_tp.get_type_id(), src_tp->get_type_id(),
      kernel_request_single, errmode);
}

intptr_t dynd::make_cuda_from_device_builtin_type_assignment_kernel(
    const arrfunc_type_data *DYND_UNUSED(self),
    const ndt::arrfunc_type *DYND_UNUSED(af_tp), char *DYND_UNUSED(data),
    void *ckb, intptr_t ckb_offset, const ndt::type &dst_tp,
    const char *DYND_UNUSED(dst_arrmeta), intptr_t DYND_UNUSED(nsrc),
    const ndt::type *src_tp, const char *const *DYND_UNUSED(src_arrmeta),
    kernel_request_t kernreq, const eval::eval_context *ectx,
    const nd::array &DYND_UNUSED(kwds),
    const std::map<nd::string, ndt::type> &DYND_UNUSED(tp_vars))
{
  assign_error_mode errmode = ectx->errmode;

  if (errmode != assign_error_nocheck &&
      is_lossless_assignment(dst_tp, *src_tp)) {
    errmode = assign_error_nocheck;
  }

  if (!dst_tp.is_builtin() || !src_tp->is_builtin() ||
      errmode == assign_error_default) {
    stringstream ss;
    ss << "cannot assign from CUDA device with types " << *src_tp << " to "
       << dst_tp;
    throw type_error(ss.str());
  }

  nd::cuda_device_to_host_assign_ck::make(ckb, kernreq, ckb_offset,
                                          src_tp->get_data_size());
  return make_builtin_type_assignment_kernel(
      ckb, ckb_offset, dst_tp.without_memory_type().get_type_id(),
      src_tp->without_memory_type().get_type_id(), kernel_request_single,
      errmode);
}

// This is meant to reflect make_pod_typed_data_assignment_kernel
size_t dynd::make_cuda_pod_typed_data_assignment_kernel(
    void *out, intptr_t offset_out, bool dst_device, bool src_device,
    size_t data_size, size_t data_alignment, kernel_request_t kernreq)
{
  if (dst_device) {
    if (src_device) {
      nd::cuda_device_to_device_copy_ck::make(out, kernreq, offset_out,
                                              data_size);
      return offset_out;
    } else {
      nd::cuda_host_to_device_copy_ck::make(out, kernreq, offset_out,
                                            data_size);
      return offset_out;
    }
  } else {
    if (src_device) {
      nd::cuda_device_to_host_copy_ck::make(out, kernreq, offset_out,
                                            data_size);
      return offset_out;
    } else {
      return make_pod_typed_data_assignment_kernel(out, offset_out, data_size,
                                                   data_alignment, kernreq);
    }
  }
}

#endif // DYND_CUDA