
cmake_minimum_required(VERSION 3.16)
project(MegaAuthService CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Boost 1.74 REQUIRED COMPONENTS system)
add_executable(mega_auth
    src/main.cpp
    src/router.hpp
    src/middleware/cors.hpp
    src/handlers/health.hpp
    src/handlers/docs.hpp
    src/handlers/auth/auth_common.hpp
    src/handlers/auth/auth_register.hpp
    src/handlers/auth/auth_register.cpp
    src/handlers/auth/auth_login.hpp
    src/handlers/auth/auth_login.cpp
    src/handlers/auth/auth_device_login.hpp
    src/handlers/auth/auth_device_login.cpp
    src/handlers/auth/auth_forgot.hpp
    src/handlers/auth/auth_forgot.cpp
    src/handlers/auth/auth_reset.hpp
    src/handlers/auth/auth_reset.cpp
    src/handlers/auth/auth_change_password.hpp
    src/handlers/auth/auth_change_password.cpp
    src/handlers/auth/auth_logout.hpp
    src/handlers/auth/auth_logout.cpp
)
add_custom_target(copy_openapi ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/openapi ${CMAKE_BINARY_DIR}/openapi
    COMMENT "Copying OpenAPI files to binary dir"
)
add_dependencies(mega_auth copy_openapi)
target_include_directories(mega_auth PRIVATE include third_party ${Boost_INCLUDE_DIRS})
target_link_libraries(mega_auth PRIVATE ${Boost_LIBRARIES})
