#pragma once
#include <string>
#include <regex>
#include <stdexcept>
#include <chrono>
#include <ctime>

namespace medical::domain {

// Value Object: Email
class Email {
private:
    std::string value_;
    
    bool isValid(const std::string& email) {
        const std::regex pattern(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
        return std::regex_match(email, pattern);
    }
    
public:
    explicit Email(const std::string& email) : value_(email) {
        if (!isValid(email)) {
            throw std::invalid_argument("Invalid email format");
        }
    }
    
    std::string value() const { return value_; }
    
    bool operator==(const Email& other) const {
        return value_ == other.value_;
    }
};

// Value Object: FullName
class FullName {
private:
    std::string firstName_;
    std::string lastName_;
    std::string patronymic_;
    
public:
    FullName(const std::string& firstName, const std::string& lastName, 
             const std::string& patronymic = "")
        : firstName_(firstName), lastName_(lastName), patronymic_(patronymic) {
        if (firstName.empty() || lastName.empty()) {
            throw std::invalid_argument("First name and last name are required");
        }
    }
    
    std::string firstName() const { return firstName_; }
    std::string lastName() const { return lastName_; }
    std::string patronymic() const { return patronymic_; }
    
    std::string fullName() const {
        if (patronymic_.empty()) {
            return lastName_ + " " + firstName_;
        }
        return lastName_ + " " + firstName_ + " " + patronymic_;
    }
    
    bool matchesMask(const std::string& mask) const {
        std::string full = fullName();
        return full.find(mask) != std::string::npos;
    }
};

// Value Object: MedicalRecordCode
class MedicalRecordCode {
private:
    std::string code_;
    
public:
    explicit MedicalRecordCode(const std::string& code) : code_(code) {
        if (code.empty()) {
            throw std::invalid_argument("Record code cannot be empty");
        }
    }
    
    std::string value() const { return code_; }
    
    static MedicalRecordCode generate() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm* tm = std::localtime(&time);
        
        char date[9];
        std::strftime(date, sizeof(date), "%Y%m%d", tm);
        
        static int counter = 0;
        char code[20];
        std::snprintf(code, sizeof(code), "MED-%s-%05d", date, ++counter);
        
        return MedicalRecordCode(std::string(code));
    }
};

// Value Object: Diagnosis
class Diagnosis {
private:
    std::string code_;
    std::string description_;
    
public:
    Diagnosis(const std::string& code, const std::string& description)
        : code_(code), description_(description) {
        if (code.empty() || description.empty()) {
            throw std::invalid_argument("Diagnosis code and description required");
        }
    }
    
    std::string code() const { return code_; }
    std::string description() const { return description_; }
};

} // namespace medical::domain
