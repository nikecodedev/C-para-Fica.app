/**
 * JNI for engine + FirstBuildIntegration. App gets handle, passes to SensorBridge.
 * Exposes tick at 100Hz, display getters (speed, distance) for TrackScreen.
 */
#include "EngineSensorAdapter.hpp"
#include "app/ui/FirstBuildIntegration.hpp"
#include <jni.h>

struct EngineHandle {
    app::ui::FirstBuildIntegration integration;
    platform::android::EngineSensorAdapter adapter;

    EngineHandle() : adapter(integration.getEngine()) {}
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
    getHandle(ptr)->integration.getEngine().getEngine().setOrigin(static_cast<double>(lat), static_cast<double>(lon), static_cast<double>(alt));
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeSetSubscriptionActive(JNIEnv*, jclass, jlong ptr, jboolean active) {
    getHandle(ptr)->integration.setSubscriptionActive(active == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeSetLicenseKeyActive(JNIEnv*, jclass, jlong ptr, jboolean active) {
    getHandle(ptr)->integration.setLicenseKeyActive(active == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeTick(JNIEnv* env, jclass, jlong ptr, jdouble timestamp, jfloatArray out) {
    auto* h = getHandle(ptr);
    h->integration.tick(static_cast<double>(timestamp));
    const auto& state = h->integration.getLastState();
    jfloat buf[10] = {
        static_cast<jfloat>(state.px), static_cast<jfloat>(state.py), static_cast<jfloat>(state.pz),
        static_cast<jfloat>(state.vx), static_cast<jfloat>(state.vy), static_cast<jfloat>(state.vz),
        static_cast<jfloat>(state.qw), static_cast<jfloat>(state.qx), static_cast<jfloat>(state.qy),
        static_cast<jfloat>(state.qz)
    };
    env->SetFloatArrayRegion(out, 0, 10, buf);
}

JNIEXPORT jdouble JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeGetSpeedDisplay(JNIEnv*, jclass, jlong ptr) {
    return static_cast<jdouble>(getHandle(ptr)->integration.getSpeedDisplay());
}

JNIEXPORT jdouble JNICALL
Java_com_ficamotor_platform_EngineBridge_nativeGetDistanceDisplay(JNIEnv*, jclass, jlong ptr) {
    return static_cast<jdouble>(getHandle(ptr)->integration.getDistanceDisplay());
}
