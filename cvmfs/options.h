/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_OPTIONS_H_
#define CVMFS_OPTIONS_H_

#include <stdint.h>

#include <string>
#include <vector>
#include <map>

#ifdef CVMFS_NAMESPACE_GUARD
namespace CVMFS_NAMESPACE_GUARD {
#endif

namespace options {

void Init();
void Fini();

void ParsePath(const std::string &config_file);
void ParseDefault(const std::string &repository_name);
void ClearConfig();
bool GetValue(const std::string &key, std::string *value);
bool GetSource(const std::string &key, std::string *value);
bool IsOn(const std::string &param_value);
std::vector<std::string> GetAllKeys();
std::string Dump();

bool ParseUIntMap(const std::string &path, std::map<uint64_t, uint64_t> *map);

}  // namespace options

#ifdef CVMFS_NAMESPACE_GUARD
}
#endif

#endif  // CVMFS_OPTIONS_H_
