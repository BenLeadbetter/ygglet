use std::env;
use std::path::PathBuf;
use std::process::Command;

fn main() {
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let conanfile = PathBuf::from("conanfile.py");
    let build_type = if env::var("PROFILE").unwrap() == "release" {
        "Release"
    } else {
        "Debug"
    };

    let status = Command::new("conan")
        .args([
            "install",
            conanfile.to_str().unwrap(),
            "--output-folder",
            out_dir.to_str().unwrap(),
            "--build=missing",
            "-s:h",
            &format!("build_type={}", build_type),
        ])
        .status()
        .expect("Failed to run conan install. Is conan installed and in PATH? Try activating your venv first.");

    if !status.success() {
        panic!("conan install failed");
    }

    println!("cargo:conan_output_dir={}", out_dir.to_str().unwrap());
    println!("cargo:build_type={}", build_type);
    println!("cargo:rerun-if-changed={}", conanfile.to_str().unwrap());
}
