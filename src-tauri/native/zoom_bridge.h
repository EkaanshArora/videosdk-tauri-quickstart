#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ZoomEventCallback)(void);

typedef struct ZoomParticipant {
  char id[128];
  char name[256];
  uint8_t is_local;
  uint8_t audio_muted;
  uint8_t video_on;
} ZoomParticipant;

typedef struct ZoomView {
  const char* user_id;
  double x;
  double y;
  double width;
  double height;
  double scale;
} ZoomView;

// Every function returns 0 on success. Details are available through
// zoom_last_error(), which keeps Rust and the platform code pleasantly small.
int zoom_initialize(void* parent_view, ZoomEventCallback callback);
int zoom_join(const char* session_name, const char* user_name,
              const char* password, const char* token);
int zoom_leave(void);
int zoom_toggle_audio(void);
int zoom_toggle_video(void);
int zoom_status(void); // 0 ready, 1 joining, 2 joined, 3 error
size_t zoom_get_participants(ZoomParticipant* output, size_t capacity);
int zoom_sync_views(const ZoomView* views, size_t count);
const char* zoom_last_error(void);
void zoom_cleanup(void);

#ifdef __cplusplus
}
#endif
