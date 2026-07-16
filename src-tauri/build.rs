use std::{
    env, fs,
    path::{Path, PathBuf},
};

fn main() {
    tauri_build::build();
    println!("cargo:rerun-if-changed=native/zoom_bridge.h");

    match env::var("CARGO_CFG_TARGET_OS").as_deref() {
        Ok("macos") => build_macos(),
        Ok("windows") => build_windows(),
        _ => panic!("This quickstart supports macOS and Windows only"),
    }
}

fn build_macos() {
    let sdk = Path::new("../../zoom-video-sdk-macos-2.6.0/Sample-Libs/ZoomVideoSDK");
    cc::Build::new()
        .cpp(true)
        .file("native/macos.mm")
        .flag("-fobjc-arc")
        .flag(&format!("-F{}", sdk.display()))
        .compile("zoom_bridge");

    println!("cargo:rerun-if-changed=native/macos.mm");
    println!("cargo:rustc-link-search=framework={}", sdk.display());
    println!("cargo:rustc-link-lib=framework=ZMVideoSDK");
    println!("cargo:rustc-link-lib=framework=AppKit");
    println!(
        "cargo:rustc-link-arg=-Wl,-rpath,{}",
        absolute(sdk).display()
    );
}

fn build_windows() {
    let sdk = Path::new("../../zoom-video-sdk-windows-2.6.0/Sample-Libs/x64");
    cc::Build::new()
        .cpp(true)
        .file("native/windows.cpp")
        .include(sdk.join("h"))
        .flag_if_supported("/std:c++17")
        .compile("zoom_bridge");

    println!("cargo:rerun-if-changed=native/windows.cpp");
    println!("cargo:rerun-if-changed=native/windows_delegate_adapter.h");
    println!(
        "cargo:rustc-link-search=native={}",
        sdk.join("lib").display()
    );
    println!("cargo:rustc-link-lib=videosdk");
    println!("cargo:rustc-link-lib=user32");
    copy_windows_runtime(&sdk.join("bin"));
}

fn copy_windows_runtime(source: &Path) {
    // Cargo places OUT_DIR at target/<profile>/build/<package>/out.
    let mut destination = PathBuf::from(env::var("OUT_DIR").unwrap());
    for _ in 0..3 {
        destination.pop();
    }
    for entry in fs::read_dir(source).expect("read Windows SDK bin directory") {
        let entry = entry.unwrap();
        let path = entry.path();
        if path.is_file() {
            fs::copy(&path, destination.join(path.file_name().unwrap())).unwrap();
        }
    }
}

fn absolute(path: &Path) -> PathBuf {
    env::current_dir()
        .unwrap()
        .join(path)
        .canonicalize()
        .unwrap()
}
