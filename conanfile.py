from conan import ConanFile


class MyProjectConan(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    requires = (
        "boost/1.90.0",
        "catch2/3.11.0",
        "cmajor/1.0.3088",
        "spdlog/1.16.0",
        "tl-expected/20190710",
    )
    generators = ("CMakeToolchain", "CMakeDeps")
