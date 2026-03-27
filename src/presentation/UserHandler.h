#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <memory>
#include "../application/UserService.h"

namespace medical::presentation {

class UserHandler : public Poco::Net::HTTPRequestHandler {
private:
    std::shared_ptr<application::UserService> userService_;
    
    void handleCreateUser(Poco::Net::HTTPServerRequest& request,
                          Poco::Net::HTTPServerResponse& response);
    
    void handleGetUser(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response);
    
    void handleSearchUsers(Poco::Net::HTTPServerRequest& request,
                           Poco::Net::HTTPServerResponse& response);
    
    void sendJsonResponse(Poco::Net::HTTPServerResponse& response,
                          int status,
                          const Poco::JSON::Object& data);
    
    void sendErrorResponse(Poco::Net::HTTPServerResponse& response,
                          int status,
                          const std::string& message);
    
public:
    explicit UserHandler(std::shared_ptr<application::UserService> service);
    
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override;
};

} // namespace medical::presentation