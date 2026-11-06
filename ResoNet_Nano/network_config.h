/**
 * @file network_config.h
 * @brief Explicit compile-time configuration for ENC28J60 diagnostics.
 *
 * The Ethernet transport is disabled by default. Enable only after assigning
 * an unused address in the target LAN and verifying the network plan.
 */
#ifndef RESONET_NETWORK_CONFIG_H
#define RESONET_NETWORK_CONFIG_H

#ifndef RESONET_ENABLE_ETHERNET
#define RESONET_ENABLE_ETHERNET 0
#endif

#if RESONET_ENABLE_ETHERNET
// Set these values for the actual LAN before enabling the Ethernet transport.
#define RESONET_MAC_0 0x02
#define RESONET_MAC_1 0x52
#define RESONET_MAC_2 0x4E
#define RESONET_MAC_3 0x00
#define RESONET_MAC_4 0x00
#define RESONET_MAC_5 0x01

#define RESONET_IP_0 192
#define RESONET_IP_1 168
#define RESONET_IP_2 1
#define RESONET_IP_3 177

#define RESONET_DNS_0 192
#define RESONET_DNS_1 168
#define RESONET_DNS_2 1
#define RESONET_DNS_3 1

#define RESONET_GATEWAY_0 192
#define RESONET_GATEWAY_1 168
#define RESONET_GATEWAY_2 1
#define RESONET_GATEWAY_3 1

#define RESONET_SUBNET_0 255
#define RESONET_SUBNET_1 255
#define RESONET_SUBNET_2 255
#define RESONET_SUBNET_3 0
#endif

#endif
