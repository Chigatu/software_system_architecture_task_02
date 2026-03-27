#pragma once
#include "../domain/Patient.h"
#include "../domain/interfaces/IRepository.h"
#include <memory>
#include <vector>
#include <optional>
#include <chrono>

namespace medical::application {

struct CreatePatientRequest {
    std::string firstName;
    std::string lastName;
    std::string patronymic;
    std::string phone;
    std::string address;
    std::string birthDate; // ISO format: YYYY-MM-DD
    std::string snils;
    std::string policyNumber;
    std::optional<int> userId;
};

struct PatientResponse {
    int id;
    std::string fullName;
    std::string phone;
    std::string address;
    int age;
    std::string snils;
    std::string policyNumber;
};

class PatientService {
private:
    std::shared_ptr<domain::IPatientRepository> patientRepository_;
    std::shared_ptr<domain::IUserRepository> userRepository_;
    
    std::chrono::system_clock::time_point parseDate(const std::string& dateStr);
    
public:
    PatientService(std::shared_ptr<domain::IPatientRepository> patientRepo,
                   std::shared_ptr<domain::IUserRepository> userRepo);
    
    PatientResponse createPatient(const CreatePatientRequest& request);
    std::optional<PatientResponse> getPatientById(int id);
    std::vector<PatientResponse> searchPatientsByFullName(const std::string& mask);
    std::optional<PatientResponse> getPatientByUserId(int userId);
    
private:
    PatientResponse mapToResponse(const domain::Patient& patient);
};

} // namespace medical::application
