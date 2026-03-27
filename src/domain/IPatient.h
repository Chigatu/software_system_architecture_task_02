#pragma once
#include "User.h"
#include <string>
#include <chrono>
#include <optional>

namespace medical::domain {

// Domain Entity: Patient (может быть связан с User или быть отдельным)
class Patient {
private:
    int id_;
    std::optional<int> userId_;  // может быть связан с аккаунтом
    FullName fullName_;
    std::string phone_;
    std::string address_;
    std::chrono::system_clock::time_point birthDate_;
    std::string snils_;  // Страховой номер
    std::string policyNumber_;
    std::chrono::system_clock::time_point createdAt_;
    
public:
    Patient(int id, const FullName& fullName, const std::string& phone,
            const std::string& address, const std::chrono::system_clock::time_point& birthDate,
            const std::string& snils, const std::string& policyNumber,
            std::optional<int> userId = std::nullopt)
        : id_(id), userId_(userId), fullName_(fullName), phone_(phone),
          address_(address), birthDate_(birthDate), snils_(snils), 
          policyNumber_(policyNumber) {
        createdAt_ = std::chrono::system_clock::now();
    }
    
    int id() const { return id_; }
    std::optional<int> userId() const { return userId_; }
    const FullName& fullName() const { return fullName_; }
    const std::string& phone() const { return phone_; }
    const std::string& address() const { return address_; }
    auto birthDate() const { return birthDate_; }
    const std::string& snils() const { return snils_; }
    const std::string& policyNumber() const { return policyNumber_; }
    
    void updateContactInfo(const std::string& phone, const std::string& address) {
        phone_ = phone;
        address_ = address;
    }
    
    bool matchesNameMask(const std::string& mask) const {
        return fullName_.matchesMask(mask);
    }
    
    int age() const {
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::years>(
            now - birthDate_);
        return age.count();
    }
};

} // namespace medical::domain