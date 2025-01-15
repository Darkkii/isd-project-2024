#include "NetworkTask.hpp"

#include "network/dhcp/DhcpServer.hpp"
#include "network/dns/DnsServer.hpp"
#include "portmacro.h"
#include <cyw43_configport.h>
#include <cyw43_ll.h>
#include <pico/cyw43_arch.h>
#include <pico/error.h>

#include <cyw43.h>
#include <pico.h>

namespace Task
{
NetworkTask::NetworkTask(const std::string serverIp) :
    BaseTask{"NetworkTask", 256, this, MED},
    m_ServerIp{std::move(serverIp)}
{}

void NetworkTask::run()
{
    int rc = init();

    if (rc != PICO_ERROR_NONE) { networkError(); }

    Network::Dhcp::DhcpServer dhcp(m_ServerIp, 24);
    Network::Dns::DnsServer dns(m_ServerIp);

    while (true) { vTaskDelay(portMAX_DELAY); }
}

int NetworkTask::init()
{
    int rc = cyw43_arch_init();

    if (rc == PICO_ERROR_NONE)
    {
        m_Initialized = true;

        cyw43_arch_enable_ap_mode("ISD_SENSOR_DATA", nullptr, CYW43_AUTH_OPEN);
    }

    return rc;
}

void NetworkTask::networkError()
{
    if (cyw43_is_initialized(&cyw43_state)) { cyw43_arch_deinit(); }

    // TODO: add indicator LED for error state
    // TODO: retry network init periodically until success
    while (true) { tight_loop_contents(); }
}

} // namespace Task