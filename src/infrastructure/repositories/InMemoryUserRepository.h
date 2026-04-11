#pragma once
#include "../../domain/interfaces/IRepository.h"
#include "../../domain/entities/User.h"
#include <unordered_map>
#include <mutex>

namespace medical::infrastructure {

class InMemoryUserRepository : public domain::IUserRepository {
private:
    std::unordered_map<int, domain::User> storage_;
    std::mutex mutex_;
    int nextId_;
    
public:
    InMemoryUserRepository();
    
    std::optional<domain::User> findById(int id) override;
    std::vector<domain::User> findAll() override;
    int save(const domain::User& entity) override;
    void update(const domain::User& entity) override;
    void remove(int id) override;
    bool exists(int id) override;
    
    std::optional<domain::User> findByLogin(const std::string& login) override;
    std::vector<domain::User> findByFullNameMask(const std::string& mask) override;
};

} // namespace medical::infrastructure
