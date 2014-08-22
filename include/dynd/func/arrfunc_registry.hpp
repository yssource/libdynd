//
// Copyright (C) 2011-14 Mark Wiebe, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#ifndef DYND__FUNC_ARRFUNC_REGISTRY_HPP
#define DYND__FUNC_ARRFUNC_REGISTRY_HPP

#include <dynd/func/arrfunc.hpp>

namespace dynd { namespace func {

/**
  * Looks up a named arrfunc from the registry.
  */
nd::arrfunc get_regfunction(const nd::string &name);
/**
  * Sets a named arrfunc in the registry.
  */
void set_regfunction(const nd::string &name, const nd::arrfunc &af);

}} // namespace dynd::func

#endif // DYND__FUNC_ARRFUNC_REGISTRY_HPP
