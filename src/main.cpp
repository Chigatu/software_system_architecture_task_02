#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/File.h>
#include <iostream>
#include <memory>

// Domain
#include "domain/entities/User.h"
#include "domain/entities/Patient.h"
#include "domain/entities/MedicalRecord.h"

// Infrastructure
#include "infrastructure/repositories/InMemoryUserRepository.h"
#include "infrastructure/repositories/InMemoryPatientRepository.h"
#include "infrastructure/repositories/InMemoryMedicalRecordRepository.h"
#include "infrastructure/security/JWTService.h"

// Application
#include "application/services/UserService.h"
#include "application/services/PatientService.h"
#include "application/services/MedicalRecordService.h"
#include "application/services/AuthService.h"

// Presentation
#include "presentation/RequestHandlerFactory.h"

using namespace medical;

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "Medical Records API Server" << std::endl;
        std::cout << "Architecture: Domain-Driven Design" << std::endl;
        std::cout << "========================================" << std::endl;
        
        std::string webPath = "../web";
        Poco::File docsFile(webPath + "/docs/index.html");
        if (docsFile.exists()) {
            std::cout << "  ✓ web/docs/index.html exists" << std::endl;
        } else {
            std::cout << "  ✗ web/docs/index.html NOT FOUND!" << std::endl;
        }
        
        Poco::File swaggerFile(webPath + "/swagger/openapi.yaml");
        if (swaggerFile.exists()) {
            std::cout << "  ✓ web/swagger/openapi.yaml exists" << std::endl;
        } else {
            std::cout << "  ✗ web/swagger/openapi.yaml NOT FOUND!" << std::endl;
        }
        std::cout << "========================================" << std::endl;
        
        // Infrastructure layer
        std::cout << "Initializing Infrastructure layer..." << std::endl;
        auto userRepository = std::make_shared<infrastructure::InMemoryUserRepository>();
        auto patientRepository = std::make_shared<infrastructure::InMemoryPatientRepository>();
        auto recordRepository = std::make_shared<infrastructure::InMemoryMedicalRecordRepository>();
        auto jwtService = std::make_shared<infrastructure::JWTService>();
        
        // Application layer
        std::cout << "Initializing Application layer..." << std::endl;
        auto userService = std::make_shared<application::UserService>(userRepository);
        auto patientService = std::make_shared<application::PatientService>(patientRepository);
        auto medicalRecordService = std::make_shared<application::MedicalRecordService>(
            recordRepository, patientRepository, userRepository);
        auto authService = std::make_shared<application::AuthService>(userService, jwtService);
        
        // Presentation layer
        std::cout << "Initializing Presentation layer..." << std::endl;
        presentation::RequestHandlerFactory handlerFactory(
            userService, patientService, medicalRecordService, authService, jwtService);
        
        // Server setup
        Poco::Net::HTTPServerParams::Ptr params = new Poco::Net::HTTPServerParams();
        params->setMaxQueued(100);
        params->setMaxThreads(16);
        
        Poco::Net::HTTPServer server(&handlerFactory, 8080, params);
        server.start();
        
        std::cout << "========================================" << std::endl;
        std::cout << "Server started successfully!" << std::endl;
        std::cout << "Port: 8080" << std::endl;
        std::cout << "Health check: http://localhost:8080/health" << std::endl;
        std::cout << "Swagger UI: http://localhost:8080/docs" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;
        
        while (true) {
            Poco::Thread::sleep(1000);
        }
        
        server.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
