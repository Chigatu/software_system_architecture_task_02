#pragma once
#include "../domain/interfaces/IRepository.h"
#include "../domain/Patient.h"
#include <unordered_map>
#include <mutex>

namespace medical::infrastructure {

class InMemoryPatientRepository : public domain::IPatientRepository {
private:
    std::unordered_map<int, domain::Patient> storage_;
    std::mutex mutex_;
    int nextId_ = 1;
    
public:
    std::optional<domain::Patient> findById(int id) override;
    std::vector<domain::Patient> findAll() override;
    int save(const domain::Patient& entity) override;
    void update(const domain::Patient& entity) override;
    void remove(int id) override;
    bool exists(int id) override;
    
    std::optional<domain::Patient> findByUserId(int userId) override;
    std::vector<domain::Patient> findByFullNameMask(const std::string& mask) override;
    std::optional<domain::Patient> findBySnils(const std::string& snils) override;
};

} // namespace medical::infrastructure
