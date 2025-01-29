#ifndef NETWORKTASK_HPP
#define NETWORKTASK_HPP

#include "task/BaseTask.hpp"

#include <memory>

namespace Task
{

class NetworkTask : public BaseTask
{
  public:
    NetworkTask(const std::shared_ptr<std::string> ssid,
                const std::shared_ptr<std::string> serverIp);
    void run() override;

  private:
    bool m_Initialized{false}; // TODO: need?
    const std::shared_ptr<std::string> m_Ssid{
        std::make_shared<std::string>("ISD_SENSOR_DATA")};
    const std::shared_ptr<std::string> m_ServerIp{
        std::make_shared<std::string>("192.168.0.1")};
    const std::shared_ptr<std::string> m_NetMask{
        std::make_shared<std::string>("255.255.255.0")};
    void networkError();
};

} // namespace Task

#endif /* NETWORKTASK_HPP */
