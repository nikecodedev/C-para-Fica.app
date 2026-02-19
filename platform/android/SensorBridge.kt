package com.ficamotor.platform

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.location.Location
import com.google.android.gms.location.FusedLocationProviderClient
import com.google.android.gms.location.LocationCallback
import com.google.android.gms.location.LocationRequest
import com.google.android.gms.location.LocationResult
import com.google.android.gms.location.LocationServices
import com.google.android.gms.location.Priority

/**
 * Android sensor bridge. FusedLocationProvider + SensorManager.
 * Raw data only. Passes to C++ via JNI. No business logic.
 */
class SensorBridge(private val context: Context) {

    private var fusedClient: FusedLocationProviderClient =
        LocationServices.getFusedLocationProviderClient(context)
    private val sensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager

    private val locationCallback = object : LocationCallback() {
        override fun onLocationResult(result: LocationResult) {
            result.lastLocation?.let { loc ->
                nativePushGps(
                    loc.time / 1000.0,
                    loc.latitude,
                    loc.longitude,
                    loc.altitude,
                    loc.accuracy.toDouble()
                )
            }
        }
    }

    private val imuListener = object : SensorEventListener {
        override fun onSensorChanged(event: SensorEvent) {
            when (event.sensor.type) {
                Sensor.TYPE_ACCELEROMETER -> {
                    accelData = event.values.copyOf()
                    accelTimestamp = event.timestamp / 1e9
                    pushImuIfReady()
                }
                Sensor.TYPE_GYROSCOPE -> {
                    gyroData = event.values.copyOf()
                    gyroTimestamp = event.timestamp / 1e9
                    pushImuIfReady()
                }
            }
        }
        override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}
    }

    private val magListener = object : SensorEventListener {
        override fun onSensorChanged(event: SensorEvent) {
            if (event.sensor.type == Sensor.TYPE_MAGNETIC_FIELD) {
                nativePushMag(event.timestamp / 1e9, event.values[0], event.values[1], event.values[2])
            }
        }
        override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}
    }

    private var accelData: FloatArray? = null
    private var gyroData: FloatArray? = null
    private var accelTimestamp = 0.0
    private var gyroTimestamp = 0.0

    private fun pushImuIfReady() {
        val a = accelData ?: return
        val g = gyroData ?: return
        if (a.size < 3 || g.size < 3) return
        val t = (accelTimestamp + gyroTimestamp) / 2
        val ax = a[0]
        val ay = a[1]
        val az = a[2]
        nativePushImu(t, ax, ay, az, g[0], g[1], g[2])
    }

    fun setSink(ptr: Long) {
        nativeSetSink(ptr)
    }

    fun start() {
        val request = LocationRequest.Builder(Priority.PRIORITY_HIGH_ACCURACY, 100)
            .setMinUpdateIntervalMillis(10)
            .setMaxUpdates(Int.MAX_VALUE)
            .build()
        fusedClient.requestLocationUpdates(request, locationCallback, null)

        sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)?.let {
            sensorManager.registerListener(imuListener, it, SensorManager.SENSOR_DELAY_FASTEST)
        }
        sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE)?.let {
            sensorManager.registerListener(imuListener, it, SensorManager.SENSOR_DELAY_FASTEST)
        }
        sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD)?.let {
            sensorManager.registerListener(magListener, it, SensorManager.SENSOR_DELAY_FASTEST)
        }
    }

    fun stop() {
        fusedClient.removeLocationUpdates(locationCallback)
        sensorManager.unregisterListener(imuListener)
        sensorManager.unregisterListener(magListener)
    }

    private external fun nativeSetSink(ptr: Long)
    private external fun nativePushGps(timestamp: Double, lat: Double, lon: Double, alt: Double, accuracy: Double)
    private external fun nativePushImu(timestamp: Double, ax: Float, ay: Float, az: Float, gx: Float, gy: Float, gz: Float)
    private external fun nativePushMag(timestamp: Double, mx: Float, my: Float, mz: Float)

    companion object {
        init { System.loadLibrary("fica_native") }
    }
}
