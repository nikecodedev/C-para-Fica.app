#include "TrackingLogger.hpp"
#include <fstream>

namespace engine {
namespace logging {

TrackingLogger::TrackingLogger() = default;

void TrackingLogger::log(const TrackingLogEntry& entry) {
    data_[head_] = entry;
    head_ = (head_ + 1) % LOG_BUFFER_SIZE;
    if (size_ < LOG_BUFFER_SIZE) ++size_;
}

bool TrackingLogger::exportToCsv(const char* path) const {
    std::ofstream out(path);
    if (!out) return false;

    out << "timestamp"
        << ",px,py,pz,vx,vy,vz,qw,qx,qy,qz"
        << ",P0,P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15"
        << ",raw_ax,raw_ay,raw_az,raw_gx,raw_gy,raw_gz,has_raw_imu"
        << ",raw_gps_x,raw_gps_y,raw_gps_z,has_raw_gps"
        << ",raw_mx,raw_my,raw_mz,has_raw_mag"
        << "\n";

    const size_t n = size_;
    for (size_t i = 0; i < n; ++i) {
        const size_t idx = (head_ + LOG_BUFFER_SIZE - size_ + i) % LOG_BUFFER_SIZE;
        const TrackingLogEntry& e = data_[idx];

        out << e.timestamp
            << "," << e.px << "," << e.py << "," << e.pz
            << "," << e.vx << "," << e.vy << "," << e.vz
            << "," << e.qw << "," << e.qx << "," << e.qy << "," << e.qz;

        for (int j = 0; j < 16; ++j) out << "," << e.P[j];

        out << "," << e.raw_ax << "," << e.raw_ay << "," << e.raw_az
            << "," << e.raw_gx << "," << e.raw_gy << "," << e.raw_gz
            << "," << (e.has_raw_imu ? "1" : "0")
            << "," << e.raw_gps_x << "," << e.raw_gps_y << "," << e.raw_gps_z
            << "," << (e.has_raw_gps ? "1" : "0")
            << "," << e.raw_mx << "," << e.raw_my << "," << e.raw_mz
            << "," << (e.has_raw_mag ? "1" : "0")
            << "\n";
    }

    return out.good();
}

void TrackingLogger::clear() {
    head_ = 0;
    size_ = 0;
}

}  // namespace logging
}  // namespace engine
