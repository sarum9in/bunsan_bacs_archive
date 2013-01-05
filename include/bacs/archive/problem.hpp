#pragma once

#include "bacs/problem/common.hpp"

#include <string>
#include <unordered_set>
#include <unordered_map>

#include <boost/optional.hpp>

namespace bacs{namespace archive{namespace problem
{
    using bacs::problem::id;
    using bacs::problem::is_allowed_id;
    using bacs::problem::validate_id;
    using bacs::problem::binary;
    using bacs::problem::hash_type;
    using bacs::problem::info_type;

    typedef std::unordered_set<id> id_set;
    typedef std::unordered_map<id, boost::optional<info_type>> info_map;
    typedef std::unordered_map<id, boost::optional<hash_type>> hash_map;
    typedef std::string flag;
    typedef std::unordered_set<flag> flag_set;

    struct status_type
    {
        flag_set flags;
        hash_type hash;
    };

    typedef std::unordered_map<id, boost::optional<status_type>> status_map;

    struct import_info
    {
        boost::optional<std::string> error;  ///< null if OK, error message otherwise
        boost::optional<status_type> status; ///< null if does not exist
    };

    typedef std::unordered_map<id, import_info> import_map;

    bool is_allowed_flag(const flag &flag_);

    /// \throws invalid_flag_error if !is_allowed_flag(flag_)
    void validate_flag(const flag &flag_);
}}}
