from conans import ConanFile, CMake, tools
import os


class Sqlpp11ConnectorPostgresConan(ConanFile):
    name = "sqlpp11-connector-postgres"
    version = "0.60"
    license = "BSD 2"
    url = "https://github.com/0xBYTESHIFT/conan-sqlpp11-connector-postgres"
    description = "Conan package for the sqlpp11 postgres connector"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "CODE_COVERAGE": [True, False],
        "ENABLE_TESTS": [True, False],
        "SHARED": [True, False],
    }

    default_options = {
        "CODE_COVERAGE": False,
        "ENABLE_TESTS": True,
        "SHARED": True,
    }
    generators = ("cmake", "virtualbuildenv", "virtualrunenv")
    build_policy = "missing"

    def requirements(self):
        self.requires("libpq/13.1")
        self.requires("sqlpp11/0.60")

    def source(self):
        git = tools.Git(folder="git-src")
        git.clone("https://github.com/matthijs/sqlpp11-connector-postgresql", "master")
        os.rename("git-src", self.name)

        cmake_loc = "".join([self.name, "/CMakeLists.txt"])
        tools.replace_in_file(cmake_loc, "if (NOT TARGET sqlpp11)",
            ''' if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
                    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
                    conan_basic_setup()
                else ()
                    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
                endif ()
                if (NOT TARGET sqlpp11)
            ''')
    def build(self):
        tools.replace_in_file(os.path.join(self.source_folder, self.name, "src/CMakeLists.txt"),
                "target_link_libraries(sqlpp11-connector-postgresql PRIVATE sqlpp11::sqlpp11 $<BUILD_INTERFACE:${PostgreSQL_LIBRARIES}>)",
                "target_link_libraries(sqlpp11-connector-postgresql PRIVATE sqlpp11::sqlpp11 ${CONAN_LIBS} $<BUILD_INTERFACE:${PostgreSQL_LIBRARIES}>)"
        )
        tools.replace_in_file(os.path.join(self.source_folder, self.name, "src/CMakeLists.txt"),
                "target_link_libraries(sqlpp11-connector-postgresql-dynamic PUBLIC sqlpp11::sqlpp11 PRIVATE ${PostgreSQL_LIBRARIES})",
                "target_link_libraries(sqlpp11-connector-postgresql-dynamic PUBLIC sqlpp11::sqlpp11 ${CONAN_LIBS} PRIVATE ${PostgreSQL_LIBRARIES})"
        )

        options = {
            "CODE_COVERAGE": "ON" if self.options.CODE_COVERAGE else "OFF",
            "ENABLE_TESTS": "ON" if self.options.ENABLE_TESTS else "OFF",
            "SHARED": "ON" if self.options.SHARED else "OFF",
        }

        cmake = CMake(self)
        cmake.definitions = options
        cmake.configure(source_folder=os.path.join(self.source_folder, self.name))
        if self.options.SHARED:
            cmake.build(target='sqlpp11-connector-postgresql-dynamic')
        else:
            cmake.build(target='sqlpp11-connector-postgresql')

    def package(self):
        if self.settings.compiler == "Visual Studio":
            cmake = CMake(self)
            cmake.install()
        else:
            self.copy(pattern="*.h*", dst="include", src="".join(self.name, "/include/data_types"))
            self.copy(pattern="*.h*", dst="include", src="".join(self.name, "/include/data_types/uuid"))
            self.copy(pattern="*.h*", dst="include", src="".join(self.name, "/include/postgresql"))

            if self.options.SHARED:
                self.copy(pattern="*.dylib", dst="lib", src="lib")
                self.copy(pattern="*.so", dst="lib", src="lib")
            else:
                self.copy(pattern="*.a", dst="lib", src="lib")

    def package_info(self):
        self.cpp_info.libs = [self.name]

        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.bindirs = ["bin"]
