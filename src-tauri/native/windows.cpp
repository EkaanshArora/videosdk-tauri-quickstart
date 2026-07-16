#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "zoom_bridge.h"
#include "zoom_video_sdk_api.h"
#include "zoom_video_sdk_interface.h"
#include "zoom_video_sdk_session_info_interface.h"
#include "helpers/zoom_video_sdk_audio_helper_interface.h"
#include "helpers/zoom_video_sdk_user_helper_interface.h"
#include "helpers/zoom_video_sdk_video_helper_interface.h"
#include "windows_delegate_adapter.h"

USING_ZOOM_VIDEO_SDK_NAMESPACE

namespace {
IZoomVideoSDK* sdk = nullptr;
HWND parent_window = nullptr;
ZoomEventCallback event_callback = nullptr;
std::map<std::wstring, HWND> video_windows;
int session_status = 0;
std::string last_error;

void notify() {
  if (event_callback) event_callback();
}

void fail(const std::string& message) {
  last_error = message;
  session_status = 3;
  notify();
}

void remember_error(const std::string& message) {
  last_error = message;
}

std::wstring wide(const char* text) {
  if (!text || !*text) return {};
  int size = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
  std::wstring result(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text, -1, result.data(), size);
  result.resize(size - 1);
  return result;
}

std::string utf8(const wchar_t* text) {
  if (!text || !*text) return {};
  int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
  std::string result(size, '\0');
  WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), size, nullptr, nullptr);
  result.resize(size - 1);
  return result;
}

void copy_text(char* output, size_t size, const wchar_t* value) {
  std::string text = utf8(value);
  std::strncpy(output, text.c_str(), size - 1);
  output[size - 1] = '\0';
}

std::vector<IZoomVideoSDKUser*> users() {
  std::vector<IZoomVideoSDKUser*> result;
  IZoomVideoSDKSession* session = sdk ? sdk->getSessionInfo() : nullptr;
  if (!session) return result;
  if (session->getMyself()) result.push_back(session->getMyself());
  auto* remote = session->getRemoteUsers();
  if (remote) {
    for (int i = 0; i < remote->GetCount(); ++i) result.push_back(remote->GetItem(i));
  }
  return result;
}

IZoomVideoSDKUser* find_user(const std::wstring& id) {
  for (auto* user : users()) {
    if (user && id == user->getUserID()) return user;
  }
  return nullptr;
}

void remove_all_views() {
  for (auto& [id, window] : video_windows) {
    if (auto* user = find_user(id)) user->GetVideoCanvas()->unSubscribeWithView(window);
    DestroyWindow(window);
  }
  video_windows.clear();
}

