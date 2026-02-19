#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * iOS sensor bridge. Uses CoreLocation (GPS) and CoreMotion (IMU, magnetometer).
 * Passes raw sensor data to C++ engine. No business logic.
 */
@interface IOSSensorBridge : NSObject

/** Set C++ SensorDataSink pointer (platform::ios::SensorDataSink*). */
- (void)setSensorSink:(void *)sink;

- (void)start;
- (void)stop;

@end

NS_ASSUME_NONNULL_END
