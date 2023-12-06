/*
 * NatRouter.cpp
 *
 *  Created on: 09.11.2020
 *      Author: gesser
 */

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "NatRouter.h"

#include <lwip/prot/ethernet.h>
#include <lwip/prot/tcp.h>
#include <lwip/ip.h>
#include <lwip/ip4.h>
#include <lwip/def.h>
#include <lwip/udp.h>
#include <lwip/inet_chksum.h>
#include <lwip/icmp.h>
#include <lwip/pbuf.h>
#include <lwip/inet.h>
#include <memory.h>
#include <string.h>
#include <lwip/ip_addr.h>

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#include <netif/wlanif.h>
#else
#include <lwip/esp_netif_net_stack.h>
#endif

#include <esp_netif_net_stack.h>
#include <../lwip/esp_netif_lwip_internal.h>

#include "ApplicationGlobals.h"

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

NatRouter * NatRouter::m_pSingleton { nullptr };

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

NatRouter::NatRouter()
: EspLog( "NatRouter" )
{
    ESP_ERROR_CHECK( m_pSingleton != nullptr );

    m_pSingleton = this;

    setDebugMask( DebugErrors | DebugSkips );
    //setDebugMask( DebugErrors );
    //setDebugMask( DebugTcp | DebugErrors | DebugSkips );
    //setDebugMask( DebugUdp | DebugProtoIp4 | DebugArp | DebugErrors);
    //setDebugMask( DebugUdp | DebugProtoIp4 | DebugErrors);
    //setDebugMask( DebugProtoIp4 );
    //setDebugMask( DebugAll );
    //setDebugPort( 5001 );

    addWhiteListPort( 53, IpProtocolType::IpProtocolUdp );     // DNS
    addWhiteListPort( 5353, IpProtocolType::IpProtocolUdp );   // MDNS
    addWhiteListPort( 67, IpProtocolType::IpProtocolUdp );     // DHCP
    addWhiteListPort( 68, IpProtocolType::IpProtocolUdp );     // DHCP
    addWhiteListPort( 123, IpProtocolType::IpProtocolUdp );    // NTP
    addWhiteListPort( 123, IpProtocolType::IpProtocolTcp );    // NTP
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

NatRouter::~NatRouter()
{
    m_theInternalPorts.clear();
    m_theWhiteListPorts.clear();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::setInternalInterface( netif * pNetIf,
                                      const ip4_addr_t ip4AddrInternalReceiver )
{
    ESP_ERROR_CHECK( pNetIf != nullptr && m_pInternalNetIf != nullptr );
    vlogInfo( "setInternalInterface %p", pNetIf );

    if ( pNetIf != m_pInternalNetIf )
    {
        vlogInfo( "setInternalInterface %p - modified", pNetIf );

        if ( pNetIf )
        {
            ESP_ERROR_CHECK( pNetIf->input == nullptr );

            if ( m_pOriginalInternalInputFunc != pNetIf->input )
            {
                vlogInfo( "setInternalInterface - input func modified %p", pNetIf->input );
                m_pOriginalInternalInputFunc = pNetIf->input;
                pNetIf->input = internalInputCallback;
            }
        }
        else if ( m_pInternalNetIf )
        {
            m_pInternalNetIf->input = m_pOriginalInternalInputFunc;
            m_pOriginalInternalInputFunc = nullptr;
        }
        else
        {
            m_pOriginalInternalInputFunc = nullptr;
            vlogError( "setInternalInterface: nullptr given" );
        }

        m_pInternalNetIf = pNetIf;
        m_ip4AddrInternalReceiver = ip4AddrInternalReceiver;
    }

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
err_t NatRouter::internalInputCallback( struct pbuf *p,
                                        struct netif *inp )
{
    err_t nEspErr = ERR_ARG;

    if ( p && inp )
    {
        if ( m_pSingleton->m_pInternalNetIf
                        && m_pSingleton->m_pOriginalInternalInputFunc )
        {
            nEspErr = m_pSingleton->processInternalIp4( p, inp );

            //            if ( p->ref > 0 && nEspErr == ESP_OK )
            //            {
            //                pbuf_free( p );
            //            }
        }
        else
        {
            //            if ( p->ref > 0 )
            //            {
            //                pbuf_free( p );
            //            }
        }
    }

    return nEspErr;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

err_t NatRouter::processInternalIp4( struct pbuf *p,
                                     struct netif *inp )
{
    err_t nEspErr = ERR_ARG;

    if ( !p )
    {
        vlogError( "processInternalIp4 reject illegal null package" );
    }
    else if ( p->len < sizeof( struct ip_hdr ) )
    {
        vlogWarning( "processInternalIp4 reject package with illegal IP header" );
    }
    else
    {
        struct ip_hdr * iph = ipHeader( p );

        if ( debugMatchMask( DebugIp4 ) )
        {
            vlogInfo( "processInternalIp4" );
            dumpIpHeader( iph );
        }

        if ( IPH_V( iph ) == 6 )
        {
            if ( debugMatchMask( DebugSkips ) )
            {
                vlogWarning( "processInternalIp4: skip package (is IPv6)" );
            }
        }
        else if ( IPH_V( iph ) != 4 )
        {
            if ( debugMatchMask( DebugSkips ) )
            {
                vlogWarning( "processInternalIp4: skip package (not IPv4) %d", IPH_V( iph ) );
            }
        }
        else
        {
            ip_addr_t  dstIp;
            ip_addr_t  srcIp;

            ip_addr_copy_from_ip4( dstIp, iph->dest );
            ip_addr_copy_from_ip4( srcIp, iph->src );

            ip4_addr_t ip4Dst = dstIp.u_addr.ip4;

            switch ( IPH_PROTO( iph ) )
            {
                case IP_PROTO_UDP:
                {
                    struct udp_hdr * udph = udpHeader( iph );

                    if ( debugMatchMask( DebugUdp ) )
                    {
                        dumpUdpHeader( udph );
                    }

                    int nDstPort = ntohs( udph->dest );
                    int nSrcPort = ntohs( udph->src );

                    // fragmented packages are treated internally
                    // Todo: this is dependent of the first initial package with correct UDP header
                    // subsequent packages have to follow its predecessor which isn't treated hereby not very correct!
                    if ( ( ntohs(IPH_OFFSET(iph)) & IP_DF ) == 0 )
                    {
                        if ( debugMatchMask( DebugUdp ) )
                        {
                            vlogInfo( "processInternalIp4: route fragmented udp package to internal interface" );
                        }

                        nEspErr = m_pOriginalInternalInputFunc( p, inp );

                        if ( nEspErr != ERR_OK && debugMatchMask( DebugErrors ) )
                        {
                            vlogError( "processInternalIp4: fragmented udp internal input failed %d (%s)", nEspErr, lwip_strerr( nEspErr ) );
                        }
                    }
                    else // not fragmented packages have correct UDP headers
                    {
                        // route package internally
                        if ( isInternalBlockedPort( nDstPort )
                                        || isInternalBlockedPort( nSrcPort ) )
                        {
                            if ( debugMatchMask( DebugUdp ) )
                            {
                                vlogInfo( "processInternalIp4: route blocked UDP package to internal interface" );
                            }

                            if ( m_pOriginalInternalInputFunc )
                            {
                                nEspErr = m_pOriginalInternalInputFunc( p, inp );
                            }

                            if ( nEspErr != ERR_OK && debugMatchMask( DebugErrors ) )
                            {
                                vlogError( "processInternalIp4: blocked UDP internal input failed %d (%s)", nEspErr, lwip_strerr( nEspErr ) );
                            }
                        }
                        else if ( !m_pExternalNetIf || !m_pExternalNetIf->lwip_netif || !m_pExternalNetIf->lwip_netif->output )
                        {
                            if ( debugMatchMask( DebugSkips ) )
                            {
                                vlogInfo( "processInternalIp4: skip route UDP package, no external interface" );
                            }
                        }
                        else
                        {
                            if ( debugMatchMask( DebugUdp ) )
                            {
                                vlogInfo( "processInternalIp4: route UDP package to external interface" );
                            }

                            nEspErr = m_pExternalNetIf->lwip_netif->output( m_pExternalNetIf->lwip_netif, p, &ip4Dst );

                            if ( nEspErr != ERR_OK && debugMatchMask( DebugErrors ) )
                            {
                                vlogError( "processInternalIp4: UDP external output failed %d (%s)", nEspErr, lwip_strerr( nEspErr ) );
                            }

                            if ( p->ref > 0 && nEspErr == ESP_OK )
                            {
                                pbuf_free( p );
                            }
                        }
                    }
                }
                break;

                case IP_PROTO_UDPLITE:
                {
                    if ( debugMatchMask( DebugSkips ) )
                    {
                        vlogWarning( "processInternalIp4: skip UDPLITE package" );
                    }
                }
                break;

                case IP_PROTO_TCP:
                {
                    if ( debugMatchMask( DebugTcp ) )
                    {
                        dumpTcpHeader( tcpHeader( iph ) );
                    }

                    if ( debugMatchMask( DebugTcp ) )
                    {
                        vlogInfo( "processInternalIp4: route TCP package to external interface" );
                    }

                    if ( !m_pExternalNetIf || !m_pExternalNetIf->lwip_netif || !m_pExternalNetIf->lwip_netif->output )
                    {
                        if ( debugMatchMask( DebugSkips ) )
                        {
                            vlogInfo( "processInternalIp4: skip route TCP package, no external interface" );
                        }
                    }
                    else
                    {
                        processOutgoingAddressTranslation( iph, p );

                        nEspErr = m_pExternalNetIf->lwip_netif->output( m_pExternalNetIf->lwip_netif, p, &ip4Dst );// >driver_transmit( m_pExternalNetIf->driver_handle, p->payload, p->len );

                        if ( nEspErr != ERR_OK && debugMatchMask( DebugErrors ) )
                        {
                            vlogError( "processInternalIp4: TCP external output failed %d (%s)", nEspErr, lwip_strerr( nEspErr ) );
                        }

                        if ( p->ref > 0 && nEspErr == ESP_OK )
                        {
                            pbuf_free( p );
                        }
                    }
                }
                break;

                case IP_PROTO_ICMP:
                {
                    if ( debugMatchMask( DebugIcmp ) )
                    {
                        dumpIpHeader( iph );
                        dumpIcmpHeader( icmpHeader( iph ) );
                    }

                    // if ICMP request to own interface
                    if ( ip4Dst.addr == inp->ip_addr.u_addr.ip4.addr )
                    {
                        if ( debugMatchMask( DebugIcmp ) )
                        {
                            vlogInfo( "processInternalIp4: process internal ICMP package" );
                        }

                        nEspErr = m_pOriginalInternalInputFunc( p, inp );

                        if ( nEspErr != ERR_OK && debugMatchMask( DebugErrors ) )
                        {
                            vlogError( "processInternalIp4: icmp internal input failed %d (%s)", nEspErr, lwip_strerr( nEspErr ) );
                        }
                    }
                    else if ( !m_pExternalNetIf || !m_pExternalNetIf->lwip_netif || !m_pExternalNetIf->lwip_netif->output )
                    {
                        if ( debugMatchMask( DebugSkips | DebugIcmp ) )
                        {
                            vlogInfo( "processInternalIp4: skip route ICMP package, no external interface" );
                        }
                    }
                    else       // ICMP request to external interface
                    {
                        // send out package via external interface
                        if ( debugMatchMask( DebugIcmp ) )
                        {
                            vlogInfo( "processInternalIp4: route ICMP package to external (%s)", ip4addr_ntoa( &dstIp.u_addr.ip4 ) );
                        }

                        processOutgoingAddressTranslation( iph, p );

                        nEspErr = m_pExternalNetIf->lwip_netif->output( m_pExternalNetIf->lwip_netif, p, &ip4Dst );// >driver_transmit( m_pExternalNetIf->driver_handle, p->payload, p->len );

                        if ( nEspErr != ERR_OK && debugMatchMask( DebugErrors ) )
                        {
                            vlogError( "processInternalIp4: icmp external output failed %d (%s)", nEspErr, lwip_strerr( nEspErr ) );
                        }

                        if ( p->ref > 0 && nEspErr == ESP_OK )
                        {
                            pbuf_free( p );
                        }
                    }
                }
                break;

                case IP_PROTO_IGMP:
                {
                    // skip IGMP data - not relevant herein
                    if ( debugMatchMask( DebugSkips ) )
                    {
                        vlogWarning( "processInternalIp4: skip IGMP package" );
                    }
                }
                break;

                default:
                {
                    vlogWarning( "processInternalIp4: unknown IPv4 protocol package %d not processed", iph->_proto );
                }
                break;
            }
        }
    }

    return nEspErr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::setExternalInterface( esp_netif_t * pNetIf )
{
    //clearTranslationTable();
    vlogInfo( "setExternalInterface %p", pNetIf );

    if ( m_pExternalNetIf != pNetIf )
    {
        vlogInfo( "setExternalInterface %p - modified", pNetIf );
        if ( pNetIf )
        {
            ESP_ERROR_CHECK( pNetIf->lwip_input_fn == nullptr );

            pNetIf->lwip_input_fn = externalInputCallback;
        }

        m_pExternalNetIf = pNetIf;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void NatRouter::externalInputCallback( void *input_netif_handle,
                                       void *buffer,
                                       size_t len,
                                       void *eb )
{

    m_pSingleton->processExternalEthernet( input_netif_handle, buffer, len, eb );
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

bool NatRouter::processExternalEthernet( void *input_netif_handle,
                                         void *buffer,
                                         size_t len,
                                         void *eb )
{
    bool bProcessed = false;

    if ( !buffer )
    {
        vlogError( "processExternalEthernet: skip illegal null package" );
    }
    else if ( len < sizeof( struct eth_hdr ) )
    {
        if ( debugMatchMask( DebugErrors ) )
        {
            vlogWarning( "processExternalEthernet: skip package with illegal ethernet header" );
        }
    }
    else
    {
        struct eth_hdr *ethhdr = ethHeader( buffer );

        if ( debugMatchMask( DebugEth ) )
        {
            dumpEthernetHeader( ethhdr );
        }

        switch ( PP_NTOHS( ethhdr->type ) )
        {
            case ETHTYPE_IP:
            {
                bProcessed = processExternalIp4( input_netif_handle, buffer, len, eb );
            }
            break;

            case ETHTYPE_IPV6:
            {
                // skip IPv6 traffic
                if ( debugMatchMask( DebugIp6 ) )
                {
                    vlogWarning( "processExternalEthernet: skip ipv6 package" );
                }
            }
            break;

            case ETHTYPE_ARP:
            {
                if ( debugMatchMask( DebugArp ) )
                {
                    vlogInfo( "processExternalEthernet: skip ARP package" );
                }
            }
            break;

            default:
            {
                if ( debugMatchMask( DebugErrors ) )
                {
                    vlogWarning( "processExternalEthernet: skip unknown ethernet package type %d", PP_NTOHS( ethhdr->type ) );
                }
            }
            break;
        }
    }

    // if not processed, let it be done with the original function to implement default behaviour
    if ( !bProcessed )
    {
        wlanif_input( input_netif_handle, buffer, len, eb );
    }
    else
    {
        esp_netif_t *esp_netif = esp_netif_get_handle_from_netif_impl( reinterpret_cast<struct netif *>( input_netif_handle ) );
        esp_netif_free_rx_buffer( esp_netif, eb);
    }

    return bProcessed;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

bool NatRouter::processExternalIp4( void *input_netif_handle,
                                    void *buffer,
                                    size_t len,
                                    void *eb )
{
    bool bProcessed = false;

    err_t nEspErr = ERR_ARG;

    struct eth_hdr * ethh = ethHeader( buffer );
    struct ip_hdr *  iph  = ipHeader( ethh );
    size_t ipPackageLen = len - sizeof( struct eth_hdr );

    if ( debugMatchMask( DebugIp4 ) )
    {
        vlogInfo( "processExternalIp4 - %d", esp_get_free_heap_size() );
        dumpIpHeader( iph );
    }

    if ( IPH_V( iph ) != 4 )
    {
        if ( debugMatchMask( DebugSkips ) )
        {
            vlogInfo( "processExternalIp4: process package (not IPv4) %d in own external interface", IPH_V( iph ) );
        }
    }
    else
    {
        ip_addr_t  dstIp;
        ip_addr_t  srcIp;

        ip_addr_copy_from_ip4( dstIp, iph->dest );
        ip_addr_copy_from_ip4( srcIp, iph->src );

        switch ( IPH_PROTO( iph ) )
        {
            case IP_PROTO_UDP:
            {
                struct udp_hdr * udph = udpHeader( iph );

                if ( debugMatchMask( DebugUdp ) )
                {
                    dumpUdpHeader( udph );
                }

                int nDstPort = ntohs( udph->dest );
                int nSrcPort = ntohs( udph->src );

                // block internal used ports
                if ( isInternalBlockedPort( nDstPort )
                                || isInternalBlockedPort( nSrcPort ) )
                {
                    if ( debugMatchMask( DebugUdp ) )
                    {
                        vlogWarning( "processExternalIp4: block UDP package by internal used port" );
                    }
                    bProcessed = true;
                }

                else if ( isBlockedByWhiteList( nDstPort, IpProtocolType::IpProtocolUdp ) )
                    //&& isBlockedByWhiteList( nSrcPort, IpProtocolType::IpProtocolUdp ) )
                {
                    if ( debugMatchMask( DebugUdp ) )
                    {
                        vlogInfo( "processExternalIp4: process UDP package in external interface (port %d) - white listed", nDstPort );
                    }

                    //                    m_pOriginalExternalInputFunc( input_netif_handle, buffer, len, eb );
                    //
                    //                    bProcessed = true;
                }

                else
                {
                    processIncommingAddressTranslation( iph );

                    if ( debugMatchMask( DebugUdp ) )
                    {
                        vlogInfo( "processExternalIp4: route UDP package to internal interface by NAT (port %d)", nDstPort );
                    }

                    // reduce ethernet package to none ethernet package
                    struct pbuf * p = pbuf_alloc_reference( iph, ipPackageLen, PBUF_REF );
                    nEspErr = m_pInternalNetIf->output( m_pInternalNetIf, p, &m_ip4AddrInternalReceiver );

                    if ( nEspErr != ERR_OK && debugMatchMask( DebugErrors ) )
                    {
                        vlogError( "processExternalIp4: UDP internal output failed %d (%s)", nEspErr, lwip_strerr( nEspErr ) );
                    }

                    if ( p->ref > 0 && nEspErr == ESP_OK )
                    {
                        pbuf_free( p );
                    }

                    bProcessed = true;
                }
            }
            break;

            case IP_PROTO_UDPLITE:
            {
                if ( debugMatchMask( DebugSkips ) )
                {
                    vlogWarning( "processExternalIp4: skip UDPLITE package" );
                }
                bProcessed = true;
            }
            break;

            case IP_PROTO_TCP:
            {
                struct tcp_hdr * tcph = tcpHeader( iph );

                if ( debugMatchMask( DebugTcp ) )
                {
                    dumpIpHeader( iph );
                    dumpTcpHeader( tcph );
                }

                int nDstPort = ntohs( tcph->dest );
                int nSrcPort = ntohs( tcph->src );

                // block internal used ports
                if ( isInternalBlockedPort( nDstPort )
                                || isInternalBlockedPort( nSrcPort ) )
                {
                    if ( debugMatchMask( DebugTcp ) )
                    {
                        vlogWarning( "processExternalIp4: block TCP package by internal used port" );
                    }
                    bProcessed = true;
                }

                else if ( isBlockedByWhiteList( nDstPort, IpProtocolType::IpProtocolTcp ) )
                    //&& isBlockedByWhiteList( nSrcPort, IpProtocolType::IpProtocolTcp ) )
                {
                    if ( debugMatchMask( DebugTcp ) )
                    {
                        vlogWarning( "processExternalIp4: process TCP package in external interface (port %d) - white listed", nDstPort );
                    }
                }

                else
                {
                    if ( debugMatchMask( DebugTcp ) )
                    {
                        vlogInfo( "processExternalIp4: route TCP package to internal interface by NAT" );
                    }

                    processIncommingAddressTranslation( iph );
                    tcph = tcpHeader( iph );
                    tcph->chksum = calculateTcpCheckSumInIpPack( iph, ipPackageLen );

                    struct pbuf * p = pbuf_alloc_reference( iph, ipPackageLen, PBUF_REF );

                    nEspErr = m_pInternalNetIf->output( m_pInternalNetIf, p, &m_ip4AddrInternalReceiver );

                    if ( nEspErr != ERR_OK && debugMatchMask( DebugErrors ) )
                    {
                        vlogError( "processExternalIp4: TCP internal output failed %d (%s)", nEspErr, lwip_strerr( nEspErr ) );
                    }

                    if ( p->ref > 0 && nEspErr == ESP_OK )
                    {
                        pbuf_free( p );
                    }

                    bProcessed = true;
                }
            }
            break;

            case IP_PROTO_ICMP:
            {
                struct icmp_echo_hdr * icmpEchoHdr = icmpHeader( iph );

                switch ( ICMPH_TYPE( icmpEchoHdr ) )
                {
                    case ICMP_ECHO:
                    {
                        if ( m_bEnableIcmpReply )
                        {
                            if ( debugMatchMask( DebugIcmp ) )
                            {
                                vlogInfo( "processExternalIp4: process ICMP package for reply" );
                            }
                        }
                    }
                    break;

                    case ICMP_ER:
                    {
                        if ( debugMatchMask( DebugIcmp ) )
                        {
                            vlogInfo( "processExternalIp4: route ICMP reply to internal" );
                            vlogInfo( "from IP address %s", ip4addr_ntoa( &srcIp.u_addr.ip4 ) );
                        }

                        processIncommingAddressTranslation( iph );

                        // reduce ethernet package to none ethernet package
                        struct pbuf * p = pbuf_alloc_reference( iph, ipPackageLen, PBUF_REF );
                        nEspErr = m_pInternalNetIf->output( m_pInternalNetIf, p, &m_ip4AddrInternalReceiver );

                        if ( nEspErr != ERR_OK && debugMatchMask( DebugErrors ) )
                        {
                            vlogError( "processExternalIp4: ICMP internal input failed %d (%s)", nEspErr, lwip_strerr( nEspErr ) );
                        }

                        if ( p->ref > 0 && nEspErr == ESP_OK )
                        {
                            pbuf_free( p );
                        }

                        bProcessed = true;
                    }
                    break;

                    default:
                    {
                        if ( debugMatchMask( DebugSkips | DebugIcmp ) )
                        {
                            vlogWarning( "processExternalIp4: skip ICMP package (type %d)", ICMPH_TYPE( icmpEchoHdr ) );
                        }
                    }
                    break;
                }
            }
            break;

            case IP_PROTO_IGMP:
            {
                // skip IGMP data - not relevant herein
                if ( debugMatchMask( DebugSkips ) )
                {
                    vlogWarning( "processExternalIp4: skip IGMP package" );
                }
            }
            break;

            default:
            {
                vlogError( "processExternalIp4 unknown IPv4 protocol package %d not processed", iph->_proto );
            }
            break;
        }
    }

    return bProcessed;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

uint16_t NatRouter::calculateTcpCheckSumInIpPack( struct pbuf * pIpBuf )
{
    struct ip_hdr * iph = ( struct ip_hdr * ) ( ( uint8_t * ) pIpBuf->payload );

    ip_addr_t  dstIp;
    ip_addr_t  srcIp;

    ip_addr_copy_from_ip4( dstIp, iph->dest );
    ip_addr_copy_from_ip4( srcIp, iph->src );

    struct pbuf staticPBuf;
    memset( &staticPBuf, 0, sizeof( staticPBuf ) );

    staticPBuf.len     = pIpBuf->tot_len - sizeof( ip_hdr );
    staticPBuf.tot_len = staticPBuf.len;
    staticPBuf.payload = ( void * ) ( ( uint8_t *) iph + sizeof( ip_hdr ) );

    struct tcp_hdr * tcphNew = reinterpret_cast<struct tcp_hdr *>( staticPBuf.payload );

    tcphNew->chksum = 0;
    uint16_t u16Chksum = ip_chksum_pseudo( &staticPBuf,
                                           IP_PROTO_TCP,
                                           staticPBuf.tot_len,
                                           &srcIp,
                                           &dstIp );

    return u16Chksum;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

uint16_t NatRouter::calculateTcpCheckSumInIpPack( struct ip_hdr * iph,
                                                  const size_t uBufLen )
{
    ip_addr_t  dstIp;
    ip_addr_t  srcIp;

    ip_addr_copy_from_ip4( dstIp, iph->dest );
    ip_addr_copy_from_ip4( srcIp, iph->src );

    struct pbuf staticPBuf;
    memset( &staticPBuf, 0, sizeof( staticPBuf ) );

    staticPBuf.len     = uBufLen - sizeof( ip_hdr );
    staticPBuf.tot_len = staticPBuf.len;
    staticPBuf.payload = ( void * ) ( ( uint8_t *) iph + sizeof( ip_hdr ) );

    struct tcp_hdr * tcphNew = reinterpret_cast<struct tcp_hdr *>( staticPBuf.payload );

    tcphNew->chksum = 0;
    uint16_t u16Chksum = ip_chksum_pseudo( &staticPBuf,
                                           IP_PROTO_TCP,
                                           staticPBuf.tot_len,
                                           &srcIp,
                                           &dstIp );

    return u16Chksum;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

void NatRouter::enableIcmpReply( const bool bEnable )
{
    m_bEnableIcmpReply = bEnable;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

void NatRouter::enableDnsReply( const bool bEnable )
{
    m_bEnableDnsReply = true;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

bool NatRouter::isInternalBlockedPort( const int nPort ) const
{
    bool bIsInternal = false;

    for ( auto i : m_theInternalPorts )
    {
        if ( i == nPort )
        {
            bIsInternal = true;
            break;
        }
    }

    return bIsInternal;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::processIncommingAddressTranslation( struct ip_hdr* iph )
{
    if ( iph )
    {
        iph->dest.addr = m_ip4AddrInternalReceiver.addr;
        IPH_CHKSUM_SET( iph, 0 );
        IPH_CHKSUM_SET( iph, inet_chksum( iph, IPH_HL_BYTES( iph ) ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::processOutgoingAddressTranslation( struct ip_hdr * iph,
                                                   struct pbuf * p )
{
    if ( iph && m_pExternalNetIf )
    {
        iph->src.addr = m_pExternalNetIf->ip_info->ip.addr;

        IPH_CHKSUM_SET( iph, 0 );
        IPH_CHKSUM_SET( iph, inet_chksum( iph, IPH_HL_BYTES( iph ) ) );

        switch ( IPH_PROTO( iph ) )
        {
            case IP_PROTO_IGMP:
            case IP_PROTO_ICMP:
            case IP_PROTO_UDP:
            {
            }
            break;

            case IP_PROTO_TCP:
            {
                struct tcp_hdr * tcph = tcpHeader( iph );
                tcph->chksum = calculateTcpCheckSumInIpPack( p );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::processOutgoingAddressTranslation( struct ip_hdr * iph,
                                                   const size_t uBufLen )
{
    if ( iph && m_pExternalNetIf )
    {
        iph->src.addr = m_pExternalNetIf->ip_info->ip.addr;

        IPH_CHKSUM_SET( iph, 0 );
        IPH_CHKSUM_SET( iph, inet_chksum( iph, IPH_HL_BYTES( iph ) ) );

        switch ( IPH_PROTO( iph ) )
        {
            case IP_PROTO_UDP:
                break;

            case IP_PROTO_TCP:
            {
                struct tcp_hdr * tcph = tcpHeader( iph );
                tcph->chksum = calculateTcpCheckSumInIpPack( iph , uBufLen);
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::setDebugMask( const int nDebugMask )
{
    m_nDebugMask = nDebugMask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int NatRouter::debugMask( void ) const
{
    return m_nDebugMask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::dumpPbuf( const struct pbuf *p ) const
{
    printf( "dumpPbuf: pbuf len=%d tot_len=%d type=%d\n",
            p->len,
            p->tot_len,
            p->type_internal );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::dumpEthernetHeader( const struct eth_hdr * pEthHdr ) const
{
    if ( isDebugPort( ipHeader( pEthHdr ) ) )
    {
        printf( "ethernet\n"
                        "   mac src: %02x:%02x:%02x:%02x:%02x:%02x\n"
                        "   mac dst: %02x:%02x:%02x:%02x:%02x:%02x\n"
                        "   type: 0x%04x\n",
                        pEthHdr->src.addr[0],
                        pEthHdr->src.addr[1],
                        pEthHdr->src.addr[2],
                        pEthHdr->src.addr[3],
                        pEthHdr->src.addr[4],
                        pEthHdr->src.addr[5],

                        pEthHdr->dest.addr[0],
                        pEthHdr->dest.addr[1],
                        pEthHdr->dest.addr[2],
                        pEthHdr->dest.addr[3],
                        pEthHdr->dest.addr[4],
                        pEthHdr->dest.addr[5],

                        lwip_ntohs( pEthHdr->type ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::dumpIpHeader( const struct ip_hdr * pIpHdr ) const
{
    if ( isDebugPort( pIpHdr ) )
    {
        if ( IPH_V( pIpHdr ) != 4 )
        {
            vlogWarning( "dumpEthernetPackage: skip package protocol type %d (not IPv4)", IPH_V( pIpHdr ) );
        }
        else
        {
            ip_addr_t  dstIp;
            ip_addr_t  srcIp;

            ip_addr_copy_from_ip4( dstIp, pIpHdr->dest );
            ip_addr_copy_from_ip4( srcIp, pIpHdr->src );

            printf( "   IPv4 header\n"
                            "       src ip: %s\n",
                            ip4addr_ntoa( &srcIp.u_addr.ip4 ) );
            printf( "       dst ip: %s\n",
                    ip4addr_ntoa( &dstIp.u_addr.ip4 ) );
            printf( "       version: %d header len: %d total len: %d\n"
                            "       tos: 0x%02x\n"
                            "       id: 0x%04x\n"
                            "       flags: 0x%02x\n"
                            "       time to live: %d\n"
                            "       protocol: %d\n"
                            "       chksum: 0x%04x\n",
                            ( pIpHdr->_v_hl >> 4 ),
                            (pIpHdr->_v_hl & 0xf),
                            lwip_ntohs( pIpHdr->_len ),
                            pIpHdr->_tos,
                            lwip_ntohs( pIpHdr->_id ),
                            lwip_ntohs( pIpHdr->_offset ),
                            pIpHdr->_ttl,
                            pIpHdr->_proto,
                            lwip_ntohs( pIpHdr->_chksum ) );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::dumpTcpHeader( const struct tcp_hdr * pTcpHdr ) const
{
    if ( isDebugPort( pTcpHdr ) )
    {
        u16_t u16Flags = lwip_ntohs( pTcpHdr->_hdrlen_rsvd_flags ) & 0x3ff;
        printf( "           src->dst port: %d->%d\n"
                        "           sequence no: %u\n"
                        "           achnowledge no: %u\n"
                        "           hdr len: %d\n"
                        "           flags: 0x%04x (%s/%s/%s/%s/%s/%s/%s/%s)\n"
                        "           window: %d\n"
                        "           chksum: 0x%04x\n"
                        "           urgent ptr: 0x%04x\n",
                        lwip_ntohs( pTcpHdr->src ),
                        lwip_ntohs( pTcpHdr->dest ),
                        static_cast<unsigned int>( lwip_ntohl( pTcpHdr->seqno ) ),
                        static_cast<unsigned int>( lwip_ntohl( pTcpHdr->ackno ) ),
                        ( lwip_ntohs( pTcpHdr->_hdrlen_rsvd_flags ) >> 12 ) << 2,
                        u16Flags,
                        ( ( u16Flags & TCP_FIN ) ? "FIN" : "" ),
                        ( ( u16Flags & TCP_SYN ) ? "SYN" : "" ),
                        ( ( u16Flags & TCP_RST ) ? "PSH" : "" ),
                        ( ( u16Flags & TCP_PSH ) ? "PSH" : "" ),
                        ( ( u16Flags & TCP_ACK ) ? "ACK" : "" ),
                        ( ( u16Flags & TCP_URG ) ? "URG" : "" ),
                        ( ( u16Flags & TCP_ECE ) ? "ECE" : "" ),
                        ( ( u16Flags & TCP_CWR ) ? "CWR" : "" ),
                        lwip_ntohs( pTcpHdr->wnd ),
                        lwip_ntohs( pTcpHdr->chksum ),
                        lwip_ntohs( pTcpHdr->urgp )
        );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::dumpUdpHeader( const struct udp_hdr * pUdpHdr ) const
{
    if ( isDebugPort( pUdpHdr ) )
    {
        printf( "           src->dst port: %d->%d\n"
                        "           len: %d\n"
                        "           chksum: 0x%04x\n",
                        lwip_ntohs( pUdpHdr->src ),
                        lwip_ntohs( pUdpHdr->dest ),
                        lwip_ntohs( pUdpHdr->len ),
                        lwip_ntohs( pUdpHdr->chksum )
        );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::dumpIcmpHeader( const struct icmp_echo_hdr * pIcmpHdr ) const
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::addWhiteListPort( const int nPort,
                                  const IpProtocolType nIpProtocolType )
{
    WhiteListEntry entry = { .nPort = nPort, .nIpProtocolType = nIpProtocolType };
    m_theWhiteListPorts.push_back( entry );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool NatRouter::isBlockedByWhiteList( const int nPort,
                                      const IpProtocolType nIpProtocolType ) const
{
    bool bMatch = false;

    if ( m_theWhiteListPorts.empty() )
    {
        //bMatch = true;
    }
    else
    {
        for ( auto port : m_theWhiteListPorts )
        {
            bMatch = ( ( port.nPort == nPort ) && ( port.nIpProtocolType == nIpProtocolType ) );

            if ( bMatch )
            {
                break;
            }
        }
    }

    return bMatch;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool NatRouter::debugMatchMask( const int nDebugMask ) const
{
    return ( ( m_nDebugMask & nDebugMask ) != 0 );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void NatRouter::setDebugPort( const int nPort )
{
    m_nDebugPort = nPort;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool NatRouter::isDebugPort( const struct ip_hdr* pIpHdr ) const
{
    bool bSuccess = true;

    if ( m_nDebugPort != 0 )
    {
        bSuccess = false;

        switch ( IPH_PROTO( pIpHdr ) )
        {
            case IP_PROTO_UDP:
            {
                const udp_hdr * pUdp = udpHeader( pIpHdr );

                if ( pUdp->dest == m_nDebugPort
                                || pUdp->src == m_nDebugPort )
                {
                    bSuccess = true;
                }
            }
            break;

            case IP_PROTO_TCP:
            {
                const tcp_hdr * pTcp = tcpHeader( pIpHdr );

                if ( pTcp->dest == m_nDebugPort
                                || pTcp->src == m_nDebugPort )
                {
                    bSuccess = true;
                }
            }
            break;
        }
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool NatRouter::isDebugPort( const struct tcp_hdr* pTcpHdr ) const
{
    bool bSuccess = true;

    if ( m_nDebugPort != 0 )
    {
        bSuccess = false;

        if ( pTcpHdr->dest == m_nDebugPort
                        || pTcpHdr->src == m_nDebugPort )
        {
            bSuccess = true;
        }
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool NatRouter::isDebugPort( const struct udp_hdr* pUdpHdr ) const
{
    bool bSuccess = true;

    if ( m_nDebugPort != 0 )
    {
        bSuccess = false;

        if ( pUdpHdr->dest == m_nDebugPort
                        || pUdpHdr->src == m_nDebugPort )
        {
            bSuccess = true;
        }
    }

    return bSuccess;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

void NatRouter::blockInternalPort( const int nInternalPort )
{
    vlogInfo( "blockInternalPort %d", nInternalPort );
    m_theInternalPorts.insert( nInternalPort );
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

struct eth_hdr* NatRouter::ethHeader( struct pbuf* p ) const
{
    struct eth_hdr * ethh = nullptr;

    if ( p )
    {
        ethh = reinterpret_cast<struct eth_hdr *>( p->payload );
    }

    return ethh;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

struct eth_hdr* NatRouter::ethHeader( void * pBuffer ) const
{
    struct eth_hdr * ethh = nullptr;

    if ( pBuffer )
    {
        ethh = reinterpret_cast<struct eth_hdr *>( pBuffer );
    }

    return ethh;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

struct ip_hdr* NatRouter::ipHeader( struct pbuf* p ) const
{
    struct ip_hdr * iph = nullptr;

    if ( p )
    {
        iph = reinterpret_cast<struct ip_hdr *>( p->payload );
    }

    return iph;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

struct ip_hdr* NatRouter::ipHeader( void * pBuffer ) const
{
    struct ip_hdr * iph = nullptr;

    if ( pBuffer )
    {
        iph = reinterpret_cast<struct ip_hdr *>( pBuffer );
    }

    return iph;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

struct ip_hdr* NatRouter::ipHeader( struct eth_hdr* pEthHdr ) const
{
    struct ip_hdr * iph = nullptr;

    if ( pEthHdr )
    {
        iph = reinterpret_cast<struct ip_hdr *>( reinterpret_cast<uint8_t *>( pEthHdr ) + sizeof( struct eth_hdr ) );
    }

    return iph;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

const struct ip_hdr* NatRouter::ipHeader( const struct eth_hdr* pEthHdr ) const
{
    const struct ip_hdr * iph = nullptr;

    if ( pEthHdr )
    {
        iph = reinterpret_cast<const struct ip_hdr *>( reinterpret_cast<const uint8_t *>( pEthHdr ) + sizeof( struct eth_hdr ) );
    }

    return iph;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

struct tcp_hdr* NatRouter::tcpHeader( struct ip_hdr* pIpHdr ) const
{
    struct tcp_hdr * tcph = nullptr;

    if ( pIpHdr )
    {
        tcph = reinterpret_cast<struct tcp_hdr *>( reinterpret_cast<uint8_t *>( pIpHdr ) + sizeof( struct ip_hdr ) );
    }

    return tcph;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

const struct tcp_hdr* NatRouter::tcpHeader( const struct ip_hdr* pIpHdr ) const
{
    const struct tcp_hdr * tcph = nullptr;

    if ( pIpHdr )
    {
        tcph = reinterpret_cast<const struct tcp_hdr *>( reinterpret_cast<const uint8_t *>( pIpHdr ) + sizeof( struct ip_hdr ) );
    }

    return tcph;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

struct udp_hdr* NatRouter::udpHeader( struct ip_hdr* pIpHdr ) const
{
    struct udp_hdr * udph = nullptr;

    if ( pIpHdr )
    {
        udph = reinterpret_cast<struct udp_hdr *>( reinterpret_cast<uint8_t *>( pIpHdr ) + sizeof( struct ip_hdr ) );
    }

    return udph;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

const struct udp_hdr* NatRouter::udpHeader( const struct ip_hdr* pIpHdr ) const
{
    const struct udp_hdr * udph = nullptr;

    if ( pIpHdr )
    {
        udph = reinterpret_cast<const struct udp_hdr *>( reinterpret_cast<const uint8_t *>( pIpHdr ) + sizeof( struct ip_hdr ) );
    }

    return udph;
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

struct icmp_echo_hdr* NatRouter::icmpHeader( struct ip_hdr* pIpHdr ) const
{
    struct icmp_echo_hdr * icmph = nullptr;

    if ( pIpHdr )
    {
        icmph = reinterpret_cast<struct icmp_echo_hdr *>( reinterpret_cast<uint8_t *>( pIpHdr ) + sizeof( struct ip_hdr ) );
    }

    return icmph;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
