cmake_minimum_required(VERSION 3.14)
project(MedicalRecordsSystem VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск POCO библиотек
find_package(Poco REQUIRED COMPONENTS Foundation Net JSON)

# Собираем все исходники рекурсивно
file(GLOB_RECURSE SOURCES
    "src/domain/entities/*.cpp"
    "src/domain/value_objects/*.cpp"
    "src/application/services/*.cpp"
    "src/infrastructure/repositories/*.cpp"
    "src/infrastructure/security/*.cpp"
    "src/presentation/handlers/*.cpp"
    "src/presentation/*.cpp"
    "src/shared/*.cpp"
    "src/main.cpp"
)

# Создаем исполняемый файл
add_executable(medical_api ${SOURCES})

# Включаемые директории
target_include_directories(medical_api PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/value_objects
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/services
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dtos
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure/repositories
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure/security
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/handlers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shared
)

# Линковка библиотек
target_link_libraries(medical_api
    Poco::Foundation
    Poco::Net
    Poco::JSON
)

# Подавляем предупреждения для чистоты вывода
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(medical_api PRIVATE -Wno-unused-parameter)
endif()
