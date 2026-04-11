#include "ErrorHandler.h"

namespace medical::shared {

void ErrorHandler::sendError(Poco::Net::HTTPServerResponse& response,
                            Poco::Net::HTTPResponse::HTTPStatus status,
                            const std::string& message) {
    response.setStatus(status);
    response.setContentType("application/json");
    
    Poco::JSON::Object error;
    error.set("error", message);
    
    std::ostream& out = response.send();
    error.stringify(out);
}

void ErrorHandler::sendJsonResponse(Poco::Net::HTTPServerResponse& response,
                                   Poco::Net::HTTPResponse::HTTPStatus status,
                                   const Poco::JSON::Object& data) {
    response.setStatus(status);
    response.setContentType("application/json");
    
    std::ostream& out = response.send();
    data.stringify(out);
}

} // namespace medical::shared
