#include "AuthHandler.h"
#include "../../shared/ErrorHandler.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <sstream>
#include <iostream>

namespace medical::presentation {

RegisterHandler::RegisterHandler(std::shared_ptr<application::UserService> userService)
    : userService_(userService) {}

void RegisterHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                   Poco::Net::HTTPServerResponse& response) {
    std::cout << "=== RegisterHandler::handleRequest ===" << std::endl;
    try {
        // Читаем тело запроса
        std::string body;
        std::istream& in = request.stream();
        std::string line;
        while (std::getline(in, line)) {
            body += line + "\n";
        }
        std::cout << "Request body: " << body << std::endl;
        
        // Парсим JSON
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(body);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        
        // Создаем DTO
        application::dto::RegisterRequestDTO dto;
        dto.login = json->getValue<std::string>("login");
        dto.email = json->getValue<std::string>("email");
        dto.firstName = json->getValue<std::string>("firstName");
        dto.lastName = json->getValue<std::string>("lastName");
        dto.patronymic = json->optValue<std::string>("patronymic", "");
        dto.password = json->getValue<std::string>("password");
        
        std::cout << "Calling userService_->registerUser" << std::endl;
        
        // Регистрируем пользователя
        auto user = userService_->registerUser(dto);
        if (!user.has_value()) {
            std::cout << "Registration failed!" << std::endl;
            shared::ErrorHandler::sendError(response, 
                Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                "User already exists or invalid data");
            return;
        }
        
        std::cout << "Registration successful, user ID: " << user->id << std::endl;
        
        // Формируем ответ
        Poco::JSON::Object responseObj;
        responseObj.set("id", user->id);
        responseObj.set("login", user->login);
        responseObj.set("email", user->email);
        responseObj.set("fullName", user->fullName);
        
        shared::ErrorHandler::sendJsonResponse(response, 
            Poco::Net::HTTPResponse::HTTP_CREATED, responseObj);
            
    } catch (const std::exception& e) {
        std::cout << "Exception in RegisterHandler: " << e.what() << std::endl;
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, e.what());
    }
}

LoginHandler::LoginHandler(std::shared_ptr<application::AuthService> authService)
    : authService_(authService) {}

void LoginHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                Poco::Net::HTTPServerResponse& response) {
    std::cout << "=== LoginHandler::handleRequest ===" << std::endl;
    try {
        // Читаем тело запроса
        std::string body;
        std::istream& in = request.stream();
        std::string line;
        while (std::getline(in, line)) {
            body += line + "\n";
        }
        std::cout << "Request body: " << body << std::endl;
        
        // Парсим JSON
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(body);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        
        // Создаем DTO
        application::dto::LoginRequestDTO dto;
        dto.login = json->getValue<std::string>("login");
        dto.password = json->getValue<std::string>("password");
        
        std::cout << "Calling authService_->login" << std::endl;
        
        // Аутентифицируем
        auto loginResponse = authService_->login(dto);
        if (!loginResponse.has_value()) {
            std::cout << "Login failed!" << std::endl;
            shared::ErrorHandler::sendError(response, 
                Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                "Invalid credentials");
            return;
        }
        
        std::cout << "Login successful, token: " << loginResponse->token << std::endl;
        
        // Формируем ответ
        Poco::JSON::Object responseObj;
        responseObj.set("token", loginResponse->token);
        
        Poco::JSON::Object userObj;
        userObj.set("id", loginResponse->user.id);
        userObj.set("login", loginResponse->user.login);
        userObj.set("email", loginResponse->user.email);
        userObj.set("fullName", loginResponse->user.fullName);
        
        responseObj.set("user", userObj);
        
        shared::ErrorHandler::sendJsonResponse(response, 
            Poco::Net::HTTPResponse::HTTP_OK, responseObj);
            
    } catch (const std::exception& e) {
        std::cout << "Exception in LoginHandler: " << e.what() << std::endl;
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, e.what());
    }
}

} // namespace medical::presentation
