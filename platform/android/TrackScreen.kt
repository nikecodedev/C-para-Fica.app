package com.ficamotor

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.ficamotor.platform.EngineBridge
import com.ficamotor.platform.PlayBillingSubscriptionManager
import com.ficamotor.platform.SensorBridge

/**
 * Track screen — first build. Displays speed, distance.
 * 100Hz UI loop; native FirstBuildIntegration at 20Hz fusion (EKF).
 * Wires EngineBridge + SensorBridge + PlayBilling. Subscription enables full fusion (20Hz EKF).
 */
class TrackScreen : AppCompatActivity() {

    private var speedLabel: TextView? = null
    private var distanceLabel: TextView? = null
    private var tickHandler: Handler? = null
    private var tickRunnable: Runnable? = null

    private var engineBridge: EngineBridge? = null
    private var sensorBridge: SensorBridge? = null
    private var billingManager: PlayBillingSubscriptionManager? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.track_screen)

        speedLabel = findViewById(R.id.speed_value)
        distanceLabel = findViewById(R.id.distance_value)

        engineBridge = EngineBridge()
        sensorBridge = SensorBridge(this)
        sensorBridge?.setSink(engineBridge!!.getSensorSinkPtr())

        engineBridge?.setOrigin(-23.20, -47.14, 580.0)

        billingManager = PlayBillingSubscriptionManager(this, this)
        billingManager?.setOnStatusChanged { active ->
            engineBridge?.setSubscriptionActive(active)
        }
        engineBridge?.setLicenseKeyActive(true)
    }

    override fun onResume() {
        super.onResume()
        billingManager?.restore()
        sensorBridge?.start()
        tickHandler = Handler(Looper.getMainLooper())
        tickRunnable = object : Runnable {
            override fun run() {
                tick()
                tickHandler?.postDelayed(this, 10)
            }
        }
        tickHandler?.post(tickRunnable!!)
    }

    override fun onPause() {
        super.onPause()
        tickRunnable?.let { tickHandler?.removeCallbacks(it) }
        sensorBridge?.stop()
    }

    override fun onDestroy() {
        super.onDestroy()
        engineBridge?.dispose()
        engineBridge = null
        sensorBridge = null
        billingManager = null
    }

    private fun tick() {
        val bridge = engineBridge ?: return
        val ts = SystemClock.elapsedRealtimeNanos() / 1e9
        bridge.tick(ts)
        val speed = bridge.getSpeedDisplay()
        val distance = bridge.getDistanceDisplay()
        speedLabel?.text = String.format("%.1f km/h", speed)
        distanceLabel?.text = String.format("%.2f km", distance)
    }
}
