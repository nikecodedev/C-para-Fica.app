#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Basic track screen — first build.
 * Displays: speed, distance, trajectory line.
 * Wires 100Hz tick to integration. No tuning.
 */
@interface TrackViewController : UIViewController

@property (nonatomic, assign) void* integration;

@end

NS_ASSUME_NONNULL_END
