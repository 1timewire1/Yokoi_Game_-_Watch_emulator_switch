#include <jni.h>

#include "yokoi_controller_state.h"

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetControllerMask(JNIEnv*, jclass, jint mask) {
    yokoi_controller_set_mask((uint32_t)mask);
}
