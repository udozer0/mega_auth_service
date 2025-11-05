cmake_minimum_required(VERSION 3.16)
project(MegaAuthService CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# pkg-config для libpqxx и libsodium
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(SODIUM REQUIRED libsodium)

# 1) Сначала объявляем исполняемый таргет
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
    src/utils/validation.cpp
    src/domain/password_hash.cpp
    src/infra/db.cpp
    src/infra/user_repo.cpp
    src/infra/cert_repo.cpp
    src/infra/email_sender.cpp
    src/infra/pki_service.cpp
    src/infra/webauthn_service.cpp
    src/registration/PasswordRegistration.cpp
    src/registration/ClientCertRegistration.cpp
    src/registration/WebAuthnRegistration.cpp
    src/registration/RegistrationService.cpp
)

# 2) Потом добавляем include-директории
target_include_directories(mega_auth PRIVATE
  include
  third_party
  ${Boost_INCLUDE_DIRS}
  ${PQXX_INCLUDE_DIRS}
  ${SODIUM_INCLUDE_DIRS}
)

# 3) Затем линки
target_link_libraries(mega_auth
  ${Boost_LIBRARIES}
  ${PQXX_LIBRARIES}
  ${SODIUM_LIBRARIES}
  Threads::Threads
  nlohmann_json::nlohmann_json
)


# 4) Копирование OpenAPI после конфигурации таргета
add_custom_target(copy_openapi ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/openapi ${CMAKE_BINARY_DIR}/openapi
    COMMENT "Copying OpenAPI files to binary dir"
)
add_dependencies(mega_auth copy_openapi)
