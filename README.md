# Zoom Video SDK + Tauri quickstart

A small Zoom Video SDK 2.6.0 example for macOS and Windows x64. It can:

- create or join a session
- show local and remote participants in one video grid
- mute or unmute audio
- start or stop video
- leave the session

Zoom callbacks drive all session updates. The app does not poll the SDK.

## Project layout

- `src/main.ts` — join form, video grid, and controls
- `src-tauri/src/lib.rs` — Tauri commands shared by both platforms
- `src-tauri/native/macos.mm` — macOS SDK integration
- `src-tauri/native/windows.cpp` — Windows SDK integration
- `src-tauri/native/windows_delegate_adapter.h` — unused required Windows callbacks
- `scripts/generateToken.ts` — local JWT generator

## Put the SDKs in place

The Zoom SDK binaries are not included in Git. Unzip both 2.6.0 SDK folders
directly into this repository, beside `package.json`:

```text
videosdk-tauri-quickstart/
├── package.json
├── zoom-video-sdk-macos-2.6.0/
└── zoom-video-sdk-windows-2.6.0/
```

The build expects these exact paths:

```text
zoom-video-sdk-macos-2.6.0/Sample-Libs/ZoomVideoSDK/
zoom-video-sdk-windows-2.6.0/Sample-Libs/x64/
```

Run all commands below from this repository root.

## Configure credentials and generate a JWT

Install the frontend dependencies:

```sh
bun install
```

Copy `.env.example` to `.env`, then add credentials from a Zoom Video SDK app:

```dotenv
SDK_KEY=your_video_sdk_key
SDK_SECRET=your_video_sdk_secret
```

`.env` is ignored by Git. Do not put the SDK secret in frontend code or commit
it to the repository.

Generate a two-hour host token:

```sh
bun run token "My Session" host
```

Paste the token into the app and enter `My Session` as the session name. The
session name is case-sensitive and must exactly match the name used to create
the token.

To join the same session from another copy of the app, generate a participant
token:

```sh
bun run token "My Session" participant
```

## Run on macOS

Requirements:

- macOS 10.15 or newer
- Bun
- Rust
- Xcode command-line tools

Install the Xcode command-line tools if needed:

```sh
xcode-select --install
```

Start the app:

```sh
bun tauri dev
```

The Zoom macOS SDK must run from a real `.app` bundle. This project therefore
uses a small Cargo runner that places the debug executable and Zoom frameworks
in `src-tauri/target/debug/Zoom Video SDK Quickstart.app`, ad-hoc signs that
development bundle, and launches it. Do not run the bare
`src-tauri/target/debug/videosdk-tauri-quickstart` executable directly; Zoom
will fail to find its internal modules and return initialization error 5.

Allow camera and microphone access when macOS asks. If macOS blocks a Zoom
framework or helper, open **System Settings → Privacy & Security**, approve the
blocked item, and start the app again.

The duplicate Objective-C class warnings printed by Zoom's frameworks also
occur with Zoom's SDK layout and do not indicate an initialization failure.

To build a macOS app bundle:

```sh
bun tauri build --bundles app
```

The result is written under:

```text
src-tauri/target/release/bundle/macos/
```

The bundle includes the required Zoom frameworks and helper applications.
Signing and notarization are left to the application that adopts this sample.

## Run on Windows x64

Requirements:

- 64-bit Windows 10 or newer
- Bun
- Rust with the x64 MSVC toolchain
- Visual Studio 2019 or newer with **Desktop development with C++**
- Microsoft Edge WebView2 Runtime, if it is not already installed

Open PowerShell and verify the tools:

```powershell
bun --version
rustc --version
cargo --version
rustup default stable-x86_64-pc-windows-msvc
```

If the Windows SDK was downloaded as a ZIP, unblock the ZIP before extracting
it. If it has already been extracted, you can unblock only that SDK folder:

```powershell
Get-ChildItem .\zoom-video-sdk-windows-2.6.0 -Recurse | Unblock-File
```

Start the app:

```powershell
bun tauri dev
```

Allow camera and microphone access when Windows asks. During the build, all
files from `Sample-Libs/x64/bin` are copied beside the development executable.

To build a Windows installer:

```powershell
bun tauri build --bundles nsis
```

The result is written under:

```text
src-tauri\target\release\bundle\nsis\
```

The installer includes the required Zoom x64 runtime files. Production
installers should be code-signed.

## Test with two participants

1. Generate a host token and join `My Session` on the first machine.
2. Generate a participant token for the exact same session name.
3. Join from the other platform or another computer with a different user name.
4. Confirm both videos appear in the grid.
5. Test mute/unmute, start/stop video, window resizing, and leaving.

## Common Windows errors

- **`videosdk.dll was not found`** — check that
  `zoom-video-sdk-windows-2.6.0/Sample-Libs/x64/bin/videosdk.dll` exists, then
  rebuild.
- **The linker cannot find `videosdk.lib`** — check that
  `Sample-Libs/x64/lib/videosdk.lib` exists.
- **A C++ compiler was not found** — install Visual Studio's **Desktop
  development with C++** workload and reopen PowerShell.
- **The JWT is rejected** — generate it again using the exact session name
  entered in the app.
