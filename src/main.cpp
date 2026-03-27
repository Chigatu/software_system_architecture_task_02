#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <random>

using namespace Poco::Net;
using namespace Poco::JSON;

// ============== Domain Models ==============
struct User {
    int id;
    std::string login;
    std::string email;
    std::string firstName;
    std::string lastName;
    std::string patronymic;
    std::string passwordHash;
    std::string token;
    bool isActive;
    
    std::string fullName() const {
        if (patronymic.empty()) {
            return lastName + " " + firstName;
        }
        return lastName + " " + firstName + " " + patronymic;
    }
    
    bool matchesNameMask(const std::string& mask) const {
        std::string name = fullName();
        return name.find(mask) != std::string::npos;
    }
};

struct Patient {
    int id;
    std::string firstName;
    std::string lastName;
    std::string patronymic;
    std::string phone;
    std::string address;
    std::string birthDate;
    std::string snils;
    std::string policyNumber;
    int userId;
    
    std::string fullName() const {
        if (patronymic.empty()) {
            return lastName + " " + firstName;
        }
        return lastName + " " + firstName + " " + patronymic;
    }
    
    bool matchesNameMask(const std::string& mask) const {
        std::string name = fullName();
        return name.find(mask) != std::string::npos;
    }
    
    int age() const {
        return 30; // Simplified
    }
};

struct MedicalRecord {
    int id;
    std::string code;
    int patientId;
    int doctorId;
    std::string diagnosisCode;
    std::string diagnosisDescription;
    std::string complaints;
    std::string recommendations;
    std::string createdAt;
};

// ============== In-Memory Storage ==============
std::map<int, User> users;
std::map<int, Patient> patients;
std::map<int, MedicalRecord> medicalRecords;
std::map<std::string, int> validTokens;
int nextUserId = 1;
int nextPatientId = 1;
int nextRecordId = 1;

// Helper functions
std::string hashPassword(const std::string& password) {
    std::string salt = "medical_salt_2024";
    std::string combined = password + salt;
    size_t hash = std::hash<std::string>{}(combined);
    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

bool verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

std::string generateToken(int userId, const std::string& login) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    auto now = std::time(nullptr);
    std::stringstream ss;
    ss << userId << "_" << login << "_" << now << "_" << dis(gen);
    return hashPassword(ss.str());
}

bool validateToken(const std::string& token, int& userId) {
    auto it = validTokens.find(token);
    if (it != validTokens.end()) {
        userId = it->second;
        return true;
    }
    return false;
}

bool authenticateRequest(HTTPServerRequest& request, int& userId) {
    std::string auth = request.get("Authorization", "");
    if (auth.empty() || auth.find("Bearer ") != 0) {
        return false;
    }
    std::string token = auth.substr(7);
    return validateToken(token, userId);
}

std::string readRequestBody(HTTPServerRequest& request) {
    std::string body;
    std::istream& in = request.stream();
    std::string line;
    while (std::getline(in, line)) {
        body += line + "\n";
    }
    return body;
}

void sendJsonResponse(HTTPServerResponse& response, HTTPResponse::HTTPStatus status, const Object& data) {
    response.setStatus(status);
    response.setContentType("application/json");
    std::ostream& out = response.send();
    data.stringify(out);
}

void sendErrorResponse(HTTPServerResponse& response, HTTPResponse::HTTPStatus status, const std::string& message) {
    Object error;
    error.set("error", message);
    sendJsonResponse(response, status, error);
}

// ============== Handlers ==============
class HealthHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        Object result;
        result.set("status", "ok");
        result.set("service", "Medical Records API");
        result.set("version", "2.0.0");
        sendJsonResponse(response, HTTPResponse::HTTP_OK, result);
    }
};

class RegisterHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        try {
            std::string body = readRequestBody(request);
            Parser parser;
            Object::Ptr json = parser.parse(body).extract<Object::Ptr>();
            
