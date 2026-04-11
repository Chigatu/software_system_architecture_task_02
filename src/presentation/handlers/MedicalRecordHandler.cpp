#include "MedicalRecordHandler.h"
#include "../../shared/ErrorHandler.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace medical::presentation {

CreateMedicalRecordHandler::CreateMedicalRecordHandler(
    std::shared_ptr<application::MedicalRecordService> recordService,
    std::shared_ptr<infrastructure::JWTService> jwtService)
    : recordService_(recordService)
    , jwtService_(jwtService) {}

void CreateMedicalRecordHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                              Poco::Net::HTTPServerResponse& response) {
    // Проверяем авторизацию
    std::string auth = request.get("Authorization", "");
    std::cout << "Auth header: '" << auth << "'" << std::endl;
    
    if (auth.empty() || auth.find("Bearer ") != 0) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
            "Missing or invalid token");
        return;
    }
    
    std::string token = auth.substr(7);
    std::cout << "Extracted token: " << token.substr(0, 20) << "..." << std::endl;
    
    auto userId = jwtService_->validateToken(token);
    if (!userId.has_value()) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
            "Invalid token");
        return;
    }
    
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
        
        application::dto::CreateMedicalRecordRequestDTO dto;
        dto.patientId = json->getValue<int>("patientId");
        dto.doctorId = json->getValue<int>("doctorId");
        
        // Триммим строки
        std::string diagCode = json->getValue<std::string>("diagnosisCode");
        std::string diagDesc = json->getValue<std::string>("diagnosisDescription");
        std::string complaints = json->getValue<std::string>("complaints");
        
        // Убираем пробелы и кавычки
        diagCode.erase(std::remove_if(diagCode.begin(), diagCode.end(), ::isspace), diagCode.end());
        
        dto.diagnosisCode = diagCode;
        dto.diagnosisDescription = diagDesc;
        dto.complaints = complaints;
        
        std::cout << "Parsed - code: '" << dto.diagnosisCode << "', desc: '" << dto.diagnosisDescription << "'" << std::endl;
        
        auto record = recordService_->createRecord(dto);
        if (!record.has_value()) {
            shared::ErrorHandler::sendError(response, 
                Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                "Invalid patient or doctor ID");
            return;
        }
        
        Poco::JSON::Object responseObj;
        responseObj.set("id", record->id);
        responseObj.set("code", record->code);
        responseObj.set("patientId", record->patientId);
        responseObj.set("doctorId", record->doctorId);
        responseObj.set("diagnosisCode", record->diagnosisCode);
        responseObj.set("diagnosisDescription", record->diagnosisDescription);
        responseObj.set("complaints", record->complaints);
        responseObj.set("createdAt", record->createdAt);
        
        shared::ErrorHandler::sendJsonResponse(response, 
            Poco::Net::HTTPResponse::HTTP_CREATED, responseObj);
            
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, e.what());
    }
}

GetPatientRecordsHandler::GetPatientRecordsHandler(
    std::shared_ptr<application::MedicalRecordService> recordService,
    std::shared_ptr<infrastructure::JWTService> jwtService)
    : recordService_(recordService)
    , jwtService_(jwtService) {}

void GetPatientRecordsHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                            Poco::Net::HTTPServerResponse& response) {
    std::string auth = request.get("Authorization", "");
    if (auth.empty() || auth.find("Bearer ") != 0) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
            "Missing or invalid token");
        return;
    }
    
    std::string token = auth.substr(7);
    auto userId = jwtService_->validateToken(token);
    if (!userId.has_value()) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
            "Invalid token");
        return;
    }
    
    try {
        std::string uri = request.getURI();
        size_t lastSlash = uri.find_last_of('/');
        if (lastSlash == std::string::npos) {
            shared::ErrorHandler::sendError(response, 
                Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "Invalid URI");
            return;
        }
        
        int patientId = std::stoi(uri.substr(lastSlash + 1));
        auto records = recordService_->getPatientRecords(patientId);
        
        Poco::JSON::Array recordsArray;
        for (const auto& record : records) {
            Poco::JSON::Object recordObj;
            recordObj.set("id", record.id);
            recordObj.set("code", record.code);
            recordObj.set("diagnosisCode", record.diagnosisCode);
            recordObj.set("diagnosisDescription", record.diagnosisDescription);
            recordObj.set("complaints", record.complaints);
            recordObj.set("createdAt", record.createdAt);
            recordsArray.add(recordObj);
        }
        
        Poco::JSON::Object responseObj;
        responseObj.set("records", recordsArray);
        
        shared::ErrorHandler::sendJsonResponse(response, 
            Poco::Net::HTTPResponse::HTTP_OK, responseObj);
            
    } catch (const std::exception& e) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, e.what());
    }
}

GetRecordByCodeHandler::GetRecordByCodeHandler(
    std::shared_ptr<application::MedicalRecordService> recordService,
    std::shared_ptr<infrastructure::JWTService> jwtService)
    : recordService_(recordService)
    , jwtService_(jwtService) {}

void GetRecordByCodeHandler::handleRequest(Poco::Net::HTTPServerRequest& request,
                                          Poco::Net::HTTPServerResponse& response) {
    std::string auth = request.get("Authorization", "");
    if (auth.empty() || auth.find("Bearer ") != 0) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
            "Missing or invalid token");
        return;
    }
    
    std::string token = auth.substr(7);
    auto userId = jwtService_->validateToken(token);
    if (!userId.has_value()) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
            "Invalid token");
        return;
    }
    
    try {
        std::string uri = request.getURI();
        size_t lastSlash = uri.find_last_of('/');
        if (lastSlash == std::string::npos) {
            shared::ErrorHandler::sendError(response, 
                Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "Invalid URI");
            return;
        }
        
        std::string code = uri.substr(lastSlash + 1);
        auto record = recordService_->findByCode(code);
        if (!record.has_value()) {
            shared::ErrorHandler::sendError(response, 
                Poco::Net::HTTPResponse::HTTP_NOT_FOUND, "Record not found");
            return;
        }
        
        Poco::JSON::Object responseObj;
        responseObj.set("id", record->id);
        responseObj.set("code", record->code);
        responseObj.set("patientId", record->patientId);
        responseObj.set("doctorId", record->doctorId);
        responseObj.set("diagnosisCode", record->diagnosisCode);
        responseObj.set("diagnosisDescription", record->diagnosisDescription);
        responseObj.set("complaints", record->complaints);
        responseObj.set("createdAt", record->createdAt);
        
        shared::ErrorHandler::sendJsonResponse(response, 
            Poco::Net::HTTPResponse::HTTP_OK, responseObj);
            
    } catch (const std::exception& e) {
        shared::ErrorHandler::sendError(response, 
            Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, e.what());
    }
}

} // namespace medical::presentation
