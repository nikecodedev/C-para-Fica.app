#pragma once

/**
 * C++ interface for raw sensor data. Android JNI calls push data here.
 * Engine implements via adapter; no business logic in platform.
 */
namespace platform {
namespace android {

class SensorDataSink {
public:
    virtual ~SensorDataSink() = default;

    virtual void pushGps(double timestamp, double lat_deg, double lon_deg,
                         double alt_m, double accuracy_m) = 0;

    virtual void pushImu(double timestamp, double ax, double ay, double az,
                         double gx, double gy, double gz) = 0;

    virtual void pushMag(double timestamp, double mx, double my, double mz) = 0;
};

}  // namespace android
}  // namespace platform
