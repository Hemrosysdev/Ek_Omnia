///////////////////////////////////////////////////////////////////////////////
///
/// @file AdcDmaReader.h
///
/// @brief Header file of class AdcDmaReader.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 16.05.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef AdcDmaReader_h
#define AdcDmaReader_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QThread>
#include <QSemaphore>
#include <QSharedPointer>

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class AdcDmaReader : public QThread
{
    Q_OBJECT

private:

    static constexpr int c_nBufferSize = 200;

public:

    explicit AdcDmaReader( QObject * const pParent = nullptr );

    ~AdcDmaReader() override;

    void create( const QString &             strDmaDeviceFile,
                 std::function<void()>       startDmaRead,
                 std::function<void ( int )> processAdcValue );

    void disable();

    void simulateFromLogFile( const QString & strLogFile );

private:

    void run() override;

signals:

    void openDmaDeviceFileFailed();

private:

    QString                  m_strDmaFile;

    QSemaphore               m_semStopThread;

    std::function<void()>    m_startDmaRead;

    std::function<void(int)> m_processAdcValue;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

#endif // AdcDmaReader_h
