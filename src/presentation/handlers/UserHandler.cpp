#include "UserHandler.h"
#include "../../shared/ErrorHandler.h"
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

namespace medical::presentation {

UserSearchHandler::UserSearchHandler(std::shared_ptr<application::UserService> userService)
    : userService_(userService) {}

void UserSearchHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                     Poco::Net::HTTPServerResponse& response) {
    try {
        Poco::URI uri(request.getURI());
        auto params = uri.getQueryParameters();
        
        std::string mask;
        for (const auto& param : params) {
            if (param.first == "mask") {
                mask = param.second;
                break;
            }
        }
        
        if (mask.empty()) {
            shared::ErrorHandler::sendError(response, 
                Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                "Missing mask parameter");
            return;
        }
        
        auto users = userService_->searchUsersByName(mask);
        
        Poco::JSON::Array usersArray;
        for (const auto& user : users) {
            Poco::JSON::Object userObj;
            userObj.set("id", user.id);
            userObj.set("login", user.login);
            userObj.set("email", user.email);
            userObj.set("fullName", user.fullName);
            usersArray.add(userObj);
        }
        
        Poco::JSON::Object responseObj;
        responseObj.set("users", usersArray);
        
        shared::ErrorHandler::sendJsonResponse(response, 
            Poco::Net::HTTPResponse::HTTP_OK, responseObj);
            
    } catch (const std::exception& e) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, e.what());
    }
}

} // namespace medical::presentation
