#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <string>

namespace medical::presentation {

class StaticFileHandler : public Poco::Net::HTTPRequestHandler {
private:
    std::string basePath_;
    
public:
    explicit StaticFileHandler(const std::string& basePath);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                      Poco::Net::HTTPServerResponse& response) override;
};

} // namespace medical::presentation
