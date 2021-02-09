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
        "SHARED": [True, False],
    }

    default_options = {
        "SHARED": False,
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
        replace = '''if (NOT TARGET sqlpp11)
    find_package(Sqlpp11 REQUIRED)
endif()
find_package(PostgreSQL REQUIRED)'''
        tools.replace_in_file(cmake_loc, replace,
            ''' if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
                    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
                    conan_basic_setup()
                else ()
                    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
                endif ()
            ''')
    def build(self):
        tools.replace_in_file(os.path.join(self.source_folder, self.name, "src/CMakeLists.txt"),
                "target_link_libraries(sqlpp11-connector-postgresql PRIVATE sqlpp11::sqlpp11 $<BUILD_INTERFACE:${PostgreSQL_LIBRARIES}>)",
                "target_link_libraries(sqlpp11-connector-postgresql PRIVATE ${CONAN_LIBS} $<BUILD_INTERFACE:${PostgreSQL_LIBRARIES}>)"
        )
        tools.replace_in_file(os.path.join(self.source_folder, self.name, "src/CMakeLists.txt"),
                "target_link_libraries(sqlpp11-connector-postgresql-dynamic PUBLIC sqlpp11::sqlpp11 PRIVATE ${PostgreSQL_LIBRARIES})",
                "target_link_libraries(sqlpp11-connector-postgresql-dynamic PUBLIC ${CONAN_LIBS} PRIVATE ${PostgreSQL_LIBRARIES})"
        )

        options = {
            "ENABLE_TESTS": "OFF",
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
        self.copy(pattern="*.h*", dst="include/sqlpp11/data_types", src="".join([self.name, "/include/sqlpp11/data_types"]))
        self.copy(pattern="*.h*", dst="include/sqlpp11/data_types/uuid", src="".join([self.name, "/include/sqlpp11/data_types/uuid"]))
        self.copy(pattern="*.h*", dst="include/sqlpp11/postgresql", src="".join([self.name, "/include/sqlpp11/postgresql"]))

        if self.options.SHARED:
            self.copy(pattern="*.so", dst="lib", src="lib")
        else:
            self.copy(pattern="*.a", dst="lib", src="lib")

    def package_info(self):
        if self.options.SHARED:
            self.cpp_info.libs = ["sqlpp11-connector-postgresql-dynamic"]
        else:
            self.cpp_info.libs = ["sqlpp11-connector-postgresql"]

        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.bindirs = ["bin"]
