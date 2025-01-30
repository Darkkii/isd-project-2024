#ifndef HTTPSERVERTASK_HPP
#define HTTPSERVERTASK_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "event_groups.h"
#include "fs/File.hpp"
#include "task/BaseTask.hpp"
#include <lwip/api.h>
#include <lwip/netbuf.h>

#include <memory>

// TODO: sensor data format
struct SensorData
{
    void *data;
};

namespace Task
{

class HttpServerTask : public BaseTask
{
  public:
    HttpServerTask(const std::shared_ptr<std::string> serverIp,
                   EventGroupHandle_t eventGroup);
    void run() override;

  private:
    const std::shared_ptr<std::string> m_ServerIp{};
    netconn *m_ServerConnection{nullptr};
    // Rtos::Queue<SensorData, 10> m_Queue;
    EventGroupHandle_t m_EventGroup;
    err_t handleRequest(netconn *client, const std::string &request) const;
    err_t sendResponse(netconn *client, std::string &header, Fs::File *file = nullptr) const;
    void networkError();
};

} // namespace Task

#endif /* HTTPSERVERTASK_HPP */
