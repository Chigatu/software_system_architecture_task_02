#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <memory>
#include "../../application/services/AuthService.h"
#include "../../application/services/UserService.h"

namespace medical::presentation {

class RegisterHandler : public Poco::Net::HTTPRequestHandler {
private:
    std::shared_ptr<application::UserService> userService_;
    
public:
    explicit RegisterHandler(std::shared_ptr<application::UserService> userService);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                      Poco::Net::HTTPServerResponse& response) override;
};

class LoginHandler : public Poco::Net::HTTPRequestHandler {
private:
    std::shared_ptr<application::AuthService> authService_;
    
public:
    explicit LoginHandler(std::shared_ptr<application::AuthService> authService);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                      Poco::Net::HTTPServerResponse& response) override;
};

} // namespace medical::presentation
