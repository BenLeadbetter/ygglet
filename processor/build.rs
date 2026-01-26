use std::env;
use std::path::PathBuf;

fn main() {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    // Read conan metadata from conan crate (via engine dependency)
    let conan_output_dir =
        PathBuf::from(env::var("DEP_CONAN_CONAN_OUTPUT_DIR").expect(
            "DEP_CONAN_CONAN_OUTPUT_DIR not set - is ygglet-conan a dependency?",
        ));
    let build_type = env::var("DEP_CONAN_BUILD_TYPE")
        .expect("DEP_CONAN_BUILD_TYPE not set - is ygglet-conan a dependency?");

    // Read engine metadata
    let engine_include_dir = PathBuf::from(
        env::var("DEP_YGGLET_ENGINE_INCLUDE_DIR")
            .expect("DEP_YGGLET_ENGINE_INCLUDE_DIR not set - is ygglet-engine a dependency?"),
    );
    let engine_lib_dir = PathBuf::from(
        env::var("DEP_YGGLET_ENGINE_LIB_DIR")
            .expect("DEP_YGGLET_ENGINE_LIB_DIR not set - is ygglet-engine a dependency?"),
    );

    let toolchain_file = conan_output_dir.join("conan_toolchain.cmake");

    // Build both library and test targets using cmake crate
    // Building tests will also build the library since it's a dependency
    let dst = cmake::Config::new(&manifest_dir)
        .define("CMAKE_TOOLCHAIN_FILE", toolchain_file.to_str().unwrap())
        .define("CMAKE_BUILD_TYPE", &build_type)
        .define("YGGLET_TESTS", "ON")
        .define("YGGLET_ENGINE_INCLUDE_DIR", engine_include_dir.to_str().unwrap())
        .define("YGGLET_ENGINE_LIB_DIR", engine_lib_dir.to_str().unwrap())
        .always_configure(true)
        .build_target("ygglet-processor-tests")
        .build();

    // Emit library search path
    let lib_dir = dst.join("build");
    println!("cargo:rustc-link-search=native={}", lib_dir.display());

    // Emit include directory for dependent crates
    let include_dir = manifest_dir.join("inc");
    println!("cargo:include_dir={}", include_dir.display());
    println!("cargo:lib_dir={}", lib_dir.display());

    // Emit test binary path
    let test_bin = dst.join("build").join("test").join("ygglet-processor-tests");
    println!("cargo:test_bin={}", test_bin.display());

    // Rerun if C++ sources change
    println!("cargo:rerun-if-changed={}/CMakeLists.txt", manifest_dir.display());
    println!("cargo:rerun-if-changed={}/src", manifest_dir.display());
    println!("cargo:rerun-if-changed={}/inc", manifest_dir.display());
    println!("cargo:rerun-if-changed={}/test", manifest_dir.display());
}
