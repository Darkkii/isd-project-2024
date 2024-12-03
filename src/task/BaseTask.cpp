#include "BaseTask.hpp"

#include <utility>

namespace Task
{

BaseTask::BaseTask(std::string name,
                   uint32_t stackDepth,
                   void *const thisPtr,
                   priority taskPriority) :
    m_Name{std::move(name)}
{
    xTaskCreate(runner,
                m_Name.c_str(),
                stackDepth,
                thisPtr,
                tskIDLE_PRIORITY + taskPriority,
                &m_Handle);
}

TaskHandle_t BaseTask::getHandle() { return m_Handle; }

void BaseTask::runner(void *params)
{
    auto *task = static_cast<BaseTask *>(params);
    task->run();
}

} // namespace Task