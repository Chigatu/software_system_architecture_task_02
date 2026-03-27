#pragma once
#include "../domain/User.h"
#include "../domain/interfaces/IRepository.h"
#include <memory>
#include <vector>
#include <optional>

namespace medical::application {

struct CreateUserRequest {
    std::string login;
    std::string email;
    std::string firstName;
    std::string lastName;
    std::string patronymic;
    std::string password;
};

struct UserResponse {
    int id;
    std::string login;
    std::string email;
    std::string fullName;
    bool isActive;
};

struct LoginRequest {
    std::string login;
    std::string password;
};

struct LoginResponse {
    std::string token;
    UserResponse user;
};

class UserService {
private:
    std::shared_ptr<domain::IUserRepository> userRepository_;
    
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    std::string generateJWT(int userId, const std::string& login);
    
public:
    explicit UserService(std::shared_ptr<domain::IUserRepository> repo);
    
    UserResponse createUser(const CreateUserRequest& request);
    std::optional<UserResponse> getUserById(int id);
    std::optional<UserResponse> getUserByLogin(const std::string& login);
    std::vector<UserResponse> searchUsersByFullName(const std::string& mask);
    LoginResponse login(const LoginRequest& request);
    bool validateToken(const std::string& token);
    
private:
    UserResponse mapToResponse(const domain::User& user);
};

} // namespace medical::application
