#ifndef BLINKTASK_HPP
#define BLINKTASK_HPP

#include "BaseTask.hpp"

namespace Task
{
class BlinkTask : public BaseTask
{
  public:
    BlinkTask();
    void run() override;

  private:
};
} // namespace Task

#endif /* BLINKTASK_HPP */
