package com.ficamotor

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import android.widget.FrameLayout
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.google.android.gms.maps.CameraUpdateFactory
import com.google.android.gms.maps.GoogleMap
import com.google.android.gms.maps.MapView
import com.google.android.gms.maps.model.LatLng
import com.google.android.gms.maps.model.Polyline
import com.google.android.gms.maps.model.PolylineOptions
import com.ficamotor.BuildConfig
import com.ficamotor.platform.EngineBridge
import com.ficamotor.platform.PlayBillingSubscriptionManager
import com.ficamotor.platform.SensorBridge
import java.util.Locale

/**
 * Track screen — Strava-style. Speed, pace, distance, map with EKF trajectory.
 * 100Hz UI loop; native FirstBuildIntegration at 20Hz fusion (EKF).
 */
class TrackScreen : AppCompatActivity() {

    private var speedLabel: TextView? = null
    private var paceLabel: TextView? = null
    private var distanceLabel: TextView? = null
    private var mapView: MapView? = null
    private var googleMap: GoogleMap? = null
    private var polyline: Polyline? = null
    private val polylinePoints = mutableListOf<LatLng>()
    private var tickHandler: Handler? = null
    private var tickRunnable: Runnable? = null

    private var mapContainer: FrameLayout? = null
    private var engineBridge: EngineBridge? = null
    private var sensorBridge: SensorBridge? = null
    private var billingManager: PlayBillingSubscriptionManager? = null

    private var isRecording = false

    private val locationPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        if (permissions[Manifest.permission.ACCESS_FINE_LOCATION] == true) {
            startSensorsAndTick()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.track_screen)

        speedLabel = findViewById(R.id.speed_value)
        paceLabel = findViewById(R.id.pace_value)
        distanceLabel = findViewById(R.id.distance_value)
        mapContainer = findViewById(R.id.map_container)
        findViewById<android.widget.Button>(R.id.btn_start)?.setOnClickListener { onStartClick() }
        findViewById<android.widget.Button>(R.id.btn_pause)?.setOnClickListener { onPauseClick() }
        findViewById<android.widget.Button>(R.id.btn_new_ride)?.setOnClickListener { onNewRide() }

