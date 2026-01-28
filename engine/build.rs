use std::env;
use std::path::{Path, PathBuf};
use std::process::Command;

fn main() {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    // Run conan install (conan2 finds conanfile.txt in cwd automatically)
    let metadata = conan2::ConanInstall::new()
        .build("missing")
        .run()
        .parse();

    // Build static library with cc
    let mut build = cc::Build::new();
    build.cpp(true).std("c++20");
    build.include(manifest_dir.join("inc"));
    build.include(manifest_dir.join("src"));
    for path in metadata.include_paths() {
        build.include(path);
    }
    build
        .file(manifest_dir.join("src/ygglet/engine/connection.cpp"))
        .file(manifest_dir.join("src/ygglet/engine/detail/control_graph.cpp"))
        .file(manifest_dir.join("src/ygglet/engine/detail/render_graph.cpp"))
        .file(manifest_dir.join("src/ygglet/engine/engine.cpp"))
        .file(manifest_dir.join("src/ygglet/engine/node.cpp"));
    build.compile("ygglet-engine");

    // Emit metadata for dependent crates (processor)
    println!("cargo:include_dir={}", manifest_dir.join("inc").display());
    println!("cargo:lib_dir={}", out_dir.display());

    // Emit conan link directives (for final binary linking)
    metadata.emit();

    // Build test executable
    build_test_executable(&manifest_dir, &out_dir, &metadata);

    // Rerun triggers
    println!("cargo:rerun-if-changed=conanfile.txt");
    println!("cargo:rerun-if-changed=src");
    println!("cargo:rerun-if-changed=inc");
    println!("cargo:rerun-if-changed=test");
}

fn build_test_executable(
    manifest_dir: &Path,
    out_dir: &Path,
    metadata: &conan2::CargoInstructions,
) {
    let compiler = cc::Build::new().cpp(true).std("c++20").get_compiler();
    let (link_libs, link_search_paths) = parse_cargo_instructions(metadata);

    let test_bin = out_dir.join("ygglet-engine-tests");
    let mut cmd = Command::new(compiler.path());
    cmd.arg("-std=c++20");

    // Sources
    cmd.arg(manifest_dir.join("test/engine.test.cpp"));

    // Include paths
    cmd.arg(format!("-I{}", manifest_dir.join("inc").display()));
    cmd.arg(format!("-I{}", manifest_dir.join("src").display()));
    for path in metadata.include_paths() {
        cmd.arg(format!("-I{}", path.display()));
    }

    // Library search paths
    cmd.arg(format!("-L{}", out_dir.display()));
    for path in metadata.library_paths() {
        cmd.arg(format!("-L{}", path.display()));
    }
    for path in &link_search_paths {
        cmd.arg(format!("-L{}", path.display()));
    }

    // Libraries (our lib first, then conan deps)
    cmd.arg("-lygglet-engine");
    for lib in &link_libs {
        cmd.arg(format!("-l{}", lib));
    }

    // C++ stdlib
    if cfg!(target_os = "macos") {
        cmd.arg("-lc++");
    } else {
        cmd.arg("-lstdc++");
    }

    cmd.arg("-o").arg(&test_bin);

    let status = cmd.status().expect("Failed to compile test executable");
    assert!(status.success(), "Test executable compilation failed");

    println!("cargo:test_bin={}", test_bin.display());
}

/// Parse cargo:rustc-link-lib and cargo:rustc-link-search from CargoInstructions
fn parse_cargo_instructions(
    metadata: &conan2::CargoInstructions,
) -> (Vec<String>, Vec<PathBuf>) {
    let text = String::from_utf8_lossy(metadata.as_bytes());
    let mut libs = Vec::new();
    let mut paths = Vec::new();
    for line in text.lines() {
        if let Some(lib) = line.strip_prefix("cargo:rustc-link-lib=") {
            let name = lib.split_once('=').map_or(lib, |(_, n)| n);
            libs.push(name.to_string());
        } else if let Some(path) = line.strip_prefix("cargo:rustc-link-search=") {
            let p = path.strip_prefix("native=").unwrap_or(path);
            paths.push(PathBuf::from(p));
        }
    }
    (libs, paths)
}
