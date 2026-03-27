#pragma once
#include "../domain/MedicalRecord.h"
#include "../domain/interfaces/IRepository.h"
#include <memory>
#include <vector>
#include <optional>

namespace medical::application {

struct CreateMedicalRecordRequest {
    int patientId;
    int doctorId;
    std::string diagnosisCode;
    std::string diagnosisDescription;
    std::string complaints;
};

struct MedicalRecordResponse {
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

class MedicalRecordService {
private:
    std::shared_ptr<domain::IMedicalRecordRepository> recordRepository_;
    std::shared_ptr<domain::IPatientRepository> patientRepository_;
    std::shared_ptr<domain::IUserRepository> userRepository_;
    
public:
    MedicalRecordService(std::shared_ptr<domain::IMedicalRecordRepository> recordRepo,
                         std::shared_ptr<domain::IPatientRepository> patientRepo,
                         std::shared_ptr<domain::IUserRepository> userRepo);
    
    MedicalRecordResponse createRecord(const CreateMedicalRecordRequest& request);
    std::vector<MedicalRecordResponse> getPatientRecords(int patientId);
    std::optional<MedicalRecordResponse> getRecordByCode(const std::string& code);
    
private:
    MedicalRecordResponse mapToResponse(const domain::MedicalRecord& record);
    std::string formatTime(const std::chrono::system_clock::time_point& tp);
};

} // namespace medical::application
