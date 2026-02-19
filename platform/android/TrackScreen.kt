package com.ficamotor

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.View
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

/**
 * Basic track screen — first build.
 * Displays: speed, distance, trajectory line.
 * Wires 100Hz tick to integration. No tuning.
 *
 * Requires: integration set from native, or create in Activity.
 */
class TrackScreen : AppCompatActivity() {

    private var speedLabel: TextView? = null
    private var distanceLabel: TextView? = null
    private var tickHandler: Handler? = null
    private var tickRunnable: Runnable? = null

    var integrationPtr: Long = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.track_screen)

        speedLabel = findViewById(R.id.speed_value)
        distanceLabel = findViewById(R.id.distance_value)
    }

    override fun onResume() {
        super.onResume()
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
    }

    private fun tick() {
        if (integrationPtr == 0L) return
        // Call native tick; update labels from native getters
        speedLabel?.text = "—"
        distanceLabel?.text = "—"
    }
}
