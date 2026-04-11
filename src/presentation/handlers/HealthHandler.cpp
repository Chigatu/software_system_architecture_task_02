#include "HealthHandler.h"
#include "../../shared/ErrorHandler.h"
#include <Poco/JSON/Object.h>

namespace medical::presentation {

void HealthHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                  Poco::Net::HTTPServerResponse& response) {
    Poco::JSON::Object result;
    result.set("status", "ok");
    result.set("service", "Medical Records API");
    result.set("version", "2.0.0");
    
    shared::ErrorHandler::sendJsonResponse(response, 
        Poco::Net::HTTPResponse::HTTP_OK, result);
}

} // namespace medical::presentation
