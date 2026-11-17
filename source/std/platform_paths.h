#pragma once

#include <string>

// Storage root for settings/savestates.
//
// - On 3DS: defaults to "sdmc:/3ds"
// - On Android: set this at runtime via JNI to Context.getFilesDir()
// - On desktop/other: defaults to "."
void set_storage_root(const std::string& root);

// Returns the storage root directory (no trailing slash guaranteed).
const std::string& storage_root();

// Joins storage_root() and a relative path using '/'.
std::string storage_path(const char* relative);

// Optional sub-folders of the storage root for configuration files and for
// logs (Switch: "yokoi_config" / "yokoi_logs"). Unset (the default, every
// other platform) they are the storage root itself, so config_path() /
// log_path() are then exactly storage_path().
void set_config_subdir(const std::string& subdir);
void set_log_subdir(const std::string& subdir);
std::string config_path(const char* relative);
std::string log_path(const char* relative);