        if (hasValidMapsKey()) {
            try {
                val mv = MapView(this)
                mapContainer?.addView(mv, FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.MATCH_PARENT
                ))
                mapView = mv
                mapView?.onCreate(savedInstanceState)
                mapView?.getMapAsync { map ->
                    try {
                        googleMap = map
                        map.moveCamera(CameraUpdateFactory.newLatLngZoom(ORIGIN_SUMARE, 15f))
                        map.uiSettings.isMyLocationButtonEnabled = true
                        if (hasLocationPermission()) {
                            try { map.isMyLocationEnabled = true } catch (_: SecurityException) {}
                        }
                    } catch (e: Exception) {
                        Log.e("TrackScreen", "Map callback error", e)
                    }
                }
            } catch (e: Exception) {
                Log.e("TrackScreen", "MapView init failed", e)
                addMapPlaceholder("Map unavailable")
            }
        } else {
            addMapPlaceholder("Set MAPS_API_KEY in local.properties")
        }

        try {
            engineBridge = EngineBridge()
            sensorBridge = SensorBridge(this)
            sensorBridge?.setSink(engineBridge!!.getSensorSinkPtr())
            engineBridge?.setOrigin(ORIGIN_LAT, ORIGIN_LON, ORIGIN_ALT)
            engineBridge?.setLicenseKeyActive(true)

            try {
                billingManager = PlayBillingSubscriptionManager(this, this)
                billingManager?.setOnStatusChanged { active -> engineBridge?.setSubscriptionActive(active) }
            } catch (e: Exception) {
                Log.e("TrackScreen", "Billing init failed", e)
                engineBridge?.setSubscriptionActive(true)
            }
        } catch (e: Exception) {
            Log.e("TrackScreen", "Engine init failed", e)
            Toast.makeText(this, "Engine failed to start: ${e.message}", Toast.LENGTH_LONG).show()
            engineBridge = null
            sensorBridge = null
        }

        if (!hasLocationPermission()) {
            locationPermissionLauncher.launch(arrayOf(Manifest.permission.ACCESS_FINE_LOCATION))
        }
        updateRecordingUi()
    }

    private fun addMapPlaceholder(msg: String) {
        val tv = TextView(this)
        tv.text = msg
        tv.setPadding(32, 32, 32, 32)
        mapContainer?.addView(tv, FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT
        ))
    }

    private fun hasValidMapsKey(): Boolean {
        val key = BuildConfig.MAPS_API_KEY
        return key.isNotBlank() && key != "YOUR_GOOGLE_MAPS_API_KEY"
    }

    private fun hasLocationPermission(): Boolean =
        ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED

    private fun startSensorsAndTick() {
        if (engineBridge == null) return
        tickRunnable?.let { tickHandler?.removeCallbacks(it) }
        try {
            sensorBridge?.start()
        } catch (_: SecurityException) { /* permission revoked */ }
        tickHandler = Handler(Looper.getMainLooper())
        tickRunnable = object : Runnable {
            override fun run() {
                tick()
                tickHandler?.postDelayed(this, 10)
            }
        }
        tickHandler?.post(tickRunnable!!)
        isRecording = true
        updateRecordingUi()
    }

    private fun stopSensorsAndTick() {
        tickRunnable?.let { tickHandler?.removeCallbacks(it) }
        tickRunnable = null
        sensorBridge?.stop()
        isRecording = false
        updateRecordingUi()
    }

    /** Pause tracking when activity goes to background; preserves isRecording for onResume. */
    private fun pauseForLifecycle() {
        tickRunnable?.let { tickHandler?.removeCallbacks(it) }
        tickRunnable = null
        sensorBridge?.stop()
    }

    private fun onStartClick() {
        if (!hasLocationPermission()) {
            locationPermissionLauncher.launch(arrayOf(Manifest.permission.ACCESS_FINE_LOCATION))
            return
        }
        startSensorsAndTick()
    }

    private fun onPauseClick() {
        stopSensorsAndTick()
    }

    private fun updateRecordingUi() {
        findViewById<android.widget.Button>(R.id.btn_start)?.isEnabled = !isRecording
        findViewById<android.widget.Button>(R.id.btn_pause)?.isEnabled = isRecording
    }

    override fun onResume() {
        super.onResume()
        mapView?.onResume()
        try {
            billingManager?.restore()
        } catch (_: Exception) {}
        if (isRecording && engineBridge != null && hasLocationPermission()) {
            startSensorsAndTick()
        } else {
            updateRecordingUi()
        }
    }

    override fun onPause() {
        super.onPause()
        if (isRecording) {
            pauseForLifecycle()
        } else {
            tickRunnable?.let { tickHandler?.removeCallbacks(it) }
            tickRunnable = null
            sensorBridge?.stop()
        }
        mapView?.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView?.onDestroy()
        engineBridge?.dispose()
        engineBridge = null
        sensorBridge = null
        billingManager = null
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView?.onLowMemory()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView?.onSaveInstanceState(outState)
    }

    private fun tick() {
        val bridge = engineBridge ?: return
        val ts = SystemClock.elapsedRealtimeNanos() / 1e9
        val state = bridge.tick(ts)
        val rawSpeed = bridge.getSpeedDisplay()
        val cappedSpeed = rawSpeed.coerceIn(0.0, MAX_DISPLAY_SPEED_KMH)
        val distance = bridge.getDistanceDisplay()

        speedLabel?.text = String.format(Locale.US, "%.1f km/h", cappedSpeed)
        paceLabel?.text = if (cappedSpeed > 0.1) formatPace(cappedSpeed) else "— /km"
        distanceLabel?.text = String.format(Locale.US, "%.2f km", distance)

        val latLng = enuToLatLng(state[0], state[1])
        if (polylinePoints.isEmpty() || distMeters(polylinePoints.last(), latLng) > 0.5) {
            polylinePoints.add(latLng)
            if (polylinePoints.size > MAX_POLYLINE_POINTS) polylinePoints.removeAt(0)
            polyline?.points = ArrayList(polylinePoints)
            if (polyline == null && googleMap != null) {
                polyline = googleMap?.addPolyline(
                    PolylineOptions().addAll(polylinePoints).color(0xFF00BFA5.toInt()).width(10f)
                )
            }
        }

    }

    private fun formatPace(speedKmh: Double): String {
        if (speedKmh <= 0) return "— /km"
        val minPerKm = 60.0 / speedKmh
        val min = minPerKm.toInt()
        val sec = ((minPerKm - min) * 60).toInt()
        return String.format(Locale.US, "%d:%02d /km", min, sec)
    }

    private fun enuToLatLng(px: Float, py: Float): LatLng {
        val mPerDegLat = 111320.0
        val mPerDegLon = 111320.0 * kotlin.math.cos(Math.toRadians(ORIGIN_LAT))
        val lat = ORIGIN_LAT + py / mPerDegLat
        val lon = ORIGIN_LON + px / mPerDegLon
        return LatLng(lat, lon)
    }

    private fun distMeters(a: LatLng, b: LatLng): Double {
        val R = 6371000.0
        val dLat = Math.toRadians(b.latitude - a.latitude)
        val dLon = Math.toRadians(b.longitude - a.longitude)
        val x = kotlin.math.sin(dLat / 2) * kotlin.math.sin(dLat / 2) +
            kotlin.math.cos(Math.toRadians(a.latitude)) * kotlin.math.cos(Math.toRadians(b.latitude)) *
            kotlin.math.sin(dLon / 2) * kotlin.math.sin(dLon / 2)
        return 2 * R * kotlin.math.atan2(kotlin.math.sqrt(x), kotlin.math.sqrt(1 - x))
    }

    private fun onNewRide() {
        engineBridge?.resetSession()
        polylinePoints.clear()
        polyline?.remove()
        polyline = null
    }

    companion object {
        private const val MAX_DISPLAY_SPEED_KMH = 120.0
        private const val MAX_POLYLINE_POINTS = 10000
        private const val ORIGIN_LAT = -23.20
        private const val ORIGIN_LON = -47.14
        private const val ORIGIN_ALT = 580.0
        private val ORIGIN_SUMARE = LatLng(ORIGIN_LAT, ORIGIN_LON)
    }
}
