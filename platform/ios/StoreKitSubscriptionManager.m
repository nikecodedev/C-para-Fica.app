#import "StoreKitSubscriptionManager.h"
#import <StoreKit/StoreKit.h>

static NSString* const kProductId = @"com.ficamotor.tracking.premium";

@interface StoreKitSubscriptionManager () <SKProductsRequestDelegate, SKPaymentTransactionObserver>
@property (nonatomic, assign) BOOL subscriptionActive;
@end

@implementation StoreKitSubscriptionManager

- (instancetype)init {
    self = [super init];
    if (self) {
        _subscriptionActive = NO;
        [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
    }
    return self;
}

- (void)dealloc {
    [[SKPaymentQueue defaultQueue] removeTransactionObserver:self];
}

- (BOOL)isSubscriptionActive {
    return _subscriptionActive;
}

- (void)purchase {
    SKProductsRequest* req = [[SKProductsRequest alloc] initWithProductIdentifiers:[NSSet setWithObject:kProductId]];
    req.delegate = self;
    [req start];
}

- (void)restore {
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

- (void)productsRequest:(SKProductsRequest*)request didReceiveResponse:(SKProductsResponse*)response {
    SKProduct* product = response.products.firstObject;
    if (product) {
        SKPayment* payment = [SKPayment paymentWithProduct:product];
        [[SKPaymentQueue defaultQueue] addPayment:payment];
    }
}

- (void)paymentQueue:(SKPaymentQueue*)queue updatedTransactions:(NSArray<SKPaymentTransaction*>*)transactions {
    for (SKPaymentTransaction* t in transactions) {
        switch (t.transactionState) {
            case SKPaymentTransactionStatePurchased:
            case SKPaymentTransactionStateRestored:
                _subscriptionActive = YES;
                if (_onStatusChanged) _onStatusChanged(YES);
                [queue finishTransaction:t];
                break;
            case SKPaymentTransactionStateFailed:
                [queue finishTransaction:t];
                break;
            default:
                break;
        }
    }
}

@end
