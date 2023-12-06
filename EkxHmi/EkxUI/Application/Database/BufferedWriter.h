///////////////////////////////////////////////////////////////////////////////
///
/// @file BufferedWriter.h
///
/// @brief Header file of class BufferedWriter.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 12.04.2023
///
/// @copyright Copyright 2023 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef BufferedWriter_h
#define BufferedWriter_h

#include <QObject>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class BufferedWriter : public QObject
{
    Q_OBJECT

public:

    BufferedWriter();

    ~BufferedWriter() override;

    void create( const QString &           strBufferFile,
                 std::function<void (int)> writerFunction,
                 const int                 nThreshold );

    const QString & bufferFile() const;

    void addDelta( const int nDelta );

    int value() const;

    void flush();

private:

    bool writeBufferFile();

    void readBufferFile();

private:

    QString                   m_strBufferFile;

    std::function<void (int)> m_writerFunction;

    int                       m_nThreshold { 0 };

    int                       m_nValue { 0 };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // BufferedWriter_h
