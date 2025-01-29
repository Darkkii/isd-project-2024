#include "NetworkTask.hpp"

#include "network/dhcp/DhcpServer.hpp"
#include "network/dns/DnsServer.hpp"
#include "portmacro.h"
#include <cyw43_configport.h>
#include <cyw43_ll.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <pico/cyw43_arch.h>
#include <pico/error.h>

#include <cyw43.h>
#include <pico.h>

namespace Task
{
NetworkTask::NetworkTask(const std::shared_ptr<std::string> ssid,
                         const std::shared_ptr<std::string> serverIp) :
    BaseTask{"NetworkTask", 256, this, MED},
    m_Ssid{std::move(ssid)},
    m_ServerIp{std::move(serverIp)}
{}

void NetworkTask::run()
{
    struct netif *interface = nullptr;
    int rc = cyw43_arch_init();

    if (rc == PICO_ERROR_NONE)
    {
        cyw43_arch_enable_ap_mode(m_Ssid->c_str(), nullptr, CYW43_AUTH_OPEN);
        interface = &cyw43_state.netif[CYW43_ITF_AP];
    }

    if (netif_is_up(interface))
    {
        ip_addr_t ip;
        ip_addr_t netmask;

        ip4addr_aton(m_ServerIp->c_str(), &ip);
        ip4addr_aton(m_NetMask->c_str(), &netmask);
        netif_set_addr(interface, &ip, &netmask, &ip);
    }

    if (rc != PICO_ERROR_NONE) { networkError(); }

    Network::Dhcp::DhcpServer dhcp{m_ServerIp, 24};
    Network::Dns::DnsServer dns{m_ServerIp};

    while (true) { vTaskDelay(portMAX_DELAY); }
}

void NetworkTask::networkError()
{
    if (cyw43_is_initialized(&cyw43_state)) { cyw43_arch_deinit(); }

    // TODO: add indicator LED for error state, retry network init periodically until success
    while (true) { tight_loop_contents(); }
}

} // namespace Task