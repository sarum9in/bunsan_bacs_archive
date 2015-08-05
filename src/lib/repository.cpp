#include <bacs/archive/repository.hpp>

#include <bacs/archive/config.hpp>
#include <bacs/archive/error.hpp>
#include <bacs/archive/problem/flags.hpp>

#include <bunsan/config/cast.hpp>
#include <bunsan/filesystem/operations.hpp>
#include <bunsan/get.hpp>
#include <bunsan/system_error.hpp>
#include <bunsan/utility/archiver.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>

#include <utility>

#include <cstdio>

namespace bacs {
namespace archive {

repository::repository(const boost::property_tree::ptree &ptree)
    : repository(bunsan::config::load<bacs::archive::config>(ptree)) {}

repository::repository(const config &config_)
    : m_flock(config_.lock),
      m_resolver(config_.resolver),
      m_location(config_.location),
      m_problem_archiver_factory(
          config_.problem.data.archiver.configured_factory()),
      m_problem(config_.problem),
      m_importer(config_.problem.importer),
      m_repository(config_.pm) {}

namespace {
template <typename T>
struct converter {
  static T call(T obj) { return std::move(obj); }
};

template <>
struct converter<problem::Revision> {
  static std::string call(const problem::Revision &h) {
    return std::string(reinterpret_cast<const char *>(h.value().data()),
                       h.value().size());
  }
};

template <typename T>
auto convert(T &&obj) {
  return converter<T>::call(std::forward<T>(obj));
}

template <typename MapType, typename Ret, typename... Args>
MapType get_all_map(repository *const this_,
                    Ret (repository::*get)(const problem::id &, Args...),
                    const problem::IdSet &id_set, Args &&... args) {
  MapType map;
  for (const problem::id &id : id_set.id()) {
    (*map.mutable_entry())[id] =
        convert((this_->*get)(id, std::forward<Args>(args)...));
  }
  return map;
}

template <typename... Args>
problem::IdSet get_all_set(repository *const this_,
                           bool (repository::*get)(const problem::id &,
                                                   Args...),
                           const problem::IdSet &id_set, Args &&... args) {
  std::unordered_set<problem::id> set;
  for (const problem::id &id : id_set.id())
    if ((this_->*get)(id, std::forward<Args>(args)...)) set.insert(id);
  problem::IdSet pset;
  *pset.mutable_id() = {set.begin(), set.end()};
  return pset;
}
}  // namespace

/* container */

problem::ImportMap repository::insert_all(
    const boost::filesystem::path &location) {
  problem::ImportMap map;
  for (boost::filesystem::directory_iterator i(location), end; i != end; ++i) {
    const problem::id id = i->path().filename().string();
    // TODO validate problem id (should be implemented in repository::insert)
    (*map.mutable_entry())[id] = insert(id, i->path());
  }
  return map;
}

problem::ImportMap repository::insert_all(
    const archiver_options &archiver_options_,
    const boost::filesystem::path &archive) {
  const bunsan::tempfile unpacked =
      bunsan::tempfile::directory_in_directory(m_location.tmpdir);
  const bunsan::utility::archiver_ptr archiver =
      archiver_options_.instance(m_resolver);
  archiver->unpack(archive, unpacked.path());
  return insert_all(unpacked.path());
}

void repository::extract_all(const problem::IdSet &id_set,
                             const boost::filesystem::path &location) {
  bunsan::filesystem::reset_dir(location);
  for (const problem::id &id : id_set.id()) {
    const boost::filesystem::path dst = location / id;
    if (!extract(id, dst) && boost::filesystem::exists(dst))
      boost::filesystem::remove_all(dst);
  }
}

void repository::extract_all(const problem::IdSet &id_set,
                             const archiver_options &archiver_options_,
                             const boost::filesystem::path &archive) {
  const bunsan::tempfile unpacked =
      bunsan::tempfile::directory_in_directory(m_location.tmpdir);
  extract_all(id_set, unpacked.path());
  archiver_options_.instance(m_resolver)
      ->pack_contents(archive, unpacked.path());
}

bunsan::tempfile repository::extract_all(
    const problem::IdSet &id_set, const archiver_options &archiver_options_) {
  bunsan::tempfile packed =
      bunsan::tempfile::regular_file_in_directory(m_location.tmpdir);
  extract_all(id_set, archiver_options_, packed.path());
  return packed;
}

problem::IdSet repository::existing(const problem::IdSet &id_set) {
  return get_all_set(this, &repository::exists, id_set);
}

problem::IdSet repository::available(const problem::IdSet &id_set) {
  return get_all_set(this, &repository::is_available, id_set);
}

bool repository::is_locked(const problem::id &id) {
  return has_flag(id, problem::flags::locked) ||
         has_flag(id, problem::flags::read_only);
}

bool repository::is_read_only(const problem::id &id) {
  return has_flag(id, problem::flags::read_only);
}
/* flags */

problem::IdSet repository::with_flag(const problem::IdSet &id_set,
                                     const problem::flag &flag) {
  return get_all_set(this, &repository::has_flag, id_set, flag);
}

problem::IdSet repository::with_flag(const problem::flag &flag) {
  return get_all_set(this, &repository::has_flag, existing(), flag);
}

problem::IdSet repository::set_flags_all(const problem::IdSet &id_set,
                                         const problem::FlagSet &flags) {
  return get_all_set(this, &repository::set_flags, id_set, flags);
}

problem::IdSet repository::unset_flags_all(const problem::IdSet &id_set,
                                           const problem::FlagSet &flags) {
  return get_all_set(this, &repository::unset_flags, id_set, flags);
}

bool repository::ignore(const problem::id &id) {
  return set_flag(id, problem::flags::ignore);
}

problem::IdSet repository::ignore_all(const problem::IdSet &id_set) {
  return get_all_set(this, &repository::ignore, id_set);
}

problem::IdSet repository::clear_flags_all(const problem::IdSet &id_set) {
  return get_all_set(this, &repository::clear_flags, id_set);
}

/* info */

problem::ImportMap repository::status_all(const problem::IdSet &id_set) {
  return get_all_map<problem::ImportMap>(this, &repository::status, id_set);
}

problem::InfoMap repository::info_all(const problem::IdSet &id_set) {
  return get_all_map<problem::InfoMap>(this, &repository::info, id_set);
}

/* repack */

problem::ImportMap repository::repack_all(const problem::IdSet &id_set) {
  return get_all_map<problem::ImportMap>(this, &repository::repack, id_set);
}

}  // namespace archive
}  // namespace bacs
