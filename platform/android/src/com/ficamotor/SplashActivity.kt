package com.ficamotor

import android.annotation.SuppressLint
import android.content.Intent
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import androidx.appcompat.app.AppCompatActivity

@SuppressLint("CustomSplashScreen")
open class SplashActivity : AppCompatActivity() {

    protected open fun nextActivityClass(): Class<out android.app.Activity>? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_splash)
        Handler(Looper.getMainLooper()).postDelayed({ finishSplash() }, 800)
    }

    private fun finishSplash() {
        nextActivityClass()?.let { startActivity(Intent(this, it)) }
        finish()
    }
}
