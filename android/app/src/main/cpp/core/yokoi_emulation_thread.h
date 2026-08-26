#pragma once

// Emulation thread lifetime + wakeups (Android-native only).

void notify_emu_thread();
void ensure_emu_thread_started();
void stop_emu_thread();
