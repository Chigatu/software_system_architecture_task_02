#include "RequestHandlerFactory.h"
#include "UserHandler.h"
#include "PatientHandler.h"
#include "MedicalRecordHandler.h"
#include "AuthHandler.h"
#include "../application/UserService.h"
#include "../application/PatientService.h"
#include "../application/MedicalRecordService.h"

namespace medical::presentation {

RequestHandlerFactory::RequestHandlerFactory(
    std::shared_ptr<application::UserService> userService,
    std::shared_ptr<application::PatientService> patientService,
    std::shared_ptr<application::MedicalRecordService> medicalRecordService)
    : userService_(userService)
    , patientService_(patientService)
    , medicalRecordService_(medicalRecordService) {}

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest& request) {
    
    std::string uri = request.getURI();
    std::string method = request.getMethod();
    
    // Health check
    if (uri == "/health" || uri == "/") {
        return new HealthHandler();
    }
    
    // Auth endpoints
    if (uri == "/api/auth/register" && method == "POST") {
        return new RegisterHandler(userService_);
    }
    if (uri == "/api/auth/login" && method == "POST") {
        return new LoginHandler(userService_);
    }
    
    // User endpoints
    if (uri == "/api/users/search" && method == "GET") {
        return new UserSearchHandler(userService_);
    }
    
    // Patient endpoints
    if (uri == "/api/patients" && method == "POST") {
        return new CreatePatientHandler(patientService_);
    }
    if (uri == "/api/patients/search" && method == "GET") {
        return new SearchPatientHandler(patientService_);
    }
    
    // Medical record endpoints
    if (uri == "/api/medical-records" && method == "POST") {
        return new CreateMedicalRecordHandler(medicalRecordService_);
    }
    if (uri.find("/api/medical-records/patient/") == 0 && method == "GET") {
        return new GetPatientRecordsHandler(medicalRecordService_);
    }
    if (uri.find("/api/medical-records/") == 0 && method == "GET") {
        return new GetRecordByCodeHandler(medicalRecordService_);
    }
    
    return nullptr; // 404 will be handled
}

} // namespace medical::presentation
