package com.ficamotor

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.util.AttributeSet
import android.view.View

/**
 * Draws the EKF trajectory path — "bisturi" precision vs Strava's jagged trace.
 * Receives (px, py) in ENU meters; auto-scales and centers.
 */
class TrajectoryView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : View(context, attrs, defStyleAttr) {

    private val pathPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.STROKE
        strokeWidth = 6f
        color = 0xFF00BFA5.toInt()  // Teal — precision, distinct from Strava orange
        strokeCap = Paint.Cap.ROUND
        strokeJoin = Paint.Join.ROUND
    }

    private val path = Path()
    private var points = mutableListOf<Pair<Float, Float>>()
    private var scale = 1f
    private var offsetX = 0f
    private var offsetY = 0f

    /** Add point (px, py) in meters. Returns true if point was added. */
    fun addPoint(px: Float, py: Float): Boolean {
        if (points.isEmpty() || points.last().let { (x, y) ->
            kotlin.math.hypot(px - x, py - y) > 0.1f
        }) {
            points.add(px to py)
            if (points.size > MAX_POINTS) points.removeAt(0)
            return true
        }
        return false
    }

    /** Replace all points. */
    fun setPoints(newPoints: List<Pair<Float, Float>>) {
        points.clear()
        points.addAll(newPoints.takeLast(MAX_POINTS))
    }

    fun clear() {
        points.clear()
        path.reset()
        invalidate()
    }

    private val startMarkerPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.FILL
        color = 0xFF00BFA5.toInt()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        val w = width.toFloat()
        val h = height.toFloat()
        if (w <= 0 || h <= 0 || points.isEmpty()) return

        when (points.size) {
            1 -> {
                val cx = w / 2
                val cy = h / 2
                canvas.drawCircle(cx, cy, 12f, startMarkerPaint)
                return
            }
        }

        val xs = points.map { it.first }
        val ys = points.map { it.second }
        val minX = xs.minOrNull() ?: 0f
        val maxX = xs.maxOrNull() ?: 0f
        val minY = ys.minOrNull() ?: 0f
        val maxY = ys.maxOrNull() ?: 0f
        val rangeX = (maxX - minX).coerceAtLeast(1f)
        val rangeY = (maxY - minY).coerceAtLeast(1f)
        val padding = 24f
        val availableW = w - 2 * padding
        val availableH = h - 2 * padding
        scale = minOf(availableW / rangeX, availableH / rangeY).coerceAtLeast(0.1f)
        offsetX = padding - minX * scale + (availableW - rangeX * scale) / 2
        offsetY = h - padding - minY * scale - (availableH - rangeY * scale) / 2

        path.reset()
        val (fx, fy) = points.first()
        path.moveTo(fx * scale + offsetX, fy * scale + offsetY)
        for (i in 1 until points.size) {
            val (px, py) = points[i]
            path.lineTo(px * scale + offsetX, py * scale + offsetY)
        }
        canvas.drawPath(path, pathPaint)
        canvas.drawCircle(fx * scale + offsetX, fy * scale + offsetY, 8f, startMarkerPaint)
    }

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        invalidate()
    }

    companion object {
        private const val MAX_POINTS = 10000
    }
}
