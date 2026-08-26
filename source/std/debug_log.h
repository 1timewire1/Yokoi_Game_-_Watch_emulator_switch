#pragma once

#include <cstdarg>

// Master logging switch.
// Set to 1 to enable SD-card logging (3DS) / file logging (other platforms).
// Default is OFF.
#ifndef ENABLE_LOGGING
#define ENABLE_LOGGING 0
#endif

#if ENABLE_LOGGING
#define YOKOI_LOG(...) ::debug_log::write(__VA_ARGS__)
#else
// When disabled, arguments are not evaluated.
#define YOKOI_LOG(...) ((void)0)
#endif

namespace debug_log {

// Opens the log file (append). Safe to call multiple times.
void init();

// Runtime on/off switch, independent of the ENABLE_LOGGING compile-time gate
// above (which only controls whether this plumbing exists at all). Defaults
// to enabled, so platforms that never call this behave exactly as before.
// Disabling closes an already-open file; re-enabling lazily reopens it (with
// a fresh banner) on the next write. Intended for a platform-side runtime
// setting (e.g. Switch's "Log File" toggle) to gate on top of.
void set_enabled(bool enabled);

// Writes a formatted line to the log (best-effort).
void write(const char* fmt, ...);

// Writes a formatted line (va_list overload).
void vwrite(const char* fmt, va_list args);

// Flushes the log (best-effort).
void flush();

// Closes the log file (best-effort).
void close();

} // namespace debug_log
