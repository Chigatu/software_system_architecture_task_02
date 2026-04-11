#include "StaticFileHandler.h"
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>
#include <iostream>

namespace medical::presentation {

std::string StaticFileHandler::webPath_ = "./web";

void StaticFileHandler::setWebPath(const std::string& path) {
    webPath_ = path;
    std::cout << "StaticFileHandler web path set to: " << webPath_ << std::endl;
}

StaticFileHandler::StaticFileHandler() {}

void StaticFileHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                     Poco::Net::HTTPServerResponse& response) {
    std::string uri = request.getURI();
    std::string filePath;
    
    if (uri == "/docs" || uri == "/docs/") {
        filePath = webPath_ + "/docs/index.html";
    } else if (uri == "/swagger/openapi.yaml") {
        filePath = webPath_ + "/swagger/openapi.yaml";
    } else if (uri.find("/docs/") == 0) {
        filePath = webPath_ + uri;
    } else {
        filePath = webPath_ + uri;
    }
    
    try {
        Poco::File file(filePath);
        if (!file.exists() || !file.isFile()) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
            response.setContentType("text/plain");
            std::ostream& out = response.send();
            out << "File not found: " << uri;
            return;
        }
        
        std::string ext = Poco::Path(filePath).getExtension();
        std::string mimeType = "application/octet-stream";
        if (ext == "html") mimeType = "text/html";
        else if (ext == "css") mimeType = "text/css";
        else if (ext == "js") mimeType = "application/javascript";
        else if (ext == "yaml") mimeType = "text/yaml";
        else if (ext == "json") mimeType = "application/json";
        
        response.setContentType(mimeType);
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        
        Poco::FileInputStream fis(filePath);
        std::ostream& out = response.send();
        out << fis.rdbuf();
        
    } catch (const std::exception& e) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << "Error: " << e.what();
    }
}

} // namespace medical::presentation
