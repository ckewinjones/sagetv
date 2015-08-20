/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class sage_LinuxInput */

#ifndef _Included_sage_LinuxInput
#define _Included_sage_LinuxInput
#ifdef __cplusplus
extern "C" {
#endif
/* Inaccessible static: loadedLib */
/*
 * Class:     sage_LinuxInput
 * Method:    setupPVR150SInput
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_sage_PVR150Input_setupPVR150Input
  (JNIEnv *, jobject, jint);

/*
 * Class:     sage_LinuxInput
 * Method:    closePVR150SInput
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sage_PVR150Input_closePVR150Input
  (JNIEnv *, jobject);

/*
 * Class:     sage_LinuxInput
 * Method:    PVR150SInputThread
 * Signature: (Lsage/SageTVInputCallback;)V
 */
JNIEXPORT void JNICALL Java_sage_PVR150Input_PVR150InputThread
  (JNIEnv *, jobject, jint, jobject);

#ifdef __cplusplus
}
#endif
#endif
