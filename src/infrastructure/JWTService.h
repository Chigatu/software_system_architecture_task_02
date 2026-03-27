#pragma once
#include <string>
#include <optional>

namespace medical::infrastructure {

class JWTService {
private:
    std::string secret_;
    int expirationSeconds_;
    
public:
    JWTService(const std::string& secret, int expirationSeconds = 3600);
    
    std::string generateToken(int userId, const std::string& login);
    std::optional<int> validateToken(const std::string& token);
    
private:
    std::string base64Encode(const std::string& input);
    std::string base64Decode(const std::string& input);
};

} // namespace medical::infrastructure