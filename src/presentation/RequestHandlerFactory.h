#pragma once
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <memory>

namespace medical {
namespace application {
    class UserService;
    class PatientService;
    class MedicalRecordService;
}
namespace presentation {

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
private:
    std::shared_ptr<application::UserService> userService_;
    std::shared_ptr<application::PatientService> patientService_;
    std::shared_ptr<application::MedicalRecordService> medicalRecordService_;
    
public:
    RequestHandlerFactory(
        std::shared_ptr<application::UserService> userService,
        std::shared_ptr<application::PatientService> patientService,
        std::shared_ptr<application::MedicalRecordService> medicalRecordService);
    
    Poco::Net::HTTPRequestHandler* createRequestHandler(
        const Poco::Net::HTTPServerRequest& request) override;
};

} // namespace presentation
} // namespace medical
