#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <memory>
#include "../../application/services/MedicalRecordService.h"
#include "../../infrastructure/security/JWTService.h"

namespace medical::presentation {

class CreateMedicalRecordHandler : public Poco::Net::HTTPRequestHandler {
private:
    std::shared_ptr<application::MedicalRecordService> recordService_;
    std::shared_ptr<infrastructure::JWTService> jwtService_;
    
public:
    CreateMedicalRecordHandler(
        std::shared_ptr<application::MedicalRecordService> recordService,
        std::shared_ptr<infrastructure::JWTService> jwtService);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                      Poco::Net::HTTPServerResponse& response) override;
};

class GetPatientRecordsHandler : public Poco::Net::HTTPRequestHandler {
private:
    std::shared_ptr<application::MedicalRecordService> recordService_;
    std::shared_ptr<infrastructure::JWTService> jwtService_;
    
public:
    GetPatientRecordsHandler(
        std::shared_ptr<application::MedicalRecordService> recordService,
        std::shared_ptr<infrastructure::JWTService> jwtService);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                      Poco::Net::HTTPServerResponse& response) override;
};

class GetRecordByCodeHandler : public Poco::Net::HTTPRequestHandler {
private:
    std::shared_ptr<application::MedicalRecordService> recordService_;
    std::shared_ptr<infrastructure::JWTService> jwtService_;
    
public:
    GetRecordByCodeHandler(
        std::shared_ptr<application::MedicalRecordService> recordService,
        std::shared_ptr<infrastructure::JWTService> jwtService);
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                      Poco::Net::HTTPServerResponse& response) override;
};

} // namespace medical::presentation
