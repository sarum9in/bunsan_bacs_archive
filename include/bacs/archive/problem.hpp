#pragma once

#include <bacs/archive/problem.pb.h>

#include <bacs/problem/id.hpp>
#include <bacs/problem/problem.pb.h>

#include <boost/optional.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace bacs {
namespace archive {
namespace problem {

using bacs::problem::id;
using bacs::problem::is_allowed_id;
using bacs::problem::validate_id;
using bacs::problem::Revision;
using flag = std::string;

const std::string &flag_cast(const problem::Flag &flag);
const std::string &flag_cast(const problem::Flag::Reserved flag);
Flag flag_cast(const std::string &flag);

bool is_allowed_flag(const flag &flag_);
bool is_allowed_flag(const Flag &flag);
bool is_allowed_flag_set(const FlagSet &flags);

/// \throws invalid_flag_error if !is_allowed_flag(flag_)
void validate_flag(const flag &flag_);
void validate_flag(const Flag &flag);
void validate_flag_set(const FlagSet &flags);

}  // namespace problem
}  // namespace archive
}  // namespace bacs
