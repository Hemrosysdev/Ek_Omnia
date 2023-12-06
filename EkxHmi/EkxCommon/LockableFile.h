///////////////////////////////////////////////////////////////////////////////
///
/// @file LockableFile.h
///
/// @brief Header file of class LockableFile.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 17.02.2023
///
/// @copyright Copyright 2023 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef LockableFile_h
#define LockableFile_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QFile>
#include <QSharedPointer>

class QLockFile;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class LockableFile : public QFile
{
    Q_OBJECT

public:

    explicit LockableFile( QObject * parent = nullptr );
    explicit LockableFile( const QString & strFileName,
                           QObject *       parent = nullptr );
    virtual ~LockableFile();

    void setFileName( const QString & strFileName );

    bool open( OpenMode flags ) override;

    void close() override;

    bool remove();

    static bool remove( const QString & strFileName );

    const QString & lockFile() const;

    bool lock();

    void unlock();

private:

    QSharedPointer<QLockFile> m_pLockFile;

    QString                   m_strLockFile;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // LockableFile_h
