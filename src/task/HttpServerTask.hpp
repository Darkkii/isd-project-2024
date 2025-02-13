#ifndef HTTPSERVERTASK_HPP
#define HTTPSERVERTASK_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "fs/File.hpp"
// #include "rtos/Queue.hpp"
// #include "sensor/SensorReading.hpp"
#include "network/NetworkGroup.hpp"
#include "task/BaseTask.hpp"
#include <lwip/api.h>
#include <lwip/netbuf.h>

#include <memory>

namespace Task
{

class HttpServerTask : public BaseTask
{
  public:
    HttpServerTask(const std::shared_ptr<std::string> serverIp,
                   std::shared_ptr<Network::NetworkGroup> networkGroup);
    void run() override;

  private:
    netconn *m_ServerConnection{nullptr};
    const std::shared_ptr<std::string> m_ServerIp{};
    // std::shared_ptr<Rtos::Queue<Sensor::SensorReading, 10>> m_Queue;
    std::shared_ptr<Network::NetworkGroup> m_NetworkGroup;
    err_t handleRequest(netconn *client, const std::string &request) const;
    err_t sendResponse(netconn *client, std::string &header, Fs::File *file = nullptr) const;
    void networkError();
};

} // namespace Task

#endif /* HTTPSERVERTASK_HPP */
