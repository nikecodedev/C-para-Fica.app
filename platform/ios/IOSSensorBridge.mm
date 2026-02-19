#import "IOSSensorBridge.h"
#import <CoreLocation/CoreLocation.h>
#import <CoreMotion/CoreMotion.h>
#import "SensorDataSink.hpp"

@interface IOSSensorBridge () <CLLocationManagerDelegate>
@property (nonatomic, assign) void *sensorSink;
@property (nonatomic, strong) CLLocationManager *locationManager;
@property (nonatomic, strong) CMMotionManager *motionManager;
@property (nonatomic, assign) double lastAccel[3];
@property (nonatomic, assign) double lastGyro[3];
@property (nonatomic, assign) BOOL hasAccel;
@property (nonatomic, assign) BOOL hasGyro;
@property (nonatomic, assign) NSTimeInterval lastImuTime;
@end

@implementation IOSSensorBridge

- (instancetype)init {
    self = [super init];
    if (self) {
        _locationManager = [[CLLocationManager alloc] init];
        _locationManager.delegate = self;
        _locationManager.desiredAccuracy = kCLLocationAccuracyBest;
        _locationManager.distanceFilter = kCLDistanceFilterNone;

        _motionManager = [[CMMotionManager alloc] init];
        _motionManager.accelerometerUpdateInterval = 0.01;  // 100 Hz
        _motionManager.gyroUpdateInterval = 0.01;
        _motionManager.magnetometerUpdateInterval = 0.02;
    }
    return self;
}

- (void)setSensorSink:(void *)sink {
    _sensorSink = sink;
}

- (void)start {
    [_locationManager startUpdatingLocation];

    _hasAccel = NO;
    _hasGyro = NO;

    if (_motionManager.accelerometerAvailable) {
        [_motionManager startAccelerometerUpdatesToQueue:[NSOperationQueue mainQueue]
                                            withHandler:^(CMAccelerometerData * _Nullable data, NSError * _Nullable error) {
            if (data && self->_sensorSink) {
                self->_lastAccel[0] = data.acceleration.x * 9.81;
                self->_lastAccel[1] = data.acceleration.y * 9.81;
                self->_lastAccel[2] = data.acceleration.z * 9.81;
                self->_hasAccel = YES;
                self->_lastImuTime = data.timestamp;
                [self pushImuToSink];
            }
        }];
    }

    if (_motionManager.gyroAvailable) {
        [_motionManager startGyroUpdatesToQueue:[NSOperationQueue mainQueue]
                                    withHandler:^(CMGyroData * _Nullable data, NSError * _Nullable error) {
            if (data && self->_sensorSink) {
                self->_lastGyro[0] = data.rotationRate.x;
                self->_lastGyro[1] = data.rotationRate.y;
                self->_lastGyro[2] = data.rotationRate.z;
                self->_hasGyro = YES;
                self->_lastImuTime = data.timestamp;
                [self pushImuToSink];
            }
        }];
    }

    if (_motionManager.magnetometerAvailable) {
        [_motionManager startMagnetometerUpdatesToQueue:[NSOperationQueue mainQueue]
                                             withHandler:^(CMMagnetometerData * _Nullable data, NSError * _Nullable error) {
            if (data && self->_sensorSink) {
                CMMagneticField m = data.magneticField;
                auto *sink = static_cast<platform::ios::SensorDataSink *>(self->_sensorSink);
                sink->pushMag(data.timestamp, m.x, m.y, m.z);
            }
        }];
    }
}

- (void)stop {
    [_locationManager stopUpdatingLocation];
    [_motionManager stopAccelerometerUpdates];
    [_motionManager stopGyroUpdates];
    [_motionManager stopMagnetometerUpdates];
}

- (void)pushImuToSink {
    if (!_sensorSink || !_hasAccel) return;
    auto *sink = static_cast<platform::ios::SensorDataSink *>(_sensorSink);
    double gx = _hasGyro ? _lastGyro[0] : 0, gy = _hasGyro ? _lastGyro[1] : 0, gz = _hasGyro ? _lastGyro[2] : 0;
    sink->pushImu(_lastImuTime, _lastAccel[0], _lastAccel[1], _lastAccel[2], gx, gy, gz);
}

#pragma mark - CLLocationManagerDelegate

- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray<CLLocation *> *)locations {
    CLLocation *loc = locations.lastObject;
    if (!loc || !_sensorSink) return;

    auto *sink = static_cast<platform::ios::SensorDataSink *>(_sensorSink);
    NSTimeInterval t = loc.timestamp.timeIntervalSince1970;
    sink->pushGps(t, loc.coordinate.latitude, loc.coordinate.longitude,
                 loc.altitude, loc.horizontalAccuracy);
}

@end
