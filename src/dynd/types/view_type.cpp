//
// Copyright (C) 2011-13 Mark Wiebe, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <vector>

#include <dynd/types/view_type.hpp>
#include <dynd/kernels/assignment_kernels.hpp>

using namespace std;
using namespace dynd;

view_type::view_type(const ndt::type& value_type, const ndt::type& operand_type)
    : base_expression_type(view_type_id, expression_kind, operand_type.get_data_size(),
                    operand_type.get_data_alignment(),
                    inherited_flags(value_type.get_flags(), operand_type.get_flags()),
                    operand_type.get_metadata_size()),
            m_value_type(value_type), m_operand_type(operand_type)
{
    if (value_type.get_data_size() != operand_type.value_type().get_data_size()) {
        std::stringstream ss;
        ss << "view_type: Cannot view " << operand_type.value_type() << " as " << value_type << " because they have different sizes";
        throw std::runtime_error(ss.str());
    }
    if (!value_type.is_pod()) {
        throw std::runtime_error("view_type: Only POD dtypes are supported");
    }
}

view_type::~view_type()
{
}

void view_type::print_data(std::ostream& o, const char *metadata, const char *data) const
{
    // Allow calling print_data in the special case that the view
    // is being used just to align the data
    if (m_operand_type.get_type_id() == fixedbytes_type_id) {
        switch (m_operand_type.get_data_size()) {
            case 1:
                m_value_type.print_data(o, metadata, data);
                return;
            case 2: {
                uint16_t tmp;
                memcpy(&tmp, data, sizeof(tmp));
                m_value_type.print_data(o, metadata, reinterpret_cast<const char *>(&tmp));
                return;
            }
            case 4: {
                uint32_t tmp;
                memcpy(&tmp, data, sizeof(tmp));
                m_value_type.print_data(o, metadata, reinterpret_cast<const char *>(&tmp));
                return;
            }
            case 8: {
                uint64_t tmp;
                memcpy(&tmp, data, sizeof(tmp));
                m_value_type.print_data(o, metadata, reinterpret_cast<const char *>(&tmp));
                return;
            }
            default: {
                vector<char> storage(m_value_type.get_data_size() + m_value_type.get_data_alignment());
                char *buffer = &storage[0];
                // Make the storage aligned as needed
                buffer = (char *)(((uintptr_t)buffer + (uintptr_t)m_value_type.get_data_alignment() - 1) & (m_value_type.get_data_alignment() - 1));
                memcpy(buffer, data, m_value_type.get_data_size());
                m_value_type.print_data(o, metadata, reinterpret_cast<const char *>(&buffer));
                return;
            }
        }
    }

    throw runtime_error("internal error: view_type::print_data isn't supposed to be called");
}

void view_type::print_type(std::ostream& o) const
{
    // Special case printing of alignment to make it more human-readable
    if (m_value_type.get_data_alignment() != 1 && m_operand_type.get_type_id() == fixedbytes_type_id &&
                    m_operand_type.get_data_alignment() == 1) {
        o << "unaligned<" << m_value_type << ">";
    } else {
        o << "view<as=" << m_value_type << ", original=" << m_operand_type << ">";
    }
}

void view_type::get_shape(size_t ndim, size_t i,
                intptr_t *out_shape, const char *DYND_UNUSED(metadata)) const
{
    if (!m_value_type.is_builtin()) {
        m_value_type.extended()->get_shape(ndim, i, out_shape, NULL);
    } else {
        stringstream ss;
        ss << "requested too many dimensions from type " << m_value_type;
        throw runtime_error(ss.str());
    }
}

bool view_type::is_lossless_assignment(const ndt::type& dst_dt, const ndt::type& src_dt) const
{
    // Treat this dtype as the value dtype for whether assignment is always lossless
    if (src_dt.extended() == this) {
        return ::dynd::is_lossless_assignment(dst_dt, m_value_type);
    } else {
        return ::dynd::is_lossless_assignment(m_value_type, src_dt);
    }
}

bool view_type::operator==(const base_type& rhs) const
{
    if (this == &rhs) {
        return true;
    } else if (rhs.get_type_id() != view_type_id) {
        return false;
    } else {
        const view_type *dt = static_cast<const view_type*>(&rhs);
        return m_value_type == dt->m_value_type;
    }
}

ndt::type view_type::with_replaced_storage_type(const ndt::type& replacement_type) const
{
    if (m_operand_type.get_kind() == expression_kind) {
        return ndt::type(new view_type(m_value_type,
                        static_cast<const base_expression_type *>(m_operand_type.extended())->with_replaced_storage_type(replacement_type)), false);
    } else {
        if (m_operand_type != replacement_type.value_type()) {
            std::stringstream ss;
            ss << "Cannot chain dtypes, because the view's storage dtype, " << m_operand_type;
            ss << ", does not match the replacement's value dtype, " << replacement_type.value_type();
            throw std::runtime_error(ss.str());
        }
        return ndt::type(new view_type(m_value_type, replacement_type), false);
    }
}

size_t view_type::make_operand_to_value_assignment_kernel(
                hierarchical_kernel *out, size_t offset_out,
                const char *DYND_UNUSED(dst_metadata), const char *DYND_UNUSED(src_metadata),
                kernel_request_t kernreq, const eval::eval_context *DYND_UNUSED(ectx)) const
{
    return ::make_pod_typed_data_assignment_kernel(out, offset_out,
                    m_value_type.get_data_size(),
                    std::min(m_value_type.get_data_alignment(), m_operand_type.get_data_alignment()),
                    kernreq);
}

size_t view_type::make_value_to_operand_assignment_kernel(
                hierarchical_kernel *out, size_t offset_out,
                const char *DYND_UNUSED(dst_metadata), const char *DYND_UNUSED(src_metadata),
                kernel_request_t kernreq, const eval::eval_context *DYND_UNUSED(ectx)) const
{
    return ::make_pod_typed_data_assignment_kernel(out, offset_out,
                    m_value_type.get_data_size(),
                    std::min(m_value_type.get_data_alignment(), m_operand_type.get_data_alignment()),
                    kernreq);
}