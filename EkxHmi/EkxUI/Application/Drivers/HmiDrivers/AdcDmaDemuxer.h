///////////////////////////////////////////////////////////////////////////////
///
/// @file AdcDmaDemuxer.h
///
/// @brief Header file of class AdcDmaDemuxer.
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

#ifndef AdcDmaDemuxer_h
#define AdcDmaDemuxer_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "AdcDmaReader.h"

#include <QMap>
#include <QMutex>

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class AdcDmaDemuxer : public AdcDmaReader
{
    Q_OBJECT

private:

    struct ChannelCallbacks
    {
        std::function<void()>    m_startDmaRead;
        std::function<void(int)> m_processAdcValue;
    };

public:

    explicit AdcDmaDemuxer( QObject * const pParent = nullptr );

    ~AdcDmaDemuxer() override;

    void create( const QString & strDmaDeviceFile,
                 const int       nDemuxerSize );

    void addChannel( const int                   nChannel,
                     std::function<void()>       startDmaRead,
                     std::function<void ( int )> processAdcValue );

private:

    void processAdcValue( const int nAdcValue );

    void processStartDmaRead();

signals:

private:

    QMutex                      m_mutexChannelChange;

    int                         m_nDemuxerSize { 0 };

    int                         m_nCurrentChannel { 0 };

    QMap<int, ChannelCallbacks> m_channelMap;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

#endif // AdcDmaDemuxer_h
