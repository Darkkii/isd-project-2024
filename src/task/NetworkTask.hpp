#ifndef NETWORKTASK_HPP
#define NETWORKTASK_HPP

#include "task/BaseTask.hpp"

namespace Task
{

class NetworkTask : public BaseTask
{
  public:
    NetworkTask(const std::string ip);
    void run() override;

  private:
    bool m_Initialized{false};
    std::string m_Network{"192.168.0.0"};
    std::string m_NetMask{"255.255.255.0"};
    int init();
    void networkError();
};

} // namespace Task

#endif /* NETWORKTASK_HPP */
