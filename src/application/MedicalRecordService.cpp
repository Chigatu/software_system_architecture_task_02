#include "MedicalRecordService.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace medical::application {

MedicalRecordService::MedicalRecordService(
    std::shared_ptr<domain::IMedicalRecordRepository> recordRepo,
    std::shared_ptr<domain::IPatientRepository> patientRepo,
    std::shared_ptr<domain::IUserRepository> userRepo)
    : recordRepository_(recordRepo), patientRepository_(patientRepo),
      userRepository_(userRepo) {}

MedicalRecordResponse MedicalRecordService::createRecord(const CreateMedicalRecordRequest& request) {
    // Check if patient exists
    auto patient = patientRepository_->findById(request.patientId);
    if (!patient.has_value()) {
        throw std::runtime_error("Patient not found");
    }
    
    // Check if doctor exists
    auto doctor = userRepository_->findById(request.doctorId);
    if (!doctor.has_value()) {
        throw std::runtime_error("Doctor not found");
    }
    
    // Create domain objects
    domain::Diagnosis diagnosis(request.diagnosisCode, request.diagnosisDescription);
    auto code = domain::MedicalRecordCode::generate();
    
    int nextId = recordRepository_->findAll().size() + 1;
    domain::MedicalRecord record(nextId, code, request.patientId, request.doctorId,
                                  diagnosis, request.complaints);
    
    recordRepository_->save(record);
    
    return mapToResponse(record);
}

std::vector<MedicalRecordResponse> MedicalRecordService::getPatientRecords(int patientId) {
    auto records = recordRepository_->findByPatientId(patientId);
    std::vector<MedicalRecordResponse> responses;
    for (const auto& record : records) {
        responses.push_back(mapToResponse(record));
    }
    return responses;
}

std::optional<MedicalRecordResponse> MedicalRecordService::getRecordByCode(const std::string& code) {
    try {
        domain::MedicalRecordCode recordCode(code);
        auto record = recordRepository_->findByCode(recordCode);
        if (!record) {
            return std::nullopt;
        }
        return mapToResponse(*record);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

MedicalRecordResponse MedicalRecordService::mapToResponse(const domain::MedicalRecord& record) {
    MedicalRecordResponse response;
    response.id = record.id();
    response.code = record.code().value();
    response.patientId = record.patientId();
    response.doctorId = record.doctorId();
    response.diagnosisCode = record.diagnosis().code();
    response.diagnosisDescription = record.diagnosis().description();
    response.complaints = record.complaints();
    response.recommendations = record.recommendations();
    response.createdAt = formatTime(record.createdAt());
    return response;
}

std::string MedicalRecordService::formatTime(const std::chrono::system_clock::time_point& tp) {
    auto time = std::chrono::system_clock::to_time_t(tp);
    std::tm* tm = std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace medical::application
