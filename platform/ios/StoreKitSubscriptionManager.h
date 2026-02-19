#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * StoreKit subscription manager.
 * Product: auto-renewable, 7-day trial, $4.99 US.
 * Receipt validation: use SKReceiptRefreshRequest + send receipt to server.
 */
@interface StoreKitSubscriptionManager : NSObject

@property (nonatomic, copy, nullable) void (^onStatusChanged)(BOOL active);

- (BOOL)isSubscriptionActive;
- (void)purchase;
- (void)restore;

@end

NS_ASSUME_NONNULL_END
