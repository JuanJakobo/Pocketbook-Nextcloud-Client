from conan import ConanFile
from conan.tools.cmake import cmake_layout

class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("libcurl/8.4.0")
        if self.settings.build_type == "Debug":
            self.requires("gtest/1.14.0")

    def build_requirements(self):
        self.tool_requires("cmake/3.27.5")

    def layout(self):
        cmake_layout(self)
