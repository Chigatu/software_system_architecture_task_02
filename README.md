# Medical Records Management System

REST API сервис для управления медицинскими записями (Вариант 20 домашнего задания).

## Функциональность

- Регистрация и аутентификация пользователей (JWT)
- Поиск пользователей по логину и маске имени
- Регистрация пациентов
- Поиск пациентов по ФИО
- Создание медицинских записей
- Получение истории записей пациента
- Получение записи по уникальному коду

## Технологии

- C++17
- POCO C++ Libraries (Net, JSON, Foundation)
- JWT аутентификация
- In-memory хранение данных
- OpenAPI 3.0 / Swagger UI

## Требования

- Linux / WSL2 / macOS / Windows (с MSYS2)
- CMake 3.14+
- GCC 9+ или Clang 10+ или MSVC 2019+
- POCO C++ Libraries
- Docker (опционально)

## Быстрый старт (Docker)

Самый простой способ запустить сервис - использовать Docker. Сервис будет доступен на любой операционной системе, где установлен Docker.

### Windows / macOS / Linux

1. Установите Docker Desktop с официального сайта
2. Склонируйте репозиторий:
   ```
   git clone https://github.com/Chigatu/medical-records-system.git
   cd medical-records-system
   ```
3. Запустите сервис:
   ```
   docker-compose up --build
   ```
4. Откройте в браузере: http://localhost:8080/docs

## Установка POCO библиотек для ручной сборки

### Ubuntu / Debian / WSL2

```
sudo apt update
sudo apt install -y libpoco-dev cmake build-essential
```

### CentOS / RHEL / Fedora

```
sudo dnf install -y poco-devel cmake gcc-c++
```

### Arch Linux

```
sudo pacman -S poco cmake base-devel
```

### macOS (Homebrew)

```
brew install poco cmake
```

### Windows (MSYS2)

1. Установите MSYS2 с официального сайта
2. Откройте MSYS2 UCRT64 и выполните:
   ```
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-poco
   ```

## Ручная сборка и запуск

```
git clone https://github.com/yourusername/medical-records-system.git
cd medical-records-system
mkdir build && cd build
cmake ..
make -j4
./medical_api
```

После запуска сервер будет доступен по адресу: http://localhost:8080

## Автоматизированная сборка (Makefile)

В корне проекта доступен Makefile для упрощения сборки и запуска:

```
make build    # Сборка проекта
make run      # Сборка и запуск
make clean    # Очистка build директории
make test     # Запуск тестов
make help     # Показать все доступные команды
```

## API Endpoints

| Метод | URL | Описание | Требуется JWT |
|-------|-----|----------|---------------|
| GET | /health | Проверка работоспособности | Нет |
| POST | /api/auth/register | Регистрация пользователя | Нет |
| POST | /api/auth/login | Вход в систему | Нет |
| GET | /api/users/search?mask={mask} | Поиск пользователей по маске имени | Нет |
| POST | /api/patients | Создание пациента | Да |
| GET | /api/patients/search?fullName={name} | Поиск пациентов по ФИО | Нет |
| POST | /api/medical-records | Создание медицинской записи | Да |
| GET | /api/medical-records/patient/{id} | Получение истории записей пациента | Да |
| GET | /api/medical-records/{code} | Получение записи по коду | Да |

## Документация API

Swagger UI доступен по адресу: http://localhost:8080/docs

OpenAPI спецификация: http://localhost:8080/swagger/openapi.yaml

## Примеры запросов

### 1. Регистрация пользователя

```
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "login": "dr.ivanov",
    "email": "ivanov@hospital.ru",
    "firstName": "Иван",
    "lastName": "Иванов",
    "patronymic": "Петрович",
    "password": "secret123"
  }'
```

### 2. Вход в систему

```
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "dr.ivanov",
    "password": "secret123"
  }'
```

Ответ:

```
{
  "token": "a1b2c3d4e5f6...",
  "user": {
    "id": 1,
    "login": "dr.ivanov",
    "email": "ivanov@hospital.ru",
    "fullName": "Иванов Иван Петрович"
  }
}
```

### 3. Поиск пользователей по маске имени

```
curl "http://localhost:8080/api/users/search?mask=Иван"
```

### 4. Создание пациента (требуется JWT)

```
TOKEN="your-token-from-login"

curl -X POST http://localhost:8080/api/patients \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "firstName": "Мария",
    "lastName": "Петрова",
    "patronymic": "Алексеевна",
    "phone": "+79991234567",
    "address": "г. Москва, ул. Ленина, д. 1",
    "birthDate": "1990-05-15",
    "snils": "123-456-789 01",
    "policyNumber": "1234567890123456"
  }'
```

### 5. Поиск пациентов по ФИО

```
curl "http://localhost:8080/api/patients/search?fullName=Петрова"
```

### 6. Создание медицинской записи (требуется JWT)

```
curl -X POST http://localhost:8080/api/medical-records \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "patientId": 1,
    "doctorId": 1,
    "diagnosisCode": "J06.9",
    "diagnosisDescription": "Острая респираторная инфекция",
    "complaints": "Кашель, насморк, температура 38.5"
  }'
```

### 7. Получение истории записей пациента (требуется JWT)

```
curl -H "Authorization: Bearer $TOKEN" \
  http://localhost:8080/api/medical-records/patient/1
```

### 8. Получение записи по коду (требуется JWT)

```
curl -H "Authorization: Bearer $TOKEN" \
  http://localhost:8080/api/medical-records/MED-20260327-00001
```

## Тестирование

Для автоматического тестирования всех эндпоинтов выполните:

```
./test_api.sh
```

Тестовый скрипт проверит:
- Регистрацию и аутентификацию
- Создание пациентов
- Создание медицинских записей
- Поиск по маскам
- Защиту эндпоинтов JWT

## Структура проекта

```
medical-records-system/
├── src/
│   └── main.cpp                 # Единый файл с реализацией
├── web/
│   ├── docs/
│   │   └── index.html           # Swagger UI интерфейс
│   └── swagger/
│       └── openapi.yaml         # OpenAPI спецификация
├── test_api.sh                  # Автоматические тесты
├── Makefile                     # Автоматизация сборки
├── Dockerfile                   # Docker образ
├── docker-compose.yaml          # Docker Compose конфигурация
├── CMakeLists.txt               # CMake конфигурация
└── README.md                    # Документация
```

## Устранение неполадок

### Docker: permission denied

Если при запуске Docker возникает ошибка доступа, добавьте пользователя в группу docker:

```
sudo usermod -aG docker $USER
# Выйдите и зайдите заново
```

### Порт 8080 уже используется

Измените порт в docker-compose.yaml:

```
ports:
  - "8081:8080"
```

### Ошибка сборки в WSL2

Убедитесь, что установлены все зависимости:

```
sudo apt update
sudo apt install -y build-essential cmake libpoco-dev
```