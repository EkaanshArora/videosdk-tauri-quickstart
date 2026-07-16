#!/bin/sh
set -eu

binary="$1"
shift

profile_dir=$(dirname "$binary")
target_dir=$(dirname "$profile_dir")
app="$profile_dir/Zoom Video SDK Quickstart.app"
contents="$app/Contents"

mkdir -p "$contents/MacOS" "$contents/Frameworks"
ditto Info.dev.plist "$contents/Info.plist"
ditto "$binary" "$contents/MacOS/videosdk-tauri-quickstart"
ditto "$target_dir/Frameworks" "$contents/Frameworks"
codesign --force --deep --sign - "$app"

exec "$contents/MacOS/videosdk-tauri-quickstart" "$@"
