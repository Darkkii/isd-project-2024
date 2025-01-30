#ifndef DNSCOMMON_HPP
#define DNSCOMMON_HPP

#include <cstdint>

constexpr uint16_t DNS_MESSAGE_LEN_MAX = 512; // Max DNS message length
constexpr uint16_t DNS_RECORDS_LEN_MAX = 416; // Max DNS message length - headers

extern "C"
{
    struct DnsMessage
    {
        uint16_t id{};
        uint16_t flags{};
        uint16_t question_count{};
        uint16_t answer_record_count{};
        uint16_t authority_record_count{};
        uint16_t additional_record_count{};
        uint8_t records[DNS_RECORDS_LEN_MAX]{};
    };
};

#endif /* DNSCOMMON_HPP */
