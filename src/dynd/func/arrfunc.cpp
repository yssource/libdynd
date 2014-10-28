//
// Copyright (C) 2011-14 Mark Wiebe, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/func/arrfunc.hpp>
#include <dynd/kernels/assignment_kernels.hpp>
#include <dynd/kernels/ckernel_common_functions.hpp>
#include <dynd/kernels/expr_kernels.hpp>
#include <dynd/types/expr_type.hpp>
#include <dynd/types/base_struct_type.hpp>
#include <dynd/types/tuple_type.hpp>
#include <dynd/types/property_type.hpp>
#include <dynd/type.hpp>

using namespace std;
using namespace dynd;

namespace {

////////////////////////////////////////////////////////////////
// Functions for the unary assignment as an arrfunc

static intptr_t instantiate_assignment_ckernel(
    const arrfunc_type_data *self, dynd::ckernel_builder *ckb,
    intptr_t ckb_offset, const ndt::type &dst_tp, const char *dst_arrmeta,
    const ndt::type *src_tp, const char *const *src_arrmeta,
    kernel_request_t kernreq, const eval::eval_context *ectx,
    const nd::array &DYND_UNUSED(aux), const nd::array &DYND_UNUSED(kwds))
{
  try
  {
    assign_error_mode errmode = *self->get_data_as<assign_error_mode>();
    if (dst_tp == self->get_return_type() &&
        src_tp[0] == self->get_arg_type(0)) {
      if (errmode == ectx->errmode) {
        return make_assignment_kernel(ckb, ckb_offset, dst_tp, dst_arrmeta,
                                      src_tp[0], src_arrmeta[0],
                                      kernreq, ectx);
      } else {
        eval::eval_context ectx_tmp(*ectx);
        ectx_tmp.errmode = errmode;
        return make_assignment_kernel(ckb, ckb_offset, dst_tp, dst_arrmeta,
                                      src_tp[0], src_arrmeta[0],
                                      kernreq, &ectx_tmp);
      }
    } else {
      stringstream ss;
      ss << "Cannot instantiate arrfunc for assigning from ";
      ss << self->get_arg_type(0) << " to " << self->get_return_type();
      ss << " using input type " << src_tp[0];
      ss << " and output type " << dst_tp;
      throw type_error(ss.str());
    }
  }
  catch (const std::exception &e)
  {
    cout << "exception: " << e.what() << endl;
    throw;
  }
}

////////////////////////////////////////////////////////////////
// Functions for property access as an arrfunc

static void delete_property_arrfunc_data(arrfunc_type_data *self_af)
{
    base_type_xdecref(*self_af->get_data_as<const base_type *>());
}

static intptr_t instantiate_property_ckernel(
    const arrfunc_type_data *self, dynd::ckernel_builder *ckb,
    intptr_t ckb_offset, const ndt::type &dst_tp, const char *dst_arrmeta,
    const ndt::type *src_tp, const char *const *src_arrmeta,
    kernel_request_t kernreq, const eval::eval_context *ectx,
    const nd::array &DYND_UNUSED(args), const nd::array &DYND_UNUSED(kwds))
{
  ndt::type prop_src_tp(*self->get_data_as<const base_type *>(), true);

  if (dst_tp.value_type() == prop_src_tp.value_type()) {
    if (src_tp[0] == prop_src_tp.operand_type()) {
      return make_assignment_kernel(ckb, ckb_offset, dst_tp, dst_arrmeta,
                                    prop_src_tp, src_arrmeta[0], kernreq, ectx);
    } else if (src_tp[0].value_type() == prop_src_tp.operand_type()) {
      return make_assignment_kernel(
          ckb, ckb_offset, dst_tp, dst_arrmeta,
          prop_src_tp.tcast<base_expr_type>()->with_replaced_storage_type(
              src_tp[0]),
          src_arrmeta[0], kernreq, ectx);
    }
  }

  stringstream ss;
  ss << "Cannot instantiate arrfunc for assigning from ";
  ss << self->get_arg_type(0) << " to " << self->get_return_type();
  ss << " using input type " << src_tp[0];
  ss << " and output type " << dst_tp;
  throw type_error(ss.str());
}

} // anonymous namespace

void dynd::make_arrfunc_from_assignment(const ndt::type &dst_tp,
                                        const ndt::type &src_tp,
                                        assign_error_mode errmode,
                                        arrfunc_type_data &out_af)
{
  memset(&out_af, 0, sizeof(arrfunc_type_data));
  *out_af.get_data_as<assign_error_mode>() = errmode;
  out_af.free_func = NULL;
  out_af.instantiate = &instantiate_assignment_ckernel;
  out_af.func_proto = ndt::make_funcproto(src_tp, dst_tp);
}

