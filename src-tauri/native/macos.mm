#import <AppKit/AppKit.h>
#import <ZMVideoSDK/ZMVideoSDK.h>

#include "zoom_bridge.h"
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

ZMVideoSDK* sdk = nil;
NSView* parent_view = nil;
ZoomEventCallback event_callback = nullptr;
NSMutableDictionary<NSString*, NSView*>* video_views = nil;
int session_status = 0;
std::string last_error;

void notify() {
  if (event_callback) event_callback();
}

void fail(NSString* message) {
  last_error = message ? message.UTF8String : "Zoom Video SDK error";
  session_status = 3;
  notify();
}

void remember_error(NSString* message) {
  last_error = message ? message.UTF8String : "Zoom Video SDK error";
}

void copy_text(char* output, size_t size, NSString* value) {
  const char* text = value ? value.UTF8String : "";
  std::strncpy(output, text, size - 1);
  output[size - 1] = '\0';
}

NSArray<ZMVideoSDKUser*>* users() {
  ZMVideoSDKSession* session = sdk.getSessionInfo;
  if (!session) return @[];
  NSMutableArray* result = [NSMutableArray array];
  if (session.getMySelf) [result addObject:session.getMySelf];
  if (session.getRemoteUsers) [result addObjectsFromArray:session.getRemoteUsers];
  return result;
}

ZMVideoSDKUser* find_user(NSString* user_id) {
  for (ZMVideoSDKUser* user in users()) {
    if ([user.getUserID isEqualToString:user_id]) return user;
  }
  return nil;
}

void remove_all_views() {
  for (NSString* user_id in video_views.allKeys) {
    NSView* view = video_views[user_id];
    ZMVideoSDKUser* user = find_user(user_id);
    if (user) [user.getVideoCanvas unSubscribeWithView:view];
    [view removeFromSuperview];
  }
  [video_views removeAllObjects];
}

@interface QuickstartDelegate : NSObject <ZMVideoSDKDelegate>
@end

@implementation QuickstartDelegate
- (void)onSessionJoin {
  session_status = 2;
  last_error.clear();
  notify();
}
- (void)onSessionLeave:(ZMVideoSDKSessionLeaveReason)reason {
  remove_all_views();
  session_status = 0;
  notify();
}
- (void)onError:(ZMVideoSDKErrors)error detail:(int)detail {
  fail([NSString stringWithFormat:@"Zoom error %ld (detail %d)", (long)error, detail]);
}
- (void)onUserJoin:(ZMVideoSDKUserHelper*)helper userList:(NSArray<ZMVideoSDKUser*>*)list { notify(); }
- (void)onUserLeave:(ZMVideoSDKUserHelper*)helper userList:(NSArray<ZMVideoSDKUser*>*)list { notify(); }
- (void)onUserVideoStatusChanged:(ZMVideoSDKVideoHelper*)helper userList:(NSArray<ZMVideoSDKUser*>*)list { notify(); }
- (void)onUserAudioStatusChanged:(ZMVideoSDKAudioHelper*)helper userList:(NSArray<ZMVideoSDKUser*>*)list { notify(); }
@end

QuickstartDelegate* delegate = nil;

extern "C" int zoom_initialize(void* parent, ZoomEventCallback callback) {
  if (sdk) return 0;

  parent_view = (__bridge NSView*)parent;
  event_callback = callback;
  video_views = [NSMutableDictionary dictionary];
  delegate = [QuickstartDelegate new];
  sdk = ZMVideoSDK.sharedVideoSDK;

  ZMVideoSDKInitParams* params = [ZMVideoSDKInitParams new];
  params.domain = @"https://zoom.us";
  params.enableLog = YES;
  ZMVideoSDKErrors error = [sdk initialize:params];
  if (error != ZMVideoSDKErrors_Success) {
    fail([NSString stringWithFormat:@"SDK initialization failed (%ld)", (long)error]);
    return 1;
  }
  [sdk addListener:delegate];
  return 0;
}

extern "C" int zoom_join(const char* session_name, const char* user_name,
                         const char* password, const char* token) {
  if (!sdk) { fail(@"SDK is not initialized"); return 1; }

  ZMVideoSDKAudioOption* audio = [ZMVideoSDKAudioOption new];
  audio.connect = YES;
  audio.mute = NO;
  ZMVideoSDKVideoOption* video = [ZMVideoSDKVideoOption new];
  video.localVideoOn = YES;

  ZMVideoSDKSessionContext* context = [ZMVideoSDKSessionContext new];
  context.sessionName = [NSString stringWithUTF8String:session_name];
  context.userName = [NSString stringWithUTF8String:user_name];
  context.sessionPassword = [NSString stringWithUTF8String:password];
  context.token = [NSString stringWithUTF8String:token];
  context.audioOption = audio;
  context.videoOption = video;

  session_status = 1;
  last_error.clear();
  if (![sdk joinSession:context]) {
    fail(@"Zoom rejected the join request");
    return 1;
  }
  return 0;
}

extern "C" int zoom_leave() {
  ZMVideoSDKErrors error = [sdk leaveSession:NO];
  if (error != ZMVideoSDKErrors_Success) {
    remember_error([NSString stringWithFormat:@"Could not leave session (%ld)", (long)error]);
    return 1;
  }
  return 0;
}

extern "C" int zoom_toggle_audio() {
  ZMVideoSDKUser* me = sdk.getSessionInfo.getMySelf;
  if (!me) { remember_error(@"Not in a session"); return 1; }
  ZMVideoSDKErrors error;
  if (me.getAudioStatus.audioType == ZMVideoSDKAudioType_None) {
    error = [sdk.getAudioHelper startAudio];
  } else if (me.getAudioStatus.isMuted) {
    error = [sdk.getAudioHelper unMuteAudio:me];
  } else {
    error = [sdk.getAudioHelper muteAudio:me];
  }
  if (error != ZMVideoSDKErrors_Success) { remember_error(@"Could not change audio"); return 1; }
  return 0;
}

extern "C" int zoom_toggle_video() {
  ZMVideoSDKUser* me = sdk.getSessionInfo.getMySelf;
  if (!me) { remember_error(@"Not in a session"); return 1; }
  ZMVideoSDKErrors error = me.getVideoPipe.getVideoStatus.isOn
    ? [sdk.getVideoHelper stopVideo]
    : [sdk.getVideoHelper startVideo];
  if (error != ZMVideoSDKErrors_Success) { remember_error(@"Could not change video"); return 1; }
  return 0;
}

extern "C" int zoom_status() { return session_status; }

extern "C" size_t zoom_get_participants(ZoomParticipant* output, size_t capacity) {
  NSArray<ZMVideoSDKUser*>* list = users();
  if (!output) return list.count;
  ZMVideoSDKUser* me = sdk.getSessionInfo.getMySelf;
  size_t count = std::min(capacity, (size_t)list.count);
  for (size_t i = 0; i < count; ++i) {
    ZMVideoSDKUser* user = list[i];
    copy_text(output[i].id, sizeof(output[i].id), user.getUserID);
    copy_text(output[i].name, sizeof(output[i].name), user.getUserName);
    output[i].is_local = user == me;
    output[i].audio_muted = !user.getAudioStatus || user.getAudioStatus.isMuted;
    output[i].video_on = user.getVideoPipe.getVideoStatus.isOn;
  }
  return count;
}

extern "C" int zoom_sync_views(const ZoomView* views, size_t count) {
  if (!parent_view) return 0;
  NSMutableSet<NSString*>* visible = [NSMutableSet set];

  for (size_t i = 0; i < count; ++i) {
    NSString* user_id = [NSString stringWithUTF8String:views[i].user_id];
    ZMVideoSDKUser* user = find_user(user_id);
    if (!user) continue;
    [visible addObject:user_id];

    NSView* view = video_views[user_id];
    if (!view) {
      view = [[NSView alloc] init];
      view.wantsLayer = YES;
      view.layer.backgroundColor = NSColor.blackColor.CGColor;
      [parent_view addSubview:view positioned:NSWindowAbove relativeTo:nil];
      video_views[user_id] = view;
      [user.getVideoCanvas subscribeWithView:view
                                  aspectMode:ZMVideoSDKVideoAspect_LetterBox
                                  resolution:ZMVideoSDKResolution_Auto];
    }

    CGFloat y = parent_view.isFlipped
      ? views[i].y
      : parent_view.bounds.size.height - views[i].y - views[i].height;
    view.frame = NSMakeRect(views[i].x, y, views[i].width, views[i].height);
  }

  for (NSString* user_id in video_views.allKeys.copy) {
    if (![visible containsObject:user_id]) {
      NSView* view = video_views[user_id];
      ZMVideoSDKUser* user = find_user(user_id);
      if (user) [user.getVideoCanvas unSubscribeWithView:view];
      [view removeFromSuperview];
      [video_views removeObjectForKey:user_id];
    }
  }
  return 0;
}

extern "C" const char* zoom_last_error() { return last_error.c_str(); }

extern "C" void zoom_cleanup() {
  if (!sdk) return;
  remove_all_views();
  [sdk removeListener:delegate];
  [sdk cleanUp];
  sdk = nil;
  delegate = nil;
}
