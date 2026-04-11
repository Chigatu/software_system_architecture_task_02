#pragma once
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <memory>
#include "../application/services/UserService.h"
#include "../application/services/PatientService.h"
#include "../application/services/MedicalRecordService.h"
#include "../application/services/AuthService.h"
#include "../infrastructure/security/JWTService.h"

namespace medical::presentation {

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
private:
    std::shared_ptr<application::UserService> userService_;
    std::shared_ptr<application::PatientService> patientService_;
    std::shared_ptr<application::MedicalRecordService> medicalRecordService_;
    std::shared_ptr<application::AuthService> authService_;
    std::shared_ptr<infrastructure::JWTService> jwtService_;
    
public:
    RequestHandlerFactory(
        std::shared_ptr<application::UserService> userService,
        std::shared_ptr<application::PatientService> patientService,
        std::shared_ptr<application::MedicalRecordService> medicalRecordService,
        std::shared_ptr<application::AuthService> authService,
        std::shared_ptr<infrastructure::JWTService> jwtService);
    
    Poco::Net::HTTPRequestHandler* createRequestHandler(
        const Poco::Net::HTTPServerRequest& request) override;
};

} // namespace medical::presentation