void dynd::make_arrfunc_from_property(const ndt::type &tp,
                                      const std::string &propname,
                                      arrfunc_type_data &out_af)
{
    if (tp.get_kind() == expr_kind) {
        stringstream ss;
        ss << "Creating an arrfunc from a property requires a non-expression"
           << ", got " << tp;
        throw type_error(ss.str());
    }
    ndt::type prop_tp = ndt::make_property(tp, propname);
    out_af.func_proto = ndt::make_funcproto(tp, prop_tp.value_type());
    out_af.free_func = &delete_property_arrfunc_data;
    *out_af.get_data_as<const base_type *>() = prop_tp.release();
    out_af.instantiate = &instantiate_property_ckernel;
}

nd::arrfunc::arrfunc(const nd::array &rhs)
{
    if (!rhs.is_null()) {
        if (rhs.get_type().get_type_id() == arrfunc_type_id) {
            if (rhs.is_immutable()) {
                const arrfunc_type_data *af =
                    reinterpret_cast<const arrfunc_type_data *>(
                        rhs.get_readonly_originptr());
                if (af->instantiate != NULL) {
                    // It's valid: immutable, arrfunc type, contains
                    // instantiate function.
                    m_value = rhs;
                } else {
                    throw invalid_argument(
                        "Require a non-empty arrfunc, "
                        "provided arrfunc has NULL "
                        "instantiate function");
                }
            } else {
                stringstream ss;
                ss << "Require an immutable arrfunc, provided arrfunc";
                rhs.get_type().extended()->print_data(
                    ss, rhs.get_arrmeta(), rhs.get_readonly_originptr());
                ss << " is not immutable";
                throw invalid_argument(ss.str());
            }
        } else {
            stringstream ss;
            ss << "Cannot implicitly convert nd::array of type "
               << rhs.get_type().value_type() << " to  arrfunc";
            throw type_error(ss.str());
        }
    }
}

nd::array nd::arrfunc::call(intptr_t narg, const nd::array *args, const kwds &kwds,
                            const eval::eval_context *ectx) const
{
  const arrfunc_type_data *af = get();

  std::vector<ndt::type> arg_tp(narg);
  for (intptr_t i = 0; i < narg; ++i) {
    arg_tp[i] = args[i].get_type();
  }

  std::vector<const char *> src_arrmeta(af->get_nsrc());
  for (intptr_t i = 0; i < af->get_nsrc(); ++i) {
    src_arrmeta[i] = args[i].get_arrmeta();
  }
  std::vector<char *> src_data(af->get_nsrc());
  for (intptr_t i = 0; i < af->get_nsrc(); ++i) {
    src_data[i] = const_cast<char *>(args[i].get_readonly_originptr());
  }

  // Pack the auxiliary arguments
  nd::array aux = pack(af->get_naux(), args + af->get_nsrc());

  // Resolve the destination type
  ndt::type dst_tp = af->resolve(af->get_nsrc(), af->get_nsrc() ? &arg_tp[0] : NULL, aux, kwds.get());

  // Construct the destination array
  nd::array res = nd::empty(dst_tp);

  // Generate and evaluate the ckernel
  ckernel_builder ckb;
  af->instantiate(af, &ckb, 0, dst_tp, res.get_arrmeta(), &arg_tp[0],
                  &src_arrmeta[0], kernel_request_single, ectx, aux, kwds.get());
  expr_single_t fn = ckb.get()->get_function<expr_single_t>();
  fn(res.get_readwrite_originptr(), src_data.empty() ? NULL : &src_data[0], ckb.get());
  return res;
}

void nd::arrfunc::call_out(intptr_t narg, const nd::array *args, const kwds &kwds,
                           const nd::array &out, const eval::eval_context *ectx)
    const
{
  const arrfunc_type_data *af = get();

  std::vector<ndt::type> arg_tp(narg);
  for (intptr_t i = 0; i < narg; ++i) {
    arg_tp[i] = args[i].get_type();
  }

  std::vector<const char *> src_arrmeta(af->get_nsrc());
  for (intptr_t i = 0; i < af->get_nsrc(); ++i) {
    src_arrmeta[i] = args[i].get_arrmeta();
  }
  std::vector<char *> src_data(af->get_nsrc());
  for (intptr_t i = 0; i < af->get_nsrc(); ++i) {
    src_data[i] = const_cast<char *>(args[i].get_readonly_originptr());
  }

  // Pack the auxiliary arguments
  nd::array aux = pack(af->get_naux(), args + af->get_nsrc());

  // Generate and evaluate the ckernel
  ckernel_builder ckb;
  af->instantiate(af, &ckb, 0, out.get_type(), out.get_arrmeta(), &arg_tp[0],
                  &src_arrmeta[0], kernel_request_single, ectx, aux, kwds.get());
  expr_single_t fn = ckb.get()->get_function<expr_single_t>();
  fn(out.get_readwrite_originptr(), src_data.empty() ? NULL : &src_data[0], ckb.get());
}
