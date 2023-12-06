/*
 * NatRouter.h
 *
 *  Created on: 09.11.2020
 *      Author: gesser
 */

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#ifndef NatRouter_H
#define NatRouter_H

#include <vector>
#include <set>

#include <esp_err.h>
#include <esp_wifi.h>
#include <lwip/netif.h>

#include "EspLog.h"

struct ip_hdr;
struct tcp_hdr;
struct udp_hdr;
struct eth_hdr;

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

class NatRouter : public UxEspCppLibrary::EspLog
{   
public:

    enum debugMask_t
    {
        DebugOff       = 0x0000,
        DebugPbuf      = 0x0001,
        DebugEth       = 0x0002,
        DebugIp4       = 0x0004,
        DebugIp6       = 0x0008,
        DebugArp       = 0x0010,
        DebugTcp       = 0x0020,
        DebugUdp       = 0x0040,
        DebugIcmp      = 0x0080,
        DebugSkips     = 0x0100,
        DebugErrors    = 0x0200,
        DebugProtoIp4  = ( DebugTcp | DebugUdp | DebugIcmp | DebugEth | DebugIp4 ),
        DebugAll       = 0xffff,
    };

    enum class IpProtocolType
    {
        IpProtocolNone = 0,
                IpProtocolUdp,
                IpProtocolTcp
    };

    struct WhiteListEntry
    {
        int             nPort;
        IpProtocolType  nIpProtocolType;
    };

public:

    NatRouter();

    virtual ~NatRouter();

    void addWhiteListPort( const int nPort,
                           const IpProtocolType nIpProtocol );
    bool isBlockedByWhiteList( const int nPort,
                               const IpProtocolType nIpProtocolType ) const;

    void blockInternalPort( const int nInternalPort );
    bool isInternalBlockedPort( const int nPort ) const;

    void enableIcmpReply( const bool bEnable );

    void enableDnsReply( const bool bEnable );

    void setInternalInterface( netif * pNetIf,
                               const ip4_addr_t ip4AddrInternalReceiver );

    void setExternalInterface( esp_netif_t * pNetIf );

    void setDebugPort( const int nPort );
    void setDebugMask( const int nDebugMask );
    int debugMask( void ) const;

    struct eth_hdr * ethHeader( struct pbuf * p ) const;
    struct eth_hdr * ethHeader( void * pBuffer ) const;
    struct ip_hdr * ipHeader( struct pbuf * p ) const;
    struct ip_hdr * ipHeader( void * pBuffer ) const;
    struct ip_hdr * ipHeader( struct eth_hdr * pEthHdr ) const;
    const struct ip_hdr * ipHeader( const struct eth_hdr * pEthHdr ) const;
    struct tcp_hdr * tcpHeader( struct ip_hdr * pIpHdr ) const;
    const struct tcp_hdr * tcpHeader( const struct ip_hdr * pIpHdr ) const;
    struct udp_hdr * udpHeader( struct ip_hdr * pIpHdr ) const;
    const struct udp_hdr * udpHeader( const struct ip_hdr * pIpHdr ) const;
    struct icmp_echo_hdr * icmpHeader( struct ip_hdr * pIpHdr ) const;

    bool isDebugPort( const struct ip_hdr * pIpHdr ) const;
    bool isDebugPort( const struct tcp_hdr * pTcpHdr ) const;
    bool isDebugPort( const struct udp_hdr * pUdpHdr ) const;

private:

    static err_t internalInputCallback( struct pbuf *p,
                                        struct netif *inp );
    static void externalInputCallback( void *input_netif_handle,
                                       void *buffer,
                                       size_t len,
                                       void *eb );

    err_t processInternalIp4( struct pbuf *p,
                              struct netif *inp );

    bool processExternalEthernet( void *input_netif_handle,
                                  void *buffer,
                                  size_t len,
                                  void *eb );
    bool processExternalIp4( void *input_netif_handle,
                             void *buffer,
                             size_t len,
                             void *eb );

    uint16_t calculateTcpCheckSumInIpPack( struct pbuf * pIpBuf );
    uint16_t calculateTcpCheckSumInIpPack( struct ip_hdr * iph,
                                           const size_t uBufLen );

    void processIncommingAddressTranslation( struct ip_hdr * iph );
    void processOutgoingAddressTranslation( struct ip_hdr* iph,
                                            const size_t uBufLen );
    void processOutgoingAddressTranslation( struct ip_hdr* iph,
                                            struct pbuf * p );

    void dumpPbuf( const struct pbuf * p ) const;
    void dumpEthernetHeader( const struct eth_hdr * pEthHdr ) const;
    void dumpIpHeader( const struct ip_hdr * pIpHdr ) const;
    void dumpTcpHeader( const struct tcp_hdr * pTcpHdr ) const;
    void dumpUdpHeader( const struct udp_hdr * pUdpHdr ) const;
    void dumpIcmpHeader( const struct icmp_echo_hdr * pIcmpHdr ) const;

    bool debugMatchMask( const int nDebugMask ) const;

private:

    static NatRouter * m_pSingleton;

    bool m_bEnableIcmpReply { true };

    bool m_bEnableDnsReply { true };

    std::set<int>   m_theInternalPorts;

    std::vector<WhiteListEntry>   m_theWhiteListPorts;

    netif * m_pInternalNetIf { nullptr };

    esp_netif_t * m_pExternalNetIf { nullptr };

    netif_input_fn  m_pOriginalInternalInputFunc { nullptr };

    ip4_addr_t m_ip4AddrInternalReceiver;

    int m_nDebugMask { DebugOff };

    int m_nDebugPort { 0 };

};

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* NatRouter_H */
