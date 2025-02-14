#include "AccessPointTask.hpp"

#include "debug/Print.hpp"
#include "network/NetworkGroup.hpp"
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
#include <utility>

namespace Task
{
AccessPointTask::AccessPointTask(const std::shared_ptr<std::string> ssid,
                                 const std::shared_ptr<std::string> serverIp,
                                 std::shared_ptr<Network::NetworkGroup> networkGroup) :
    BaseTask{"AccessPointTask", 256, this, MED},
    m_Ssid{std::move(ssid)},
    m_ServerIp{std::move(serverIp)},
    m_NetworkGroup{std::move(networkGroup)}
{}

void AccessPointTask::run()
{
    int rc = cyw43_arch_init();

    if (rc == PICO_ERROR_NONE)
    {
        ip_addr_t ip;
        ip_addr_t netmask;

        cyw43_arch_enable_ap_mode(m_Ssid->c_str(), nullptr, CYW43_AUTH_OPEN);
        struct netif *interface = &cyw43_state.netif[CYW43_ITF_AP];

        if (netif_is_up(interface))
        {
            ip4addr_aton(m_ServerIp->c_str(), &ip);
            ip4addr_aton(m_NetMask->c_str(), &netmask);
            netif_set_addr(interface, &ip, &netmask, &ip);
        }
    }
    else { networkError(); }

    m_NetworkGroup->set(Network::AP);
    Debug::printInfo("AP", "WiFi network established.");

    // TODO: remove DNS
    m_NetworkGroup->wait(Network::AP_DHCP);
    Network::Dns::DnsServer dns{m_ServerIp};
    m_NetworkGroup->set(Network::DNS);

    while (true) { vTaskDelay(portMAX_DELAY); }
}

void AccessPointTask::networkError()
{
    if (cyw43_is_initialized(&cyw43_state)) { cyw43_arch_deinit(); }

    // TODO: add indicator LED for error state, retry network init periodically until success
    while (true) { tight_loop_contents(); }
}

} // namespace Task