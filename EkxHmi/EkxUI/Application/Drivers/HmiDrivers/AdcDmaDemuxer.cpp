///////////////////////////////////////////////////////////////////////////////
///
/// @file AdcDmaDemuxer.cpp
///
/// @brief main application entry point of AdcDmaDemuxer.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 20.02.2023
///
/// @copyright Copyright 2023 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "AdcDmaDemuxer.h"

#include <QDebug>
#include <QMutexLocker>

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AdcDmaDemuxer::AdcDmaDemuxer( QObject * const pParent )
    : AdcDmaReader( pParent )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AdcDmaDemuxer::~AdcDmaDemuxer()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcDmaDemuxer::create( const QString & strDmaDeviceFile,
                            const int       nDemuxerSize )
{
    QMutexLocker locker( &m_mutexChannelChange );

    m_nDemuxerSize    = nDemuxerSize;
    m_nCurrentChannel = 0;

    auto processAdcValueFunc = [this]( const int nAdcValue )
                               {
                                   processAdcValue( nAdcValue );
                               };
    auto processStartDmaReadFunc = [this]()
                                   {
                                       processStartDmaRead();
                                   };
    AdcDmaReader::create( strDmaDeviceFile,
                          processStartDmaReadFunc,
                          processAdcValueFunc );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcDmaDemuxer::addChannel( const int                 nChannel,
                                std::function<void()>     startDmaRead,
                                std::function<void (int)> processAdcValue )
{
    QMutexLocker locker( &m_mutexChannelChange );

    m_channelMap[nChannel] = ChannelCallbacks{ startDmaRead, processAdcValue };
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcDmaDemuxer::processAdcValue( const int nAdcValue )
{
    QMutexLocker locker( &m_mutexChannelChange );

    if ( m_channelMap.find( m_nCurrentChannel ) == m_channelMap.end() )
    {
        //qDebug() << "processAdcValue didn't find key" << m_nCurrentChannel;
    }
    else
    {
        m_channelMap[m_nCurrentChannel].m_processAdcValue( nAdcValue );
    }
    m_nCurrentChannel++;

    if ( m_nDemuxerSize == m_nCurrentChannel )
    {
        m_nCurrentChannel = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcDmaDemuxer::processStartDmaRead()
{
    QMutexLocker locker( &m_mutexChannelChange );

    m_nCurrentChannel = 0;

    foreach( ChannelCallbacks callbacks, m_channelMap )
    {
        callbacks.m_startDmaRead();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo
