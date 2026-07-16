#pragma once

#include "zoom_video_sdk_delegate_interface.h"

USING_ZOOM_VIDEO_SDK_NAMESPACE

// Zoom's Windows delegate has many required callbacks and no adapter class.
// Keeping the no-op methods here leaves QuickstartDelegate focused on the
// seven events this app actually uses.
class ZoomDelegateAdapter : public IZoomVideoSDKDelegate {
 public:
  void onSessionJoin() override {}
  void onSessionLeave() override {}
  void onSessionLeave(ZoomVideoSDKSessionLeaveReason) override {}
  void onError(ZoomVideoSDKErrors, int) override {}
  void onUserJoin(IZoomVideoSDKUserHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override {}
  void onUserLeave(IZoomVideoSDKUserHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override {}
  void onUserVideoStatusChanged(IZoomVideoSDKVideoHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override {}
  void onUserAudioStatusChanged(IZoomVideoSDKAudioHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override {}
  void onUserShareStatusChanged(IZoomVideoSDKShareHelper*, IZoomVideoSDKUser*, IZoomVideoSDKShareAction*) override {}
  void onShareContentChanged(IZoomVideoSDKShareHelper*, IZoomVideoSDKUser*, IZoomVideoSDKShareAction*) override {}
  void onFailedToStartShare(IZoomVideoSDKShareHelper*, IZoomVideoSDKUser*) override {}
  void onShareSettingChanged(ZoomVideoSDKShareSetting) override {}
  void onUserRecordingConsent(IZoomVideoSDKUser*) override {}
  void onLiveStreamStatusChanged(IZoomVideoSDKLiveStreamHelper*, ZoomVideoSDKLiveStreamStatus) override {}
  void onChatNewMessageNotify(IZoomVideoSDKChatHelper*, IZoomVideoSDKChatMessage*) override {}
  void onUserHostChanged(IZoomVideoSDKUserHelper*, IZoomVideoSDKUser*) override {}
  void onUserActiveAudioChanged(IZoomVideoSDKAudioHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override {}
  void onSessionNeedPassword(IZoomVideoSDKPasswordHandler*) override {}
  void onSessionPasswordWrong(IZoomVideoSDKPasswordHandler*) override {}
  void onMixedAudioRawDataReceived(AudioRawData*) override {}
  void onOneWayAudioRawDataReceived(AudioRawData*, IZoomVideoSDKUser*) override {}
  void onSharedAudioRawDataReceived(AudioRawData*) override {}
  void onUserManagerChanged(IZoomVideoSDKUser*) override {}
  void onUserNameChanged(IZoomVideoSDKUser*) override {}
  void onUserFailoverStatusChanged(IZoomVideoSDKUser*, bool) override {}
  void onCameraControlRequestResult(IZoomVideoSDKUser*, bool) override {}
  void onCameraControlRequestReceived(IZoomVideoSDKUser*, ZoomVideoSDKCameraControlRequestType, IZoomVideoSDKCameraControlRequestHandler*) override {}
  void onRemoteControlStatus(IZoomVideoSDKUser*, IZoomVideoSDKShareAction*, ZoomVideoSDKRemoteControlStatus) override {}
  void onRemoteControlRequestReceived(IZoomVideoSDKUser*, IZoomVideoSDKShareAction*, IZoomVideoSDKRemoteControlRequestHandler*) override {}
  void onRemoteControlServiceInstallResult(bool) override {}
  void onCommandReceived(IZoomVideoSDKUser*, const zchar_t*) override {}
  void onCommandChannelConnectResult(bool) override {}
  void onInviteByPhoneStatus(PhoneStatus, PhoneFailedReason) override {}
  void onCalloutJoinSuccess(IZoomVideoSDKUser*, const zchar_t*) override {}
  void onCloudRecordingStatus(RecordingStatus, IZoomVideoSDKRecordingConsentHandler*) override {}
  void onHostAskUnmute() override {}
  void onMultiCameraStreamStatusChanged(ZoomVideoSDKMultiCameraStreamStatus, IZoomVideoSDKUser*, IZoomVideoSDKRawDataPipe*) override {}
  void onMicSpeakerVolumeChanged(unsigned int, unsigned int) override {}
  void onAudioLevelChanged(unsigned int, bool, IZoomVideoSDKUser*) override {}
  void onAudioDeviceStatusChanged(ZoomVideoSDKAudioDeviceType, ZoomVideoSDKAudioDeviceStatus) override {}
  void onTestMicStatusChanged(ZoomVideoSDK_TESTMIC_STATUS) override {}
  void onSelectedAudioDeviceChanged() override {}
  void onCameraListChanged() override {}
  void onLiveTranscriptionStatus(ZoomVideoSDKLiveTranscriptionStatus) override {}
  void onOriginalLanguageMsgReceived(ILiveTranscriptionMessageInfo*) override {}
  void onLiveTranscriptionMsgInfoReceived(ILiveTranscriptionMessageInfo*) override {}
  void onLiveTranscriptionMsgError(ILiveTranscriptionLanguage*, ILiveTranscriptionLanguage*) override {}
  void onSpokenLanguageChanged(ILiveTranscriptionLanguage*) override {}
  void onVoiceInterpretationReady() override {}
  void onChatMsgDeleteNotification(IZoomVideoSDKChatHelper*, const zchar_t*, ZoomVideoSDKChatMessageDeleteType) override {}
  void onChatPrivilegeChanged(IZoomVideoSDKChatHelper*, ZoomVideoSDKChatPrivilegeType) override {}
  void onSendFileStatus(IZoomVideoSDKSendFile*, const FileTransferStatus&) override {}
  void onReceiveFileStatus(IZoomVideoSDKReceiveFile*, const FileTransferStatus&) override {}
  void onProxyDetectComplete() override {}
  void onProxySettingNotification(IZoomVideoSDKProxySettingHandler*) override {}
  void onSSLCertVerifiedFailNotification(IZoomVideoSDKSSLCertificateInfo*) override {}
  void onUserVideoNetworkStatusChanged(ZoomVideoSDKNetworkStatus, IZoomVideoSDKUser*) override {}
  void onShareNetworkStatusChanged(ZoomVideoSDKNetworkStatus, bool) override {}
  void onUserNetworkStatusChanged(ZoomVideoSDKDataType, ZoomVideoSDKNetworkStatus, IZoomVideoSDKUser*) override {}
  void onUserOverallNetworkStatusChanged(ZoomVideoSDKNetworkStatus, IZoomVideoSDKUser*) override {}
  void onCallCRCDeviceStatusChanged(ZoomVideoSDKCRCCallStatus) override {}
  void onVideoCanvasSubscribeFail(ZoomVideoSDKSubscribeFailReason, IZoomVideoSDKUser*, void*) override {}
  void onShareCanvasSubscribeFail(IZoomVideoSDKUser*, void*, IZoomVideoSDKShareAction*) override {}
  void onAnnotationHelperCleanUp(IZoomVideoSDKAnnotationHelper*) override {}
  void onAnnotationPrivilegeChange(IZoomVideoSDKUser*, IZoomVideoSDKShareAction*) override {}
  void onAnnotationHelperActived(void*) override {}
  void onAnnotationToolTypeChanged(IZoomVideoSDKAnnotationHelper*, void*, ZoomVideoSDKAnnotationToolType) override {}
  void onVideoAlphaChannelStatusChanged(bool) override {}
  void onSpotlightVideoChanged(IZoomVideoSDKVideoHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override {}
  void onBindIncomingLiveStreamResponse(bool, const zchar_t*) override {}
  void onUnbindIncomingLiveStreamResponse(bool, const zchar_t*) override {}
  void onIncomingLiveStreamStatusResponse(bool, IVideoSDKVector<IncomingLiveStreamStatus>*) override {}
  void onStartIncomingLiveStreamResponse(bool, const zchar_t*) override {}
  void onStopIncomingLiveStreamResponse(bool, const zchar_t*) override {}
  void onShareContentSizeChanged(IZoomVideoSDKShareHelper*, IZoomVideoSDKUser*, IZoomVideoSDKShareAction*) override {}
  void onUnsharingWindowsChanged(IVideoSDKVector<void*>*, IZoomVideoSDKShareHelper*, IZoomVideoSDKUser*, IZoomVideoSDKShareAction*) override {}
  void onSharingActiveMonitorChanged(IVideoSDKVector<void*>*, IZoomVideoSDKShareHelper*, IZoomVideoSDKUser*, IZoomVideoSDKShareAction*) override {}
  void onSharingProcessWindowsStateChanged(bool, IZoomVideoSDKShareHelper*, IZoomVideoSDKUser*, IZoomVideoSDKShareAction*) override {}
  void onSubSessionStatusChanged(ZoomVideoSDKSubSessionStatus, IVideoSDKVector<ISubSessionKit*>*) override {}
  void onSubSessionManagerHandle(IZoomVideoSDKSubSessionManager*) override {}
  void onSubSessionParticipantHandle(IZoomVideoSDKSubSessionParticipant*) override {}
  void onSubSessionUsersUpdate(ISubSessionKit*) override {}
  void onBroadcastMessageFromMainSession(const zchar_t*, const zchar_t*) override {}
  void onSubSessionUserHelpRequest(ISubSessionUserHelpRequestHandler*) override {}
  void onSubSessionUserHelpRequestResult(ZoomVideoSDKUserHelpRequestResult) override {}
  void onStartBroadcastResponse(bool, const zchar_t*) override {}
  void onStopBroadcastResponse(bool) override {}
  void onGetBroadcastControlStatus(bool, ZoomVideoSDKBroadcastControlStatus) override {}
  void onStreamingJoinStatusChanged(ZoomVideoSDKStreamingJoinStatus) override {}
  void onEmojiReactionReceived(IZoomVideoSDKUser*, ZoomVideoSDKEmojiReactionType) override {}
  void onWhiteboardExported(ZoomVideoSDKExportFormat, unsigned char*, long) override {}
  void onUserWhiteboardShareStatusChanged(IZoomVideoSDKUser*, IZoomVideoSDKWhiteboardHelper*) override {}
  void onRealTimeMediaStreamsStatus(RealTimeMediaStreamsStatus) override {}
  void onRealTimeMediaStreamsFail(RealTimeMediaStreamsFailReason) override {}
  void onCanvasSnapshotTaken(IZoomVideoSDKUser*, bool) override {}
  void onCanvasSnapshotIncompatible(IZoomVideoSDKUser*) override {}
  void onQOSStatisticsReceived(const ZoomVideoSDKQOSStatistics&, IZoomVideoSDKUser*) override {}
};
