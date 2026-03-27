#include "PatientService.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace medical::application {

PatientService::PatientService(std::shared_ptr<domain::IPatientRepository> patientRepo,
                               std::shared_ptr<domain::IUserRepository> userRepo)
    : patientRepository_(patientRepo), userRepository_(userRepo) {}

std::chrono::system_clock::time_point PatientService::parseDate(const std::string& dateStr) {
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        throw std::invalid_argument("Invalid date format. Use YYYY-MM-DD");
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

PatientResponse PatientService::createPatient(const CreatePatientRequest& request) {
    // Проверяем SNILS на уникальность
    if (patientRepository_->findBySnils(request.snils).has_value()) {
        throw std::runtime_error("Patient with this SNILS already exists");
    }
    
    // Если указан userId, проверяем что пользователь существует
    if (request.userId.has_value()) {
        if (!userRepository_->findById(request.userId.value()).has_value()) {
            throw std::runtime_error("User not found");
        }
    }
    
    // Создаем domain объекты
    domain::FullName fullName(request.firstName, request.lastName, request.patronymic);
    auto birthDate = parseDate(request.birthDate);
    
    // Создаем пациента
    int nextId = patientRepository_->findAll().size() + 1;
    domain::Patient patient(nextId, fullName, request.phone, request.address,
                            birthDate, request.snils, request.policyNumber,
                            request.userId);
    
    patientRepository_->save(patient);
    
    return mapToResponse(patient);
}

std::optional<PatientResponse> PatientService::getPatientById(int id) {
    auto patient = patientRepository_->findById(id);
    if (!patient) {
        return std::nullopt;
    }
    return mapToResponse(*patient);
}

std::vector<PatientResponse> PatientService::searchPatientsByFullName(const std::string& mask) {
    auto patients = patientRepository_->findByFullNameMask(mask);
    std::vector<PatientResponse> responses;
    for (const auto& patient : patients) {
        responses.push_back(mapToResponse(patient));
    }
    return responses;
}

std::optional<PatientResponse> PatientService::getPatientByUserId(int userId) {
    auto patient = patientRepository_->findByUserId(userId);
    if (!patient) {
        return std::nullopt;
    }
    return mapToResponse(*patient);
}

PatientResponse PatientService::mapToResponse(const domain::Patient& patient) {
    PatientResponse response;
    response.id = patient.id();
    response.fullName = patient.fullName().fullName();
    response.phone = patient.phone();
    response.address = patient.address();
    response.age = patient.age();
    response.snils = patient.snils();
    response.policyNumber = patient.policyNumber();
    return response;
}

} // namespace medical::application
