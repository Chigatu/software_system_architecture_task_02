#include "UserService.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>

namespace medical::application {

UserService::UserService(std::shared_ptr<domain::IUserRepository> repo)
    : userRepository_(repo) {}

std::string UserService::hashPassword(const std::string& password) {
    // Простой хеш для демо (в реальности используйте bcrypt или подобное)
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
}

bool UserService::verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

std::string UserService::generateJWT(int userId, const std::string& login) {
    // Простой JWT-like токен для демо (в реальности используйте нормальную JWT библиотеку)
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()).count();
    
    std::stringstream ss;
    ss << userId << ":" << login << ":" << timestamp;
    return hashPassword(ss.str());
}

UserResponse UserService::createUser(const CreateUserRequest& request) {
    // Check if user exists
    if (userRepository_->findByLogin(request.login).has_value()) {
        throw std::runtime_error("User with this login already exists");
    }
    
    // Create domain objects
    domain::Email email(request.email);
    domain::FullName fullName(request.firstName, request.lastName, request.patronymic);
    std::string hashedPassword = hashPassword(request.password);
    
    // Create user
    int nextId = userRepository_->findAll().size() + 1;
    domain::User user(nextId, request.login, email, fullName, hashedPassword);
    
    userRepository_->save(user);
    
    return mapToResponse(user);
}

std::optional<UserResponse> UserService::getUserById(int id) {
    auto user = userRepository_->findById(id);
    if (!user) {
        return std::nullopt;
    }
    return mapToResponse(*user);
}

std::optional<UserResponse> UserService::getUserByLogin(const std::string& login) {
    auto user = userRepository_->findByLogin(login);
    if (!user) {
        return std::nullopt;
    }
    return mapToResponse(*user);
}

std::vector<UserResponse> UserService::searchUsersByFullName(const std::string& mask) {
    auto users = userRepository_->findByFullNameMask(mask);
    std::vector<UserResponse> responses;
    for (const auto& user : users) {
        responses.push_back(mapToResponse(user));
    }
    return responses;
}

LoginResponse UserService::login(const LoginRequest& request) {
    auto user = userRepository_->findByLogin(request.login);
    if (!user) {
        throw std::runtime_error("Invalid credentials");
    }
    
    if (!verifyPassword(request.password, user->passwordHash())) {
        throw std::runtime_error("Invalid credentials");
    }
    
    if (!user->isActive()) {
        throw std::runtime_error("User account is deactivated");
    }
    
    LoginResponse response;
    response.token = generateJWT(user->id(), user->login());
    response.user = mapToResponse(*user);
    
    return response;
}

bool UserService::validateToken(const std::string& token) {
    // Простая валидация токена
    return !token.empty() && token.find(":") != std::string::npos;
}

UserResponse UserService::mapToResponse(const domain::User& user) {
    UserResponse response;
    response.id = user.id();
    response.login = user.login();
    response.email = user.email().value();
    response.fullName = user.fullName().fullName();
    response.isActive = user.isActive();
    return response;
}

} // namespace medical::application