            std::string login = json->getValue<std::string>("login");
            for (const auto& pair : users) {
                if (pair.second.login == login) {
                    sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, "User already exists");
                    return;
                }
            }
            
            User user;
            user.id = nextUserId++;
            user.login = login;
            user.email = json->getValue<std::string>("email");
            user.firstName = json->getValue<std::string>("firstName");
            user.lastName = json->getValue<std::string>("lastName");
            user.patronymic = json->optValue<std::string>("patronymic", "");
            user.passwordHash = hashPassword(json->getValue<std::string>("password"));
            user.isActive = true;
            
            users[user.id] = user;
            
            Object responseObj;
            responseObj.set("id", user.id);
            responseObj.set("login", user.login);
            responseObj.set("email", user.email);
            responseObj.set("fullName", user.fullName());
            
            sendJsonResponse(response, HTTPResponse::HTTP_CREATED, responseObj);
        } catch (const std::exception& e) {
            sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, e.what());
        }
    }
};

class LoginHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        try {
            std::string body = readRequestBody(request);
            Parser parser;
            Object::Ptr json = parser.parse(body).extract<Object::Ptr>();
            
            std::string login = json->getValue<std::string>("login");
            std::string password = json->getValue<std::string>("password");
            
            User* foundUser = nullptr;
            for (auto& pair : users) {
                if (pair.second.login == login) {
                    foundUser = &pair.second;
                    break;
                }
            }
            
            if (!foundUser || !verifyPassword(password, foundUser->passwordHash)) {
                sendErrorResponse(response, HTTPResponse::HTTP_UNAUTHORIZED, "Invalid credentials");
                return;
            }
            
            std::string token = generateToken(foundUser->id, foundUser->login);
            validTokens[token] = foundUser->id;
            
            Object responseObj;
            responseObj.set("token", token);
            Object userObj;
            userObj.set("id", foundUser->id);
            userObj.set("login", foundUser->login);
            userObj.set("email", foundUser->email);
            userObj.set("fullName", foundUser->fullName());
            responseObj.set("user", userObj);
            
            sendJsonResponse(response, HTTPResponse::HTTP_OK, responseObj);
        } catch (const std::exception& e) {
            sendErrorResponse(response, HTTPResponse::HTTP_UNAUTHORIZED, e.what());
        }
    }
};

class UserSearchHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        try {
            std::string uri = request.getURI();
            size_t pos = uri.find("mask=");
            if (pos == std::string::npos) {
                sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, "Missing mask parameter");
                return;
            }
            
            std::string mask = uri.substr(pos + 5);
            
            Array arr;
            for (const auto& pair : users) {
                if (pair.second.matchesNameMask(mask)) {
                    Object obj;
                    obj.set("id", pair.second.id);
                    obj.set("login", pair.second.login);
                    obj.set("email", pair.second.email);
                    obj.set("fullName", pair.second.fullName());
                    arr.add(obj);
                }
            }
            
            Object responseObj;
            responseObj.set("users", arr);
            sendJsonResponse(response, HTTPResponse::HTTP_OK, responseObj);
        } catch (const std::exception& e) {
            sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, e.what());
        }
    }
};

class CreatePatientHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        int userId;
        if (!authenticateRequest(request, userId)) {
            sendErrorResponse(response, HTTPResponse::HTTP_UNAUTHORIZED, "Invalid or missing token");
            return;
        }
        
        try {
            std::string body = readRequestBody(request);
            Parser parser;
            Object::Ptr json = parser.parse(body).extract<Object::Ptr>();
            
            std::string snils = json->getValue<std::string>("snils");
            for (const auto& pair : patients) {
                if (pair.second.snils == snils) {
                    sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, "Patient with this SNILS already exists");
                    return;
                }
            }
            
            Patient patient;
            patient.id = nextPatientId++;
            patient.firstName = json->getValue<std::string>("firstName");
            patient.lastName = json->getValue<std::string>("lastName");
            patient.patronymic = json->optValue<std::string>("patronymic", "");
            patient.phone = json->getValue<std::string>("phone");
            patient.address = json->getValue<std::string>("address");
            patient.birthDate = json->getValue<std::string>("birthDate");
            patient.snils = snils;
            patient.policyNumber = json->getValue<std::string>("policyNumber");
            patient.userId = json->optValue<int>("userId", 0);
            
            patients[patient.id] = patient;
            
            Object responseObj;
            responseObj.set("id", patient.id);
            responseObj.set("fullName", patient.fullName());
            responseObj.set("phone", patient.phone);
            responseObj.set("address", patient.address);
            responseObj.set("age", patient.age());
            responseObj.set("snils", patient.snils);
            responseObj.set("policyNumber", patient.policyNumber);
            
            sendJsonResponse(response, HTTPResponse::HTTP_CREATED, responseObj);
        } catch (const std::exception& e) {
            sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, e.what());
        }
    }
};

class SearchPatientHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        try {
            std::string uri = request.getURI();
            size_t pos = uri.find("fullName=");
            if (pos == std::string::npos) {
                sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, "Missing fullName parameter");
                return;
            }
            
            std::string fullName = uri.substr(pos + 9);
            
            Array arr;
            for (const auto& pair : patients) {
                if (pair.second.matchesNameMask(fullName)) {
                    Object obj;
                    obj.set("id", pair.second.id);
                    obj.set("fullName", pair.second.fullName());
                    obj.set("phone", pair.second.phone);
                    obj.set("address", pair.second.address);
                    obj.set("age", pair.second.age());
                    obj.set("snils", pair.second.snils);
                    arr.add(obj);
                }
            }
            
            Object responseObj;
            responseObj.set("patients", arr);
            sendJsonResponse(response, HTTPResponse::HTTP_OK, responseObj);
        } catch (const std::exception& e) {
            sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, e.what());
        }
    }
};

class CreateMedicalRecordHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        int userId;
        if (!authenticateRequest(request, userId)) {
            sendErrorResponse(response, HTTPResponse::HTTP_UNAUTHORIZED, "Invalid or missing token");
            return;
        }
        
        try {
            std::string body = readRequestBody(request);
            Parser parser;
            Object::Ptr json = parser.parse(body).extract<Object::Ptr>();
            
            int patientId = json->getValue<int>("patientId");
            int doctorId = json->getValue<int>("doctorId");
            
            if (patients.find(patientId) == patients.end()) {
                sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, "Patient not found");
                return;
            }
            
            if (users.find(doctorId) == users.end()) {
                sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, "Doctor not found");
                return;
            }
            
            auto now = std::time(nullptr);
            std::tm* tm = std::localtime(&now);
            char date[9];
            std::strftime(date, sizeof(date), "%Y%m%d", tm);
            static int counter = 0;
            char code[20];
            std::snprintf(code, sizeof(code), "MED-%s-%05d", date, ++counter);
            
            MedicalRecord record;
            record.id = nextRecordId++;
            record.code = code;
            record.patientId = patientId;
            record.doctorId = doctorId;
            record.diagnosisCode = json->getValue<std::string>("diagnosisCode");
            record.diagnosisDescription = json->getValue<std::string>("diagnosisDescription");
            record.complaints = json->getValue<std::string>("complaints");
            record.recommendations = "";
            
            char timeStr[20];
            std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm);
            record.createdAt = timeStr;
            
            medicalRecords[record.id] = record;
            
            Object responseObj;
            responseObj.set("id", record.id);
            responseObj.set("code", record.code);
            responseObj.set("patientId", record.patientId);
            responseObj.set("doctorId", record.doctorId);
            responseObj.set("diagnosisCode", record.diagnosisCode);
            responseObj.set("diagnosisDescription", record.diagnosisDescription);
            responseObj.set("complaints", record.complaints);
            responseObj.set("createdAt", record.createdAt);
            
            sendJsonResponse(response, HTTPResponse::HTTP_CREATED, responseObj);
        } catch (const std::exception& e) {
            sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, e.what());
        }
    }
};

class GetPatientRecordsHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        int userId;
        if (!authenticateRequest(request, userId)) {
            sendErrorResponse(response, HTTPResponse::HTTP_UNAUTHORIZED, "Invalid or missing token");
            return;
        }
        
        try {
            std::string uri = request.getURI();
            size_t lastSlash = uri.find_last_of('/');
            if (lastSlash == std::string::npos) {
                sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, "Invalid URI");
                return;
            }
            
            int patientId = std::stoi(uri.substr(lastSlash + 1));
            
            Array arr;
            for (const auto& pair : medicalRecords) {
                if (pair.second.patientId == patientId) {
                    Object obj;
                    obj.set("id", pair.second.id);
                    obj.set("code", pair.second.code);
                    obj.set("diagnosisCode", pair.second.diagnosisCode);
                    obj.set("diagnosisDescription", pair.second.diagnosisDescription);
                    obj.set("complaints", pair.second.complaints);
                    obj.set("createdAt", pair.second.createdAt);
                    arr.add(obj);
                }
            }
            
            Object responseObj;
            responseObj.set("records", arr);
            sendJsonResponse(response, HTTPResponse::HTTP_OK, responseObj);
        } catch (const std::exception& e) {
            sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, e.what());
        }
    }
};

class GetRecordByCodeHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        int userId;
        if (!authenticateRequest(request, userId)) {
            sendErrorResponse(response, HTTPResponse::HTTP_UNAUTHORIZED, "Invalid or missing token");
            return;
        }
        
        try {
            std::string uri = request.getURI();
            size_t lastSlash = uri.find_last_of('/');
            if (lastSlash == std::string::npos) {
                sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, "Invalid URI");
                return;
            }
            
            std::string code = uri.substr(lastSlash + 1);
            
            MedicalRecord* foundRecord = nullptr;
            for (auto& pair : medicalRecords) {
                if (pair.second.code == code) {
                    foundRecord = &pair.second;
                    break;
                }
            }
            
            if (!foundRecord) {
                sendErrorResponse(response, HTTPResponse::HTTP_NOT_FOUND, "Record not found");
                return;
            }
            
            Object responseObj;
            responseObj.set("id", foundRecord->id);
            responseObj.set("code", foundRecord->code);
            responseObj.set("patientId", foundRecord->patientId);
            responseObj.set("doctorId", foundRecord->doctorId);
            responseObj.set("diagnosisCode", foundRecord->diagnosisCode);
            responseObj.set("diagnosisDescription", foundRecord->diagnosisDescription);
            responseObj.set("complaints", foundRecord->complaints);
            responseObj.set("createdAt", foundRecord->createdAt);
            
            sendJsonResponse(response, HTTPResponse::HTTP_OK, responseObj);
        } catch (const std::exception& e) {
            sendErrorResponse(response, HTTPResponse::HTTP_BAD_REQUEST, e.what());
        }
    }
};

// Static file handler for Swagger UI
class StaticFileHandler : public HTTPRequestHandler {
private:
    std::string basePath_;
    
public:
    StaticFileHandler(const std::string& basePath) : basePath_(basePath) {}
    
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        std::string uri = request.getURI();
        
        std::string filePath;
        
        if (uri == "/docs" || uri == "/docs/") {
            filePath = basePath_ + "/docs/index.html";
        } else if (uri == "/swagger/openapi.yaml") {
            filePath = basePath_ + "/swagger/openapi.yaml";
        } else if (uri.find("/docs/") == 0) {
            filePath = basePath_ + uri;
        } else if (uri.find("/swagger/") == 0) {
            filePath = basePath_ + uri;
        } else {
            filePath = basePath_ + uri;
        }
        
