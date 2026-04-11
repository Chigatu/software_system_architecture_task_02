#include "RequestHandlerFactory.h"
#include "handlers/HealthHandler.h"
#include "handlers/NotFoundHandler.h"
#include "handlers/AuthHandler.h"
#include "handlers/UserHandler.h"
#include "handlers/PatientHandler.h"
#include "handlers/MedicalRecordHandler.h"
#include "StaticFileHandler.h"
#include <iostream>

namespace medical::presentation {

RequestHandlerFactory::RequestHandlerFactory(
    std::shared_ptr<application::UserService> userService,
    std::shared_ptr<application::PatientService> patientService,
    std::shared_ptr<application::MedicalRecordService> medicalRecordService,
    std::shared_ptr<application::AuthService> authService,
    std::shared_ptr<infrastructure::JWTService> jwtService)
    : userService_(userService)
    , patientService_(patientService)
    , medicalRecordService_(medicalRecordService)
    , authService_(authService)
    , jwtService_(jwtService) {}

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest& request) {
    
    std::string uri = request.getURI();
    std::string method = request.getMethod();
    
    std::cout << "Request: " << method << " " << uri << std::endl;
    
    // Static files для Swagger UI
    if (uri == "/docs" || uri == "/docs/" || uri.find("/docs/") == 0 || 
        uri == "/swagger/openapi.yaml" || uri.find("/swagger/") == 0) {
        return new StaticFileHandler("web");
    }
    
    // Health check
    if (uri == "/health" || uri == "/") {
        return new HealthHandler();
    }
    
    // Auth endpoints
    if (uri == "/api/auth/register" && method == "POST") {
        return new RegisterHandler(userService_);
    }
    if (uri == "/api/auth/login" && method == "POST") {
        return new LoginHandler(authService_);
    }
    
    // User endpoints
    if (uri.find("/api/users/search") == 0 && method == "GET") {
        return new UserSearchHandler(userService_);
    }
    
    // Patient endpoints
    if (uri == "/api/patients" && method == "POST") {
        return new CreatePatientHandler(patientService_, jwtService_);
    }
    if (uri.find("/api/patients/search") == 0 && method == "GET") {
        return new SearchPatientHandler(patientService_);
    }
    
    // Medical record endpoints
    if (uri == "/api/medical-records" && method == "POST") {
        return new CreateMedicalRecordHandler(medicalRecordService_, jwtService_);
    }
    if (uri.find("/api/medical-records/patient/") == 0 && method == "GET") {
        return new GetPatientRecordsHandler(medicalRecordService_, jwtService_);
    }
    if (uri.find("/api/medical-records/") == 0 && method == "GET") {
        if (uri.find("/api/medical-records/patient/") == std::string::npos) {
            return new GetRecordByCodeHandler(medicalRecordService_, jwtService_);
        }
    }
    
    return new NotFoundHandler();
}

} // namespace medical::presentation
