#pragma once
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Object.h>
#include <string>

namespace medical::shared {

class ErrorHandler {
public:
    static void sendError(Poco::Net::HTTPServerResponse& response, 
                         Poco::Net::HTTPResponse::HTTPStatus status,
                         const std::string& message);
    
    static void sendJsonResponse(Poco::Net::HTTPServerResponse& response,
                                Poco::Net::HTTPResponse::HTTPStatus status,
                                const Poco::JSON::Object& data);
};

} // namespace medical::shared
