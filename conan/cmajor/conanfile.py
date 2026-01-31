from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.scm import Git
from conan.tools.files import copy
import os


class CmajorConan(ConanFile):
    name = "cmajor"
    version = "1.0.3088"
    license = "GPLv3"
    settings = "os", "arch", "compiler", "build_type"

    def layout(self):
        cmake_layout(self)

    def source(self):
        git = Git(self)
        git.clone(
            "https://github.com/cmajor-lang/cmajor.git",
            target=".",
            args=["--recurse-submodules"],
        )
        git.checkout(self.version)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(
            variables={
                "BUILD_CMAJ": False,
                "BUILD_CMAJ_LIB": True,
                "BUILD_EXAMPLES": False,
            }
        )
        cmake.build()

    def package(self):
        # Copy headers
        copy(
            self,
            "*.h",
            src=os.path.join(self.source_folder, "include"),
            dst=os.path.join(self.package_folder, "include"),
            keep_path=True,
        )

        # Copy the shared library - search recursively from build folder
        # macOS
        copy(
            self,
            "**/libCmajPerformer.dylib",
            src=self.build_folder,
            dst=os.path.join(self.package_folder, "lib"),
            keep_path=False,
        )
        # Linux
        copy(
            self,
            "**/libCmajPerformer.so*",
            src=self.build_folder,
            dst=os.path.join(self.package_folder, "lib"),
            keep_path=False,
        )
        # Windows
        copy(
            self,
            "**/CmajPerformer.dll",
            src=self.build_folder,
            dst=os.path.join(self.package_folder, "bin"),
            keep_path=False,
        )
        copy(
            self,
            "**/CmajPerformer.lib",
            src=self.build_folder,
            dst=os.path.join(self.package_folder, "lib"),
            keep_path=False,
        )

        # Copy standard library (cmajor code files)
        copy(
            self,
            "*.cmajor",
            src=os.path.join(self.source_folder, "standard_library"),
            dst=os.path.join(self.package_folder, "res", "standard_library"),
            keep_path=True,
        )

    def package_info(self):
        self.cpp_info.libs = ["CmajPerformer"]
        self.cpp_info.includedirs = ["include"]

        # Set CMAJOR_DLL define so users know to use dynamic linking
        self.cpp_info.defines = ["CMAJOR_DLL=1"]

        # Add standard library path as a runtime data path
        self.cpp_info.resdirs = ["res"]
