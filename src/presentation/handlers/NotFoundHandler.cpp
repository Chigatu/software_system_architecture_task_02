#include "NotFoundHandler.h"
#include <Poco/JSON/Object.h>

namespace medical::presentation {

void NotFoundHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                    Poco::Net::HTTPServerResponse& response) {
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.setContentType("application/json");
    
    Poco::JSON::Object error;
    error.set("error", "Endpoint not found");
    error.set("path", request.getURI());
    error.set("method", request.getMethod());
    
    std::ostream& out = response.send();
    error.stringify(out);
}

} // namespace medical::presentation
