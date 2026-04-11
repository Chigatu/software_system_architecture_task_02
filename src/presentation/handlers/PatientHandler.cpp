#include "PatientHandler.h"
#include "../../shared/ErrorHandler.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/URI.h>
#include <sstream>
#include <iostream>

namespace medical::presentation {

CreatePatientHandler::CreatePatientHandler(
    std::shared_ptr<application::PatientService> patientService,
    std::shared_ptr<infrastructure::JWTService> jwtService)
    : patientService_(patientService)
    , jwtService_(jwtService) {}

void CreatePatientHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                        Poco::Net::HTTPServerResponse& response) {
    std::cout << "=== CreatePatientHandler ===" << std::endl;
    
    // Проверяем авторизацию
    std::string auth = request.get("Authorization", "");
    std::cout << "Authorization header: '" << auth << "'" << std::endl;
    
    if (auth.empty() || auth.find("Bearer ") != 0) {
        std::cout << "Missing or invalid Authorization header" << std::endl;
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
            "Missing or invalid token");
        return;
    }
    
    std::string token = auth.substr(7);
    std::cout << "Token: " << token << std::endl;
    
    auto userIdOpt = jwtService_->validateToken(token);
    if (!userIdOpt.has_value()) {
        std::cout << "Token validation failed" << std::endl;
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
            "Invalid token");
        return;
    }
    
    int userId = userIdOpt.value();
    std::cout << "Token valid for userId: " << userId << std::endl;
    
    try {
        std::string body;
        std::istream& in = request.stream();
        std::string line;
        while (std::getline(in, line)) {
            body += line + "\n";
        }
        
        std::cout << "Request body: " << body << std::endl;
        
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(body);
        Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
        
        application::dto::CreatePatientRequestDTO dto;
        dto.firstName = json->getValue<std::string>("firstName");
        dto.lastName = json->getValue<std::string>("lastName");
        dto.patronymic = json->optValue<std::string>("patronymic", "");
        dto.phone = json->getValue<std::string>("phone");
        dto.address = json->getValue<std::string>("address");
        dto.birthDate = json->getValue<std::string>("birthDate");
        dto.snils = json->getValue<std::string>("snils");
        dto.policyNumber = json->getValue<std::string>("policyNumber");
        dto.userId = userId;
        
        std::cout << "Creating patient for userId: " << userId << std::endl;
        
        auto patient = patientService_->createPatient(dto);
        if (!patient.has_value()) {
            std::cout << "Failed to create patient" << std::endl;
            shared::ErrorHandler::sendError(response, 
                Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                "Patient with this SNILS already exists or invalid data");
            return;
        }
        
        std::cout << "Patient created with ID: " << patient->id << std::endl;
        
        Poco::JSON::Object responseObj;
        responseObj.set("id", patient->id);
        responseObj.set("fullName", patient->fullName);
        responseObj.set("phone", patient->phone);
        responseObj.set("address", patient->address);
        responseObj.set("age", patient->age);
        responseObj.set("snils", patient->snils);
        responseObj.set("policyNumber", patient->policyNumber);
        
        shared::ErrorHandler::sendJsonResponse(response, 
            Poco::Net::HTTPResponse::HTTP_CREATED, responseObj);
            
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, e.what());
    }
}

SearchPatientHandler::SearchPatientHandler(
    std::shared_ptr<application::PatientService> patientService)
    : patientService_(patientService) {}

void SearchPatientHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                        Poco::Net::HTTPServerResponse& response) {
    try {
        Poco::URI uri(request.getURI());
        auto params = uri.getQueryParameters();
        
        std::string fullName;
        for (const auto& param : params) {
            if (param.first == "fullName") {
                fullName = param.second;
                break;
            }
        }
        
        if (fullName.empty()) {
            shared::ErrorHandler::sendError(response, 
                Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                "Missing fullName parameter");
            return;
        }
        
        auto patients = patientService_->searchPatientsByName(fullName);
        
        Poco::JSON::Array patientsArray;
        for (const auto& patient : patients) {
            Poco::JSON::Object patientObj;
            patientObj.set("id", patient.id);
            patientObj.set("fullName", patient.fullName);
            patientObj.set("phone", patient.phone);
            patientObj.set("address", patient.address);
            patientObj.set("age", patient.age);
            patientObj.set("snils", patient.snils);
            patientsArray.add(patientObj);
        }
        
        Poco::JSON::Object responseObj;
        responseObj.set("patients", patientsArray);
        
        shared::ErrorHandler::sendJsonResponse(response, 
            Poco::Net::HTTPResponse::HTTP_OK, responseObj);
            
    } catch (const std::exception& e) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, e.what());
    }
}

} // namespace medical::presentation
