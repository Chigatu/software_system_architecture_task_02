#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <memory>
#include "../../application/services/PatientService.h"
#include "../../infrastructure/security/JWTService.h"

namespace medical::presentation {

class CreatePatientHandler : public Poco::Net::HTTPRequestHandler {
private:
    std::shared_ptr<application::PatientService> patientService_;
    std::shared_ptr<infrastructure::JWTService> jwtService_;
    
public:
    CreatePatientHandler(std::shared_ptr<application::PatientService> patientService,
                        std::shared_ptr<infrastructure::JWTService> jwtService);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                      Poco::Net::HTTPServerResponse& response) override;
};

class SearchPatientHandler : public Poco::Net::HTTPRequestHandler {
private:
    std::shared_ptr<application::PatientService> patientService_;
    
public:
    explicit SearchPatientHandler(std::shared_ptr<application::PatientService> patientService);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                      Poco::Net::HTTPServerResponse& response) override;
};

} // namespace medical::presentation
