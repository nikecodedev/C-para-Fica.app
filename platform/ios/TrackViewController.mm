#import "TrackViewController.h"
#include "../../app/ui/FirstBuildIntegration.hpp"

@interface TrackViewController ()
@property (nonatomic, weak) UILabel* speedLabel;
@property (nonatomic, weak) UILabel* distanceLabel;
@property (nonatomic, weak) UIView* trajectoryView;
@property (nonatomic, strong) CADisplayLink* displayLink;
@end

@implementation TrackViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor colorWithWhite:0.95 alpha:1];

    UILabel* speedTitle = [[UILabel alloc] init];
    speedTitle.text = @"SPEED";
    speedTitle.font = [UIFont systemFontOfSize:12 weight:UIFontWeightMedium];
    [self.view addSubview:speedTitle];

    UILabel* speedVal = [[UILabel alloc] init];
    speedVal.font = [UIFont systemFontOfSize:32 weight:UIFontWeightSemibold];
    speedVal.text = @"0 km/h";
    [self.view addSubview:speedVal];
    _speedLabel = speedVal;

    UILabel* distTitle = [[UILabel alloc] init];
    distTitle.text = @"DISTANCE";
    distTitle.font = [UIFont systemFontOfSize:12 weight:UIFontWeightMedium];
    [self.view addSubview:distTitle];

    UILabel* distVal = [[UILabel alloc] init];
    distVal.font = [UIFont systemFontOfSize:24 weight:UIFontWeightRegular];
    distVal.text = @"0 km";
    [self.view addSubview:distVal];
    _distanceLabel = distVal;

    UIView* traj = [[UIView alloc] init];
    traj.backgroundColor = [UIColor colorWithWhite:0.9 alpha:1];
    traj.layer.borderColor = [UIColor lightGrayColor].CGColor;
    traj.layer.borderWidth = 1;
    [self.view addSubview:traj];
    _trajectoryView = traj;

    speedTitle.translatesAutoresizingMaskIntoConstraints = NO;
    speedVal.translatesAutoresizingMaskIntoConstraints = NO;
    distTitle.translatesAutoresizingMaskIntoConstraints = NO;
    distVal.translatesAutoresizingMaskIntoConstraints = NO;
    traj.translatesAutoresizingMaskIntoConstraints = NO;

    [NSLayoutConstraint activateConstraints:@[
        [speedTitle.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:20],
        [speedTitle.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:20],
        [speedVal.topAnchor constraintEqualToAnchor:speedTitle.bottomAnchor constant:4],
        [speedVal.leadingAnchor constraintEqualToAnchor:speedTitle.leadingAnchor],
        [distTitle.topAnchor constraintEqualToAnchor:speedVal.bottomAnchor constant:24],
        [distTitle.leadingAnchor constraintEqualToAnchor:speedTitle.leadingAnchor],
        [distVal.topAnchor constraintEqualToAnchor:distTitle.bottomAnchor constant:4],
        [distVal.leadingAnchor constraintEqualToAnchor:distTitle.leadingAnchor],
        [traj.topAnchor constraintEqualToAnchor:distVal.bottomAnchor constant:24],
        [traj.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:20],
        [traj.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-20],
        [traj.heightAnchor constraintEqualToConstant:200]
    ]];
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(tick)];
    _displayLink.preferredFrameRateRange = CAFrameRateRangeMake(100, 100, 100);
    [_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
}

- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    [_displayLink invalidate];
}

- (void)tick {
    auto* i = (app::ui::FirstBuildIntegration*)_integration;
    if (!i) return;
    double t = CACurrentMediaTime();
    i->tick(t);

    _speedLabel.text = [NSString stringWithFormat:@"%.1f %s", i->getSpeedDisplay(), i->getSpeedUnit()];
    _distanceLabel.text = [NSString stringWithFormat:@"%.2f %s", i->getDistanceDisplay(), i->getDistanceUnit()];
    [_trajectoryView setNeedsDisplay];
}

@end
