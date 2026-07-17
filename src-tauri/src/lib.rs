use raw_window_handle::{HasWindowHandle, RawWindowHandle};
use serde::{Deserialize, Serialize};
use std::{
    ffi::{CStr, CString},
    os::raw::{c_char, c_void},
    sync::OnceLock,
};
use tauri::{Emitter, Manager, WebviewWindow};

static APP: OnceLock<tauri::AppHandle> = OnceLock::new();

#[repr(C)]
#[derive(Clone, Copy)]
struct NativeParticipant {
    id: [c_char; 128],
    name: [c_char; 256],
    is_local: u8,
    audio_muted: u8,
    video_on: u8,
}

impl Default for NativeParticipant {
    fn default() -> Self {
        Self {
            id: [0; 128],
            name: [0; 256],
            is_local: 0,
            audio_muted: 0,
            video_on: 0,
        }
    }
}

#[repr(C)]
struct NativeView {
    user_id: *const c_char,
    x: f64,
    y: f64,
    width: f64,
    height: f64,
    scale: f64,
}

unsafe extern "C" {
    fn zoom_initialize(parent_view: *mut c_void, callback: extern "C" fn()) -> i32;
    fn zoom_join(
        session_name: *const c_char,
        user_name: *const c_char,
        password: *const c_char,
        token: *const c_char,
    ) -> i32;
    fn zoom_leave() -> i32;
    fn zoom_toggle_audio() -> i32;
    fn zoom_toggle_video() -> i32;
    fn zoom_status() -> i32;
    fn zoom_get_participants(output: *mut NativeParticipant, capacity: usize) -> usize;
    fn zoom_sync_views(views: *const NativeView, count: usize) -> i32;
    fn zoom_last_error() -> *const c_char;
    fn zoom_cleanup();
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct JoinInput {
    session_name: String,
    user_name: String,
    password: String,
    token: String,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct VideoView {
    user_id: String,
    x: f64,
    y: f64,
    width: f64,
    height: f64,
    scale: f64,
}

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
struct Participant {
    id: String,
    name: String,
    is_local: bool,
    audio_muted: bool,
    video_on: bool,
}

#[derive(Serialize)]
struct SessionState {
    status: &'static str,
    participants: Vec<Participant>,
    error: Option<String>,
}

extern "C" fn on_zoom_event() {
    if let Some(app) = APP.get() {
        let _ = app.emit("zoom-state-changed", ());
    }
}

#[tauri::command]
fn initialize_sdk(window: WebviewWindow) -> Result<(), String> {
    APP.set(window.app_handle().clone()).ok();
    let handle = window.window_handle().map_err(|error| error.to_string())?;
    let parent = match handle.as_raw() {
        RawWindowHandle::AppKit(handle) => handle.ns_view.as_ptr(),
        RawWindowHandle::Win32(handle) => handle.hwnd.get() as *mut c_void,
        _ => return Err("Only macOS and Windows are supported".into()),
    };
    check(unsafe { zoom_initialize(parent, on_zoom_event) })
}

#[tauri::command]
fn join_session(input: JoinInput) -> Result<(), String> {
    let session_name = c_string(&input.session_name)?;
    let user_name = c_string(&input.user_name)?;
    let password = c_string(&input.password)?;
    let token = c_string(&input.token)?;
    check(unsafe {
        zoom_join(
            session_name.as_ptr(),
            user_name.as_ptr(),
            password.as_ptr(),
            token.as_ptr(),
        )
    })
}

#[tauri::command]
fn get_session_state() -> SessionState {
    let status = match unsafe { zoom_status() } {
        1 => "joining",
        2 => "joined",
        3 => "error",
        _ => "ready",
    };
    let count = unsafe { zoom_get_participants(std::ptr::null_mut(), 0) };
    let mut native = vec![NativeParticipant::default(); count];
    if count > 0 {
        unsafe { zoom_get_participants(native.as_mut_ptr(), native.len()) };
    }

    SessionState {
        status,
        participants: native
            .into_iter()
            .map(|user| Participant {
                id: chars_to_string(&user.id),
                name: chars_to_string(&user.name),
                is_local: user.is_local != 0,
                audio_muted: user.audio_muted != 0,
                video_on: user.video_on != 0,
            })
            .collect(),
        error: (status == "error").then(last_error),
    }
}

#[tauri::command]
fn sync_video_views(views: Vec<VideoView>) -> Result<(), String> {
    let ids = views
        .iter()
        .map(|view| c_string(&view.user_id))
        .collect::<Result<Vec<_>, _>>()?;
    let native = views
        .iter()
        .zip(&ids)
        .map(|(view, id)| NativeView {
            user_id: id.as_ptr(),
            x: view.x,
            y: view.y,
            width: view.width,
            height: view.height,
            scale: view.scale,
        })
        .collect::<Vec<_>>();
    check(unsafe { zoom_sync_views(native.as_ptr(), native.len()) })
}

#[tauri::command]
fn toggle_audio() -> Result<(), String> {
    check(unsafe { zoom_toggle_audio() })
}

#[tauri::command]
fn toggle_video() -> Result<(), String> {
    check(unsafe { zoom_toggle_video() })
}

#[tauri::command]
fn leave_session() -> Result<(), String> {
    check(unsafe { zoom_leave() })
}

fn check(code: i32) -> Result<(), String> {
    if code == 0 {
        Ok(())
    } else {
        Err(last_error())
    }
}

fn last_error() -> String {
    unsafe {
        let value = zoom_last_error();
        if value.is_null() {
            "Zoom Video SDK error".into()
        } else {
            CStr::from_ptr(value).to_string_lossy().into_owned()
        }
    }
}

fn c_string(value: &str) -> Result<CString, String> {
    CString::new(value).map_err(|_| "Text fields cannot contain a null character".into())
}

fn chars_to_string(value: &[c_char]) -> String {
    unsafe {
        CStr::from_ptr(value.as_ptr())
            .to_string_lossy()
            .into_owned()
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            initialize_sdk,
            join_session,
            get_session_state,
            sync_video_views,
            toggle_audio,
            toggle_video,
            leave_session
        ])
        .run(tauri::generate_context!())
        .expect("error while running Tauri");
    unsafe { zoom_cleanup() };
}