class QuickstartDelegate : public ZoomDelegateAdapter {
 public:
  void onSessionJoin() override {
    session_status = 2;
    last_error.clear();
    notify();
  }
  void onSessionLeave() override { left(); }
  void onSessionLeave(ZoomVideoSDKSessionLeaveReason) override { left(); }
  void onError(ZoomVideoSDKErrors error, int detail) override {
    fail("Zoom error " + std::to_string(error) + " (detail " + std::to_string(detail) + ")");
  }
  void onUserJoin(IZoomVideoSDKUserHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override { notify(); }
  void onUserLeave(IZoomVideoSDKUserHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override { notify(); }
  void onUserVideoStatusChanged(IZoomVideoSDKVideoHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override { notify(); }
  void onUserAudioStatusChanged(IZoomVideoSDKAudioHelper*, IVideoSDKVector<IZoomVideoSDKUser*>*) override { notify(); }
  void onVideoCanvasSubscribeFail(ZoomVideoSDKSubscribeFailReason reason, IZoomVideoSDKUser*, void*) override {
    remember_error("Could not display video (reason " + std::to_string(reason) + ")");
  }

 private:
  void left() {
    remove_all_views();
    session_status = 0;
    notify();
  }
};

QuickstartDelegate* delegate = nullptr;
}

extern "C" int zoom_initialize(void* parent, ZoomEventCallback callback) {
  if (sdk) return 0;
  parent_window = static_cast<HWND>(parent);
  event_callback = callback;
  sdk = CreateZoomVideoSDKObj();
  delegate = new QuickstartDelegate();

  ZoomVideoSDKInitParams params;
  params.domain = L"https://zoom.us";
  params.enableLog = true;
  ZoomVideoSDKErrors error = sdk->initialize(params);
  if (error != ZoomVideoSDKErrors_Success) {
    fail("SDK initialization failed (" + std::to_string(error) + ")");
    return 1;
  }
  sdk->addListener(delegate);
  return 0;
}

extern "C" int zoom_join(const char* session_name, const char* user_name,
                         const char* password, const char* token) {
  if (!sdk) { fail("SDK is not initialized"); return 1; }
  std::wstring session = wide(session_name);
  std::wstring name = wide(user_name);
  std::wstring pass = wide(password);
  std::wstring jwt = wide(token);

  ZoomVideoSDKSessionContext context;
  context.sessionName = session.c_str();
  context.userName = name.c_str();
  context.sessionPassword = pass.c_str();
  context.token = jwt.c_str();
  context.audioOption.connect = true;
  context.audioOption.mute = false;
  context.videoOption.localVideoOn = true;

  session_status = 1;
  last_error.clear();
  if (!sdk->joinSession(context)) {
    fail("Zoom rejected the join request");
    return 1;
  }
  return 0;
}

extern "C" int zoom_leave() {
  auto error = sdk->leaveSession(false);
  if (error != ZoomVideoSDKErrors_Success) { remember_error("Could not leave session"); return 1; }
  return 0;
}

extern "C" int zoom_toggle_audio() {
  auto* session = sdk->getSessionInfo();
  auto* me = session ? session->getMyself() : nullptr;
  if (!me) { remember_error("Not in a session"); return 1; }
  ZoomVideoSDKErrors error;
  if (me->getAudioStatus().audioType == ZoomVideoSDKAudioType_None) {
    error = sdk->getAudioHelper()->startAudio();
  } else if (me->getAudioStatus().isMuted) {
    error = sdk->getAudioHelper()->unMuteAudio(me);
  } else {
    error = sdk->getAudioHelper()->muteAudio(me);
  }
  if (error != ZoomVideoSDKErrors_Success) { remember_error("Could not change audio"); return 1; }
  return 0;
}

extern "C" int zoom_toggle_video() {
  auto* session = sdk->getSessionInfo();
  auto* me = session ? session->getMyself() : nullptr;
  if (!me) { remember_error("Not in a session"); return 1; }
  auto error = me->GetVideoPipe()->getVideoStatus().isOn
    ? sdk->getVideoHelper()->stopVideo()
    : sdk->getVideoHelper()->startVideo();
  if (error != ZoomVideoSDKErrors_Success) { remember_error("Could not change video"); return 1; }
  return 0;
}

extern "C" int zoom_status() { return session_status; }

extern "C" size_t zoom_get_participants(ZoomParticipant* output, size_t capacity) {
  auto list = users();
  if (!output) return list.size();
  auto* session = sdk->getSessionInfo();
  auto* me = session ? session->getMyself() : nullptr;
  size_t count = std::min(capacity, list.size());
  for (size_t i = 0; i < count; ++i) {
    auto* user = list[i];
    copy_text(output[i].id, sizeof(output[i].id), user->getUserID());
    copy_text(output[i].name, sizeof(output[i].name), user->getUserName());
    output[i].is_local = user == me;
    output[i].audio_muted = user->getAudioStatus().audioType == ZoomVideoSDKAudioType_None || user->getAudioStatus().isMuted;
    output[i].video_on = user->GetVideoPipe()->getVideoStatus().isOn;
  }
  return count;
}

extern "C" int zoom_sync_views(const ZoomView* views, size_t count) {
  std::set<std::wstring> visible;
  for (size_t i = 0; i < count; ++i) {
    std::wstring id = wide(views[i].user_id);
    auto* user = find_user(id);
    if (!user) continue;
    visible.insert(id);

    HWND window = video_windows[id];
    if (!window) {
      // WebView2 is composited above normal child HWNDs. An owned popup stays
      // with the Tauri window while remaining visible above the webview.
      window = CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
          L"STATIC", nullptr, WS_POPUP | WS_VISIBLE,
          0, 0, 1, 1, parent_window, nullptr, GetModuleHandle(nullptr), nullptr);
      if (!window) {
        remember_error("Could not create a native video window");
        return 1;
      }
      video_windows[id] = window;
      auto error = user->GetVideoCanvas()->subscribeWithView(
          window, ZoomVideoSDKVideoAspect_LetterBox);
      if (error != ZoomVideoSDKErrors_Success) {
        remember_error("Could not subscribe the native video canvas (" +
            std::to_string(error) + ")");
        DestroyWindow(window);
        video_windows.erase(id);
        return 1;
      }
    }

    POINT origin{0, 0};
    ClientToScreen(parent_window, &origin);
    double scale = views[i].scale;
    SetWindowPos(window, HWND_TOP,
        origin.x + static_cast<int>(views[i].x * scale),
        origin.y + static_cast<int>(views[i].y * scale),
        static_cast<int>(views[i].width * scale), static_cast<int>(views[i].height * scale),
        SWP_SHOWWINDOW | SWP_NOACTIVATE);
  }

  for (auto it = video_windows.begin(); it != video_windows.end();) {
    if (visible.find(it->first) == visible.end()) {
      if (auto* user = find_user(it->first)) user->GetVideoCanvas()->unSubscribeWithView(it->second);
      DestroyWindow(it->second);
      it = video_windows.erase(it);
    } else {
      ++it;
    }
  }
  return 0;
}

extern "C" const char* zoom_last_error() { return last_error.c_str(); }

extern "C" void zoom_cleanup() {
  if (!sdk) return;
  remove_all_views();
  sdk->removeListener(delegate);
  sdk->cleanup();
  delete delegate;
  delegate = nullptr;
  DestroyZoomVideoSDKObj();
  sdk = nullptr;
}
