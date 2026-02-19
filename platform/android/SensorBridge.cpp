/**
 * Android JNI bridge. Java/Kotlin calls these natives; we push to C++ engine.
 * Link with -shared, include from Android.mk or CMake.
 */
#include "SensorDataSink.hpp"
#include <jni.h>
#include <cstdint>

static platform::android::SensorDataSink* g_sink = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_SensorBridge_nativeSetSink(JNIEnv*, jclass, jlong ptr) {
    g_sink = reinterpret_cast<platform::android::SensorDataSink*>(static_cast<intptr_t>(ptr));
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_SensorBridge_nativePushGps(JNIEnv* env, jclass clazz,
        jdouble timestamp, jdouble lat, jdouble lon, jdouble alt, jdouble accuracy) {
    if (g_sink) {
        g_sink->pushGps(static_cast<double>(timestamp),
                        static_cast<double>(lat), static_cast<double>(lon),
                        static_cast<double>(alt), static_cast<double>(accuracy));
    }
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_SensorBridge_nativePushImu(JNIEnv* env, jclass clazz,
        jdouble timestamp, jfloat ax, jfloat ay, jfloat az, jfloat gx, jfloat gy, jfloat gz) {
    if (g_sink) {
        g_sink->pushImu(static_cast<double>(timestamp),
                        static_cast<double>(ax), static_cast<double>(ay), static_cast<double>(az),
                        static_cast<double>(gx), static_cast<double>(gy), static_cast<double>(gz));
    }
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_SensorBridge_nativePushMag(JNIEnv* env, jclass clazz,
        jdouble timestamp, jfloat mx, jfloat my, jfloat mz) {
    if (g_sink) {
        g_sink->pushMag(static_cast<double>(timestamp),
                        static_cast<double>(mx), static_cast<double>(my), static_cast<double>(mz));
    }
}

}  // extern "C"
