from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps
import os


class YggletConan(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    requires = (
        "boost/1.90.0",
        "catch2/3.11.0",
        "cmajor/1.0.3088",
        "foonathan-memory/0.7.3",
        "spdlog/1.16.0",
        "tl-expected/20190710",
        "strong_type/v9",
    )

    def generate(self):
        tc = CMakeToolchain(self, generator=os.environ.get("CMAKE_GENERATOR"))
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()
