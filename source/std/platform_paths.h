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
