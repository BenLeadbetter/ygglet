// ygglet-processor crate
// This crate builds the C++ processor library via build.rs

#[cfg(test)]
mod tests {
    use std::process::Command;

    #[test]
    fn cpp_tests() {
        // Use OUT_DIR to find the test binary built by build.rs
        let out_dir = env!("OUT_DIR");
        let test_bin = std::path::Path::new(out_dir).join("ygglet-processor-tests");

        let status = Command::new(&test_bin)
            .status()
            .expect("Failed to run C++ tests");
        assert!(status.success(), "C++ tests failed");
    }
}
