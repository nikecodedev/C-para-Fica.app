/**
 * JNI for engine + sensor bridge. App gets handle, passes to SensorBridge.
 */
#include "EngineSensorAdapter.hpp"
#include "../../engine/core/EngineAccessManager.hpp"
#include <jni.h>

struct EngineHandle {
    engine::core::EngineAccessManager engine;
    platform::android::EngineSensorAdapter adapter;

    EngineHandle() : adapter(engine) {}
};

static EngineHandle* getHandle(jlong ptr) {
    return reinterpret_cast<EngineHandle*>(static_cast<intptr_t>(ptr));
}

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeCreate(JNIEnv*, jclass) {
    auto* h = new EngineHandle();
    return static_cast<jlong>(reinterpret_cast<intptr_t>(h));
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeDestroy(JNIEnv*, jclass, jlong ptr) {
    delete getHandle(ptr);
}

JNIEXPORT jlong JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeGetSensorSinkPtr(JNIEnv*, jclass, jlong ptr) {
    return static_cast<jlong>(reinterpret_cast<intptr_t>(&getHandle(ptr)->adapter));
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeSetOrigin(JNIEnv*, jclass, jlong ptr, jdouble lat, jdouble lon, jdouble alt) {
    getHandle(ptr)->engine.getEngine().setOrigin(static_cast<double>(lat), static_cast<double>(lon), static_cast<double>(alt));
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeSetSubscriptionActive(JNIEnv*, jclass, jlong ptr, jboolean active) {
    getHandle(ptr)->engine.setSubscriptionActive(active == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeTick(JNIEnv* env, jclass, jlong ptr, jdouble timestamp, jfloatArray out) {
    auto state = getHandle(ptr)->engine.tick(static_cast<double>(timestamp));
    jfloat buf[10] = {
        static_cast<jfloat>(state.px), static_cast<jfloat>(state.py), static_cast<jfloat>(state.pz),
        static_cast<jfloat>(state.vx), static_cast<jfloat>(state.vy), static_cast<jfloat>(state.vz),
        static_cast<jfloat>(state.qw), static_cast<jfloat>(state.qx), static_cast<jfloat>(state.qy),
        static_cast<jfloat>(state.qz)
    };
    env->SetFloatArrayRegion(out, 0, 10, buf);
}