        try {
            Poco::File file(filePath);
            if (!file.exists() || !file.isFile()) {
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
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
            response.setStatus(HTTPResponse::HTTP_OK);
            
            Poco::FileInputStream fis(filePath);
            std::ostream& out = response.send();
            out << fis.rdbuf();
            
        } catch (const std::exception& e) {
            response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            response.send() << "Error: " << e.what();
        }
    }
};

class HandlerFactory : public HTTPRequestHandlerFactory {
public:
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) override {
        std::string uri = request.getURI();
        std::string method = request.getMethod();
        
        std::cout << "Request: " << method << " " << uri << std::endl;
        
        // Swagger UI static files
        if (uri == "/docs" || uri == "/docs/" || uri.find("/docs/") == 0 || 
            uri == "/swagger/openapi.yaml" || uri.find("/swagger/") == 0) {
            return new StaticFileHandler("web");
        }
        
        // Health check
        if (uri == "/health" || uri == "/") {
            return new HealthHandler();
        }
        
        // API endpoints
        if (uri == "/api/auth/register" && method == "POST") {
            return new RegisterHandler();
        }
        if (uri == "/api/auth/login" && method == "POST") {
            return new LoginHandler();
        }
        
        if (uri.find("/api/users/search") == 0 && method == "GET") {
            return new UserSearchHandler();
        }
        
        if (uri == "/api/patients" && method == "POST") {
            return new CreatePatientHandler();
        }
        if (uri.find("/api/patients/search") == 0 && method == "GET") {
            return new SearchPatientHandler();
        }
        
        if (uri == "/api/medical-records" && method == "POST") {
            return new CreateMedicalRecordHandler();
        }
        if (uri.find("/api/medical-records/patient/") == 0 && method == "GET") {
            return new GetPatientRecordsHandler();
        }
        if (uri.find("/api/medical-records/") == 0 && method == "GET") {
            if (uri.find("/api/medical-records/patient/") == std::string::npos) {
                return new GetRecordByCodeHandler();
            }
        }
        
        return nullptr;
    }
};

int main() {
    try {
        std::string webPath = "web";
        
        // Проверяем наличие файлов
        std::cout << "========================================" << std::endl;
        std::cout << "Web files path: " << webPath << std::endl;
        
        Poco::File docsFile(webPath + "/docs/index.html");
        if (docsFile.exists()) {
            std::cout << "  ✓ web/docs/index.html exists" << std::endl;
        } else {
            std::cout << "  ✗ web/docs/index.html NOT FOUND!" << std::endl;
            std::cout << "  Creating directory..." << std::endl;
        }
        
        Poco::File swaggerFile(webPath + "/swagger/openapi.yaml");
        if (swaggerFile.exists()) {
            std::cout << "  ✓ web/swagger/openapi.yaml exists" << std::endl;
        } else {
            std::cout << "  ✗ web/swagger/openapi.yaml NOT FOUND!" << std::endl;
        }
        std::cout << "========================================" << std::endl;
        
        HTTPServerParams::Ptr params = new HTTPServerParams();
        params->setMaxQueued(100);
        params->setMaxThreads(16);
        
        HTTPServer server(new HandlerFactory(), 8080, params);
        server.start();
        
        std::cout << "Medical Records API Server started" << std::endl;
        std::cout << "Port: 8080" << std::endl;
        std::cout << "Health check: http://localhost:8080/health" << std::endl;
        std::cout << "Swagger UI: http://localhost:8080/docs" << std::endl;
        std::cout << std::endl;
        std::cout << "Protected endpoints (require Bearer token):" << std::endl;
        std::cout << "  POST   /api/patients" << std::endl;
        std::cout << "  POST   /api/medical-records" << std::endl;
        std::cout << "  GET    /api/medical-records/patient/{id}" << std::endl;
        std::cout << "  GET    /api/medical-records/{code}" << std::endl;
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
