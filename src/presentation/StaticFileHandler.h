#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <string>

namespace medical::presentation {

class StaticFileHandler : public Poco::Net::HTTPRequestHandler {
private:
    static std::string webPath_;
    
public:
    static void setWebPath(const std::string& path);
    explicit StaticFileHandler();
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                      Poco::Net::HTTPServerResponse& response) override;
};

} // namespace medical::presentation
