package com.ficamotor.platform

/**
 * JNI bridge to C++ engine. Creates engine, provides sensor sink handle.
 */
class EngineBridge {

    private var handle: Long = 0

    init {
        handle = nativeCreate()
    }

    fun getSensorSinkPtr(): Long = nativeGetSensorSinkPtr(handle)

    fun setOrigin(lat: Double, lon: Double, alt: Double) {
        nativeSetOrigin(handle, lat, lon, alt)
    }

    fun setSubscriptionActive(active: Boolean) {
        nativeSetSubscriptionActive(handle, active)
    }

    /** Chave com Validade: enable full fusion for client tests (call after validating key). */
    fun setLicenseKeyActive(active: Boolean) {
        nativeSetLicenseKeyActive(handle, active)
    }

    fun tick(timestamp: Double): FloatArray {
        val out = FloatArray(10)
        nativeTick(handle, timestamp, out)
        return out
    }

    /** Display speed (km/h or mph). */
    fun getSpeedDisplay(): Double = nativeGetSpeedDisplay(handle)

    /** Display distance (km or miles). */
    fun getDistanceDisplay(): Double = nativeGetDistanceDisplay(handle)

    /** Reset trajectory and accumulated distance (new ride). */
    fun resetSession() = nativeResetSession(handle)

    fun dispose() {
        if (handle != 0L) {
            nativeDestroy(handle)
            handle = 0
        }
    }

    private external fun nativeCreate(): Long
    private external fun nativeDestroy(ptr: Long)
    private external fun nativeGetSensorSinkPtr(ptr: Long): Long
    private external fun nativeSetOrigin(ptr: Long, lat: Double, lon: Double, alt: Double)
    private external fun nativeSetSubscriptionActive(ptr: Long, active: Boolean)
    private external fun nativeSetLicenseKeyActive(ptr: Long, active: Boolean)
    private external fun nativeTick(ptr: Long, timestamp: Double, out: FloatArray)
    private external fun nativeGetSpeedDisplay(ptr: Long): Double
    private external fun nativeGetDistanceDisplay(ptr: Long): Double
    private external fun nativeResetSession(ptr: Long)

    companion object {
        init { System.loadLibrary("fica_native") }
    }
}
