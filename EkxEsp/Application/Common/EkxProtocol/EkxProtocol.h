/*
 * EkxProtocol.h
 *
 *  Created on: 30.04.2023
 *      Author: gesser
 */

#ifndef EkxProtocol_h
#define EkxProtocol_h

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace EkxProtocol
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace ProtoApi
{

class DataBase;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Message
{
public:

    Message();
    virtual ~Message();

    void registerData( DataBase * const pData );

    const std::string & serializedData() const;

    void serialize() const;

    bool deserialize( const std::string & data ) const;

    bool deserialize( Message & message ) const;

    void serializeData( const void * const pData,
                        const uint32_t     u32Size ) const;

    bool deserializeData( void * const   pData,
                          const uint32_t u32Size ) const;

    uint32_t size() const;

protected:

    std::vector<DataBase *> m_dataList;

    mutable std::string     m_strSerializedData;

    mutable uint32_t        m_u32DeserializePos = 0;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class DataBase
{
public:

    DataBase( Message * const pMessage );
    virtual ~DataBase();

    virtual void serializeData() const = 0;

    bool deserializeData();

    virtual bool deserializeData( Message & message ) = 0;

    virtual uint32_t size() const = 0;

    const Message * message() const;

    Message * message();

private:

    Message * m_pMessage = nullptr;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class DataString : public DataBase
{
public:

    DataString( Message * const pMessage );

    void serializeData() const override;

    bool deserializeData( Message & message ) override;

    uint32_t size() const override;

    const std::string & value() const;

    void setValue( const std::string & strValue );

private:

    std::string m_strValue;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

template <typename type>
class DataFixed : public DataBase
{
public:

    DataFixed( Message * const pMessage ) : DataBase( pMessage )
    {
    }

    const type value() const
    {
        return m_value;
    }

    void setValue( const type value )
    {
        m_value = value;
    }

    void serializeData() const override
    {
        message()->serializeData( &m_value, sizeof( m_value ) );
    }

    bool deserializeData( Message & message ) override
    {
        return message.deserializeData( &m_value, sizeof( m_value ) );
    }

    uint32_t size() const override
    {
        return sizeof( type );
    }

private:

    type m_value = static_cast<type>( 0 );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace ProtoApi

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

enum class DriverId : uint8_t
{
    Broadcast = 0,
    SystemDriver,
    PduDcHallMotorDriver,
    AgsaStepperMotorDriver,
    EepromDriver,
    SpotLightDriver,
    StartStopBtnDriver,
    StartStopLightDriver,
    TempSensorDriver,
    WifiDriver,
    McuDriver,
    HttpServerDriver,
    SwUpdateDriver,

    Last
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace Payload
{

enum class PayloadType : uint8_t
{
    None = 0,
    Ack,
    Nack,
    Ping,
    Pong,
    Reset,
    DiscreteStatus,
    AdcStatus,
    DcHallStartMotor,
    DcHallStartMotorSteps,
    DcHallStopMotor,
    DcHallTestMotor,
    DcHallStatus,
    EepromFlash,
    EepromStatus,
    PwmFaderSettings,
    PwmFadeIn,
    PwmFadeOut,
    PwmSetDuty,
    StepperMotorStart,
    StepperMotorStartSteps,
    StepperMotorStop,
    StepperMotorChangeRunFrequency,
    StepperMotorStatus,
    RequestDriverStatus,
    WifiConfig,
    WifiStatus,
    JsonMessage,
    McuCommand,
    McuReply,
    FileDownloadStartRequest,
    FileDownloadStartResponse,
    FileDownloadDataRequest,
    FileDownloadDataResponse,
    FileUploadStart,
    FileUploadStop,
    FileUploadData,
    SwUpdateCommand,
    SwUpdateStatus,
    SystemStatus,

    Last
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class MessagePayload : public ProtoApi::Message
{
public:

    MessagePayload( const PayloadType payloadType );

    virtual MessagePayload * clone() const = 0;

    PayloadType payloadType() const;

private:

    PayloadType m_payloadType = PayloadType::None;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class MessagePayloadRef : public ProtoApi::DataBase
{
public:

    MessagePayloadRef( ProtoApi::Message * const pMessage );
    ~MessagePayloadRef() override;

    void serializeData() const override;

    bool deserializeData( ProtoApi::Message & message ) override;

    uint32_t size() const override;

    const MessagePayload * value() const;

    void setValue( MessagePayload * pValue );

    PayloadType payloadType() const;

private:

    PayloadType      m_payloadType = PayloadType::None;

    MessagePayload * m_pValue = nullptr;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Ack : public MessagePayload
{
public:

    Ack() : MessagePayload( PayloadType::Ack )
    {
    }

    MessagePayload * clone() const override
    {
        return new Ack( *this );
    }

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Nack : public MessagePayload
{
public:

    enum class NackReason : uint8_t
    {
        Unknown = 0,
        WrongCrc,
        DriverNotFound,
        NoBroadcastCommand,
        PayloadBroken,
        UnknownDriverCommand,
        WrongParameter,
        Timeout,
        FileUploadBusy,
        FileUploadOffset,
        FileUploadOverflow,
        FileUploadStorage,
        FileUploadTimeout,
        FileUploadIllegalState,
        FileUploadIllegalSize,
        FileUploadNotStarted,
        NoJsonObject
    };

public:

    Nack();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<NackReason> & nackReason();

    const ProtoApi::DataFixed<NackReason> & nackReason() const;

private:

    ProtoApi::DataFixed<NackReason> m_nackReason;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Reset : public MessagePayload
{
public:

    Reset() : MessagePayload( PayloadType::Reset )
    {
    }

    MessagePayload * clone() const override
    {
        return new Reset( *this );
    }

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Ping : public MessagePayload
{
public:

    Ping() : MessagePayload( PayloadType::Ping )
    {
    }

    MessagePayload * clone() const override
    {
        return new Ping( *this );
    }

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Pong : public MessagePayload
{
public:

    Pong() : MessagePayload( PayloadType::Pong )
    {
    }

    MessagePayload * clone() const override
    {
        return new Pong( *this );
    }

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class DcHallStartMotor : public MessagePayload
{
public:

    DcHallStartMotor();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<uint32_t> & pwmDuty10th();

    const ProtoApi::DataFixed<uint32_t> & pwmDuty10th() const;

    ProtoApi::DataFixed<uint32_t> & pwmFrequency();

    const ProtoApi::DataFixed<uint32_t> & pwmFrequency() const;

private:

    ProtoApi::DataFixed<uint32_t> m_pwmDuty10th;
    ProtoApi::DataFixed<uint32_t> m_pwmFrequency;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class DcHallStartMotorSteps : public MessagePayload
{
public:

    DcHallStartMotorSteps();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<uint32_t> & pwmDuty10th();

    const ProtoApi::DataFixed<uint32_t> & pwmDuty10th() const;

    ProtoApi::DataFixed<uint32_t> & pwmFrequency();

    const ProtoApi::DataFixed<uint32_t> & pwmFrequency() const;

    ProtoApi::DataFixed<uint32_t> & steps();

    const ProtoApi::DataFixed<uint32_t> & steps() const;

private:

    ProtoApi::DataFixed<uint32_t> m_pwmDuty10th;
    ProtoApi::DataFixed<uint32_t> m_pwmFrequency;
    ProtoApi::DataFixed<uint32_t> m_steps;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class DcHallStopMotor : public MessagePayload
{
public:

    DcHallStopMotor() : MessagePayload( PayloadType::DcHallStopMotor )
    {
    }

    MessagePayload * clone() const override
    {
        return new DcHallStopMotor( *this );
    }

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class DcHallTestMotor : public MessagePayload
{
public:

    DcHallTestMotor() : MessagePayload( PayloadType::DcHallTestMotor )
    {
    }

    MessagePayload * clone() const override
    {
        return new DcHallTestMotor( *this );
    }

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class DcHallStatus : public MessagePayload
{
public:

    DcHallStatus();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<bool> & valid();

    const ProtoApi::DataFixed<bool> & valid() const;

    ProtoApi::DataFixed<bool> & faultPinActive();

    const ProtoApi::DataFixed<bool> & faultPinActive() const;

    ProtoApi::DataFixed<bool> & motorRunning();

    const ProtoApi::DataFixed<bool> & motorRunning() const;

    ProtoApi::DataFixed<uint64_t> & motorRunTime();

    const ProtoApi::DataFixed<uint64_t> & motorRunTime() const;

    ProtoApi::DataFixed<bool> & motorTestOk();

    const ProtoApi::DataFixed<bool> & motorTestOk() const;

    ProtoApi::DataFixed<uint32_t> & stepCounter();

    const ProtoApi::DataFixed<uint32_t> & stepCounter() const;

private:

    ProtoApi::DataFixed<bool>     m_valid;
    ProtoApi::DataFixed<bool>     m_faultPinActive;
    ProtoApi::DataFixed<bool>     m_motorRunning;
    ProtoApi::DataFixed<uint64_t> m_motorRunTime;
    ProtoApi::DataFixed<bool>     m_motorTestOk;
    ProtoApi::DataFixed<uint32_t> m_stepCounter;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EepromStatus : public MessagePayload
{
public:

    EepromStatus();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<bool> & valid();

    const ProtoApi::DataFixed<bool> & valid() const;

    ProtoApi::DataString & productNo();

    const ProtoApi::DataString & productNo() const;

    ProtoApi::DataString & serialNo();

    const ProtoApi::DataString & serialNo() const;

private:

    ProtoApi::DataFixed<bool> m_valid;
    ProtoApi::DataString      m_productNo;
    ProtoApi::DataString      m_serialNo;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EepromFlash : public MessagePayload
{
public:

    EepromFlash();

    MessagePayload * clone() const override;

    ProtoApi::DataString & productNo();

    const ProtoApi::DataString & productNo() const;

    ProtoApi::DataString & serialNo();

    const ProtoApi::DataString & serialNo() const;

private:

    ProtoApi::DataString m_productNo;
    ProtoApi::DataString m_serialNo;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PwmSetDuty : public MessagePayload
{
public:

    PwmSetDuty();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<uint32_t> & pwmDuty();

    const ProtoApi::DataFixed<uint32_t> & pwmDuty() const;

private:

    ProtoApi::DataFixed<uint32_t> m_pwmDuty;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PwmFadeIn : public MessagePayload
{
public:

    PwmFadeIn() : MessagePayload( PayloadType::PwmFadeIn )
    {
    }

    MessagePayload * clone() const override
    {
        return new PwmFadeIn( *this );
    }

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PwmFadeOut : public MessagePayload
{
public:

    PwmFadeOut() : MessagePayload( PayloadType::PwmFadeOut )
    {
    }

    MessagePayload * clone() const override
    {
        return new PwmFadeOut( *this );
    }

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PwmFaderSettings : public MessagePayload
{
public:

    PwmFaderSettings();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<uint32_t> & fadeInDutyPercent();

    const ProtoApi::DataFixed<uint32_t> & fadeInDutyPercent() const;

    ProtoApi::DataFixed<uint32_t> & fadeOutDutyPercent();

    const ProtoApi::DataFixed<uint32_t> & fadeOutDutyPercent() const;

    ProtoApi::DataFixed<uint32_t> & fadeInTimeMs();

    const ProtoApi::DataFixed<uint32_t> & fadeInTimeMs() const;

    ProtoApi::DataFixed<uint32_t> & fadeOutTimeMs();

    const ProtoApi::DataFixed<uint32_t> & fadeOutTimeMs() const;

private:

    ProtoApi::DataFixed<uint32_t> m_fadeInDutyPercent;
    ProtoApi::DataFixed<uint32_t> m_fadeOutDutyPercent;
    ProtoApi::DataFixed<uint32_t> m_fadeInTimeMs;
    ProtoApi::DataFixed<uint32_t> m_fadeOutTimeMs;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class DiscreteStatus : public MessagePayload
{
public:

    DiscreteStatus();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<bool> & valid();

    const ProtoApi::DataFixed<bool> & valid() const;

    ProtoApi::DataFixed<bool> & discreteHigh();

    const ProtoApi::DataFixed<bool> & discreteHigh() const;

private:

    ProtoApi::DataFixed<bool> m_valid;
    ProtoApi::DataFixed<bool> m_discreteHigh;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class StepperMotorStart : public MessagePayload
{
public:

    StepperMotorStart();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<bool> & direction();

    const ProtoApi::DataFixed<bool> & direction() const;

    ProtoApi::DataFixed<uint32_t> & startFrequency();

    const ProtoApi::DataFixed<uint32_t> & startFrequency() const;

    ProtoApi::DataFixed<uint32_t> & runFrequency();

    const ProtoApi::DataFixed<uint32_t> & runFrequency() const;

    ProtoApi::DataFixed<uint32_t> & rampFreqPerStep();

    const ProtoApi::DataFixed<uint32_t> & rampFreqPerStep() const;

private:

    ProtoApi::DataFixed<bool>     m_direction;
    ProtoApi::DataFixed<uint32_t> m_startFrequency;
    ProtoApi::DataFixed<uint32_t> m_runFrequency;
    ProtoApi::DataFixed<uint32_t> m_rampFreqPerStep;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class StepperMotorStartSteps : public MessagePayload
{
public:

    StepperMotorStartSteps();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<bool> & direction();

    const ProtoApi::DataFixed<bool> & direction() const;

    ProtoApi::DataFixed<uint32_t> & steps();

    const ProtoApi::DataFixed<uint32_t> & steps() const;

    ProtoApi::DataFixed<uint32_t> & startFrequency();

    const ProtoApi::DataFixed<uint32_t> & startFrequency() const;

    ProtoApi::DataFixed<uint32_t> & runFrequency();

    const ProtoApi::DataFixed<uint32_t> & runFrequency() const;

    ProtoApi::DataFixed<uint32_t> & rampFreqPerStep();

    const ProtoApi::DataFixed<uint32_t> & rampFreqPerStep() const;

private:

    ProtoApi::DataFixed<bool>     m_direction;
    ProtoApi::DataFixed<uint32_t> m_steps;
    ProtoApi::DataFixed<uint32_t> m_startFrequency;
    ProtoApi::DataFixed<uint32_t> m_runFrequency;
    ProtoApi::DataFixed<uint32_t> m_rampFreqPerStep;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class StepperMotorStop : public MessagePayload
{
public:

    StepperMotorStop() : MessagePayload( PayloadType::StepperMotorStop )
    {
    }

    MessagePayload * clone() const override
    {
        return new StepperMotorStop( *this );
    }

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class StepperMotorChangeRunFrequency : public MessagePayload
{
public:

    StepperMotorChangeRunFrequency();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<uint32_t> & runFrequency();

    const ProtoApi::DataFixed<uint32_t> & runFrequency() const;

private:

    ProtoApi::DataFixed<uint32_t> m_runFrequency;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class StepperMotorStatus : public MessagePayload
{
public:

    StepperMotorStatus();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<bool> & valid();

    const ProtoApi::DataFixed<bool> & valid() const;

    ProtoApi::DataFixed<bool> & motorRunning();

    const ProtoApi::DataFixed<bool> & motorRunning() const;

    ProtoApi::DataFixed<bool> & faultPinActive();

    const ProtoApi::DataFixed<bool> & faultPinActive() const;

private:

    ProtoApi::DataFixed<bool> m_valid;
    ProtoApi::DataFixed<bool> m_motorRunning;
    ProtoApi::DataFixed<bool> m_faultPinActive;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class AdcStatus : public MessagePayload
{
public:

    AdcStatus();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<bool> & valid();

    const ProtoApi::DataFixed<bool> & valid() const;

    ProtoApi::DataFixed<uint32_t> & adcVoltage();

    const ProtoApi::DataFixed<uint32_t> & adcVoltage() const;

private:

    ProtoApi::DataFixed<bool>     m_valid;
    ProtoApi::DataFixed<uint32_t> m_adcVoltage;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

enum class WifiMode : uint8_t
{
    Off = 0,
    Ap,
    ApSta,
    Sta,

    Last
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class WifiConfig : public MessagePayload
{
public:

    WifiConfig();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<WifiMode> & wifiMode();

    const ProtoApi::DataFixed<WifiMode> & wifiMode() const;

    ProtoApi::DataString & apSsid();

    const ProtoApi::DataString & apSsid() const;

    ProtoApi::DataString & apPassword();

    const ProtoApi::DataString & apPassword() const;

    ProtoApi::DataString & apIp();

    const ProtoApi::DataString & apIp() const;

    ProtoApi::DataString & staSsid();

    const ProtoApi::DataString & staSsid() const;

    ProtoApi::DataString & staPassword();

    const ProtoApi::DataString & staPassword() const;

    ProtoApi::DataFixed<bool> & staDhcp();

    const ProtoApi::DataFixed<bool> & staDhcp() const;

    ProtoApi::DataString & staStaticIp();

    const ProtoApi::DataString & staStaticIp() const;

    ProtoApi::DataString & staStaticGateway();

    const ProtoApi::DataString & staStaticGateway() const;

    ProtoApi::DataString & staStaticNetmask();

    const ProtoApi::DataString & staStaticNetmask() const;

    ProtoApi::DataString & staStaticDns();

    const ProtoApi::DataString & staStaticDns() const;

private:

    ProtoApi::DataFixed<WifiMode> m_wifiMode;
    ProtoApi::DataString          m_apSsid;
    ProtoApi::DataString          m_apPassword;
    ProtoApi::DataString          m_apIp;
    ProtoApi::DataString          m_staSsid;
    ProtoApi::DataString          m_staPassword;
    ProtoApi::DataFixed<bool>     m_staDhcp;
    ProtoApi::DataString          m_staStaticIp;
    ProtoApi::DataString          m_staStaticGateway;
    ProtoApi::DataString          m_staStaticNetmask;
    ProtoApi::DataString          m_staStaticDns;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class WifiStatus : public MessagePayload
{
public:

    WifiStatus();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<bool> & valid();

    const ProtoApi::DataFixed<bool> & valid() const;

    ProtoApi::DataString & apIp();

    const ProtoApi::DataString & apIp() const;

    ProtoApi::DataString & apMac();

    const ProtoApi::DataString & apMac() const;

    ProtoApi::DataString & staIp();

    const ProtoApi::DataString & staIp() const;

    ProtoApi::DataString & staMac();

    const ProtoApi::DataString & staMac() const;

private:

    ProtoApi::DataFixed<bool> m_valid;
    ProtoApi::DataString      m_apIp;
    ProtoApi::DataString      m_apMac;
    ProtoApi::DataString      m_staIp;
    ProtoApi::DataString      m_staMac;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class JsonMessage : public MessagePayload
{
public:

    JsonMessage();

    MessagePayload * clone() const override;

    ProtoApi::DataString & json();

    const ProtoApi::DataString & json() const;

    ProtoApi::DataFixed<uint32_t> & refMsgCounter();

    const ProtoApi::DataFixed<uint32_t> & refMsgCounter() const;

private:

    ProtoApi::DataString          m_json;

    ProtoApi::DataFixed<uint32_t> m_refMsgCounter;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class RequestDriverStatus : public MessagePayload
{
public:

    RequestDriverStatus() : MessagePayload( PayloadType::RequestDriverStatus )
    {
    }

    MessagePayload * clone() const override;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

enum class FileDownloadStatus : uint8_t
{
    OkOrContinue = 0,
    FilePreparationError,
    FileNotExisting,
    ReadError,
    ReadEndOfFile,
    ReadErrorSetOffset,
    ReadErrorSizeOvershoot,
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FileDownloadStartRequest : public MessagePayload
{
public:

    FileDownloadStartRequest();

    MessagePayload * clone() const override;

    ProtoApi::DataString & fileName();

    const ProtoApi::DataString & fileName() const;

private:

    ProtoApi::DataString m_fileName;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FileDownloadStartResponse : public MessagePayload
{
public:

    FileDownloadStartResponse();

    MessagePayload * clone() const override;

    ProtoApi::DataString & fileName();

    const ProtoApi::DataString & fileName() const;

    ProtoApi::DataFixed<uint32_t> & fileSize();

    const ProtoApi::DataFixed<uint32_t> & fileSize() const;

    ProtoApi::DataFixed<FileDownloadStatus> & status();

    const ProtoApi::DataFixed<FileDownloadStatus> & status() const;

    ProtoApi::DataFixed<uint32_t> & refMsgCounter();

    const ProtoApi::DataFixed<uint32_t> & refMsgCounter() const;

private:

    ProtoApi::DataString                    m_fileName;
    ProtoApi::DataFixed<uint32_t>           m_fileSize;
    ProtoApi::DataFixed<FileDownloadStatus> m_status;
    ProtoApi::DataFixed<uint32_t>           m_refMsgCounter;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FileDownloadDataRequest : public MessagePayload
{
public:

    FileDownloadDataRequest();

    MessagePayload * clone() const override;

    ProtoApi::DataString & fileName();

    const ProtoApi::DataString & fileName() const;

    ProtoApi::DataFixed<uint32_t> & chunkOffset();

    const ProtoApi::DataFixed<uint32_t> & chunkOffset() const;

    ProtoApi::DataFixed<uint32_t> & chunkSize();

    const ProtoApi::DataFixed<uint32_t> & chunkSize() const;

private:

    ProtoApi::DataString          m_fileName;
    ProtoApi::DataFixed<uint32_t> m_chunkOffset;
    ProtoApi::DataFixed<uint32_t> m_chunkSize;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FileDownloadDataResponse : public MessagePayload
{
public:

    FileDownloadDataResponse();

    MessagePayload * clone() const override;

    ProtoApi::DataString & fileName();

    const ProtoApi::DataString & fileName() const;

    ProtoApi::DataFixed<uint32_t> & chunkOffset();

    const ProtoApi::DataFixed<uint32_t> & chunkOffset() const;

    ProtoApi::DataString & chunkData();

    const ProtoApi::DataString & chunkData() const;

    ProtoApi::DataFixed<FileDownloadStatus> & status();

    const ProtoApi::DataFixed<FileDownloadStatus> & status() const;

    ProtoApi::DataFixed<uint32_t> & refMsgCounter();

    const ProtoApi::DataFixed<uint32_t> & refMsgCounter() const;

private:

    ProtoApi::DataString                    m_fileName;
    ProtoApi::DataFixed<uint32_t>           m_chunkOffset;
    ProtoApi::DataString                    m_chunkData;
    ProtoApi::DataFixed<FileDownloadStatus> m_status;
    ProtoApi::DataFixed<uint32_t>           m_refMsgCounter;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FileUploadStart : public MessagePayload
{
public:

    FileUploadStart();

    MessagePayload * clone() const override;

    ProtoApi::DataString & fileName();

    const ProtoApi::DataString & fileName() const;

    ProtoApi::DataFixed<uint32_t> & fileSize();

    const ProtoApi::DataFixed<uint32_t> & fileSize() const;

private:

    ProtoApi::DataString          m_fileName;
    ProtoApi::DataFixed<uint32_t> m_fileSize;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FileUploadStop : public MessagePayload
{
public:

    enum class StopReason : uint8_t
    {
        Unknown = 0,
        Finished,
        UserAbort,
        DataTimeout,
    };

public:

    FileUploadStop();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<StopReason> & stopReason();

    const ProtoApi::DataFixed<StopReason> & stopReason() const;

    ProtoApi::DataFixed<uint32_t> & transferedSize();

    const ProtoApi::DataFixed<uint32_t> & transferedSize() const;

private:

    ProtoApi::DataFixed<StopReason> m_stopReason;
    ProtoApi::DataFixed<uint32_t>   m_transferedSize;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FileUploadData : public MessagePayload
{
public:

    FileUploadData();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<uint32_t> & chunkNo();

    const ProtoApi::DataFixed<uint32_t> & chunkNo() const;

    ProtoApi::DataFixed<uint32_t> & chunkOffset();

    const ProtoApi::DataFixed<uint32_t> & chunkOffset() const;

    ProtoApi::DataString & chunkData();

    const ProtoApi::DataString & chunkData() const;

private:

    ProtoApi::DataFixed<uint32_t> m_chunkNo;
    ProtoApi::DataFixed<uint32_t> m_chunkOffset;
    ProtoApi::DataString          m_chunkData;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

enum class SwUpdateCmd : uint8_t
{
    Idle = 0,
    Start,
    Data,
    Finish,
    Abort,
    Reboot
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class SwUpdateCommand : public MessagePayload
{
public:

    SwUpdateCommand();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<SwUpdateCmd> & cmd();

    const ProtoApi::DataFixed<SwUpdateCmd> & cmd() const;

    ProtoApi::DataFixed<uint32_t> & transactionId();

    const ProtoApi::DataFixed<uint32_t> & transactionId() const;

    ProtoApi::DataFixed<uint32_t> & chunkNo();

    const ProtoApi::DataFixed<uint32_t> & chunkNo() const;

    ProtoApi::DataString & chunkData();

    const ProtoApi::DataString & chunkData() const;

private:

    ProtoApi::DataFixed<SwUpdateCmd> m_cmd;
    ProtoApi::DataFixed<uint32_t>    m_transactionId;
    ProtoApi::DataFixed<uint32_t>    m_chunkNo;
    ProtoApi::DataString             m_chunkData;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class SwUpdateStatus : public MessagePayload
{
public:

    SwUpdateStatus();

    MessagePayload * clone() const override;

    ProtoApi::DataFixed<bool> & valid();

    const ProtoApi::DataFixed<bool> & valid() const;

    ProtoApi::DataFixed<SwUpdateCmd> & cmd();

    const ProtoApi::DataFixed<SwUpdateCmd> & cmd() const;

    ProtoApi::DataFixed<bool> & success();

    const ProtoApi::DataFixed<bool> & success() const;

    ProtoApi::DataFixed<uint32_t> & transactionId();

    const ProtoApi::DataFixed<uint32_t> & transactionId() const;

    ProtoApi::DataString & swVersionNo();

    const ProtoApi::DataString & swVersionNo() const;

private:

    ProtoApi::DataFixed<bool>        m_valid;
    ProtoApi::DataFixed<SwUpdateCmd> m_cmd;
    ProtoApi::DataFixed<bool>        m_success;
    ProtoApi::DataFixed<uint32_t>    m_transactionId;
    ProtoApi::DataString             m_swVersionNo;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class McuCommand : public MessagePayload
{
public:

    McuCommand();

    MessagePayload * clone() const override;

    ProtoApi::DataString & command();

    const ProtoApi::DataString & command() const;

private:

    ProtoApi::DataString m_command;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class McuReply : public MessagePayload
{
public:

    McuReply();

    MessagePayload * clone() const override;

    ProtoApi::DataString & reply();

    const ProtoApi::DataString & reply() const;

private:

    ProtoApi::DataString m_reply;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class SystemStatus : public MessagePayload
{
public:

    enum class Status : uint8_t
    {
        Normal = 0,
                        HeapWarning,
                        HeapErrorReboot,
    };

public:

    SystemStatus();

    SystemStatus * clone() const override;

    ProtoApi::DataFixed<Status>    & status();
    const ProtoApi::DataFixed<Status>    & status() const;
    ProtoApi::DataFixed<uint32_t>  & freeHeapSize();
    const ProtoApi::DataFixed<uint32_t>  & freeHeapSize() const;
    ProtoApi::DataFixed<uint32_t>  & messageNumTx();
    const ProtoApi::DataFixed<uint32_t>  & messageNumTx() const;
    ProtoApi::DataFixed<uint32_t>  & messageNumRx();
    const ProtoApi::DataFixed<uint32_t>  & messageNumRx() const;
    ProtoApi::DataFixed<uint32_t>  & messageNumRxErrors();
    const ProtoApi::DataFixed<uint32_t>  & messageNumRxErrors() const;
    ProtoApi::DataFixed<uint32_t>  & messageNumTxTimeouts();
    const ProtoApi::DataFixed<uint32_t>  & messageNumTxTimeouts() const;

private:

    ProtoApi::DataFixed<Status>      m_status;
    ProtoApi::DataFixed<uint32_t>    m_freeHeapSize;
    ProtoApi::DataFixed<uint32_t>    m_messageNumTx;
    ProtoApi::DataFixed<uint32_t>    m_messageNumRx;
    ProtoApi::DataFixed<uint32_t>    m_messageNumRxErrors;
    ProtoApi::DataFixed<uint32_t>    m_messageNumTxTimeouts;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const char * payloadTypeString( const Payload::PayloadType payloadType );

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace Payload

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class MessageFrame : public ProtoApi::Message
{
public:

    static constexpr uint32_t c_u32Preamble = 0x5aa55aa5;

public:

    MessageFrame();

    ProtoApi::DataFixed<uint32_t> & preamble();

    const ProtoApi::DataFixed<uint32_t> & preamble() const;

    ProtoApi::DataFixed<uint32_t> & length();

    const ProtoApi::DataFixed<uint32_t> & length() const;

    ProtoApi::DataFixed<DriverId> & driverId();

    const ProtoApi::DataFixed<DriverId> & driverId() const;

    ProtoApi::DataFixed<uint32_t> & msgCounter();

    const ProtoApi::DataFixed<uint32_t> & msgCounter() const;

    ProtoApi::DataFixed<uint8_t> & repeatCounter();

    const ProtoApi::DataFixed<uint8_t> & repeatCounter() const;

    Payload::MessagePayloadRef & payloadRef();

    const Payload::MessagePayloadRef & payloadRef() const;

    ProtoApi::DataFixed<uint32_t> & crc32();

    const ProtoApi::DataFixed<uint32_t> & crc32() const;

private:

    ProtoApi::DataFixed<uint32_t> m_preamble;
    ProtoApi::DataFixed<uint32_t> m_length;
    ProtoApi::DataFixed<DriverId> m_driverId;
    ProtoApi::DataFixed<uint32_t> m_msgCounter;
    ProtoApi::DataFixed<uint8_t>  m_repeatCounter;
    Payload::MessagePayloadRef    m_payloadRef;
    ProtoApi::DataFixed<uint32_t> m_crc32;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Serializer
{
public:

    Serializer( std::function<void ( const std::string &, MessageFrame & )> calcCheckSum );

    void serialize( MessageFrame & message );

    const std::string & serializedData() const;

private:

    std::function<void ( const std::string &, MessageFrame & )> m_calcCheckSum;

    std::string                                                 m_strSerializedData;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class Deserializer
{
public:

    enum class ParseResult
    {
        NoPreamble = 0,
        Incomplete,
        Invalid,
        WrongCrc,
        Success,
    };

public:

    Deserializer( std::function<void ( const std::string &,
                                       MessageFrame & )> calcCheckSum );

    void addStreamData( const uint8_t * pData,
                        const uint32_t  u32Size );

    void flushStreamData();

    bool hasData() const;

    ParseResult parseData();

    const MessageFrame & messageFrame() const;

    MessageFrame & messageFrame();

    const std::string & streamData() const;

private:

    std::function<void ( const std::string &, MessageFrame & )> m_calcCheckSum;

    std::string                                                 m_strStreamData;

    MessageFrame                                                m_messageFrame;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace EkxProtocol

#endif /* EkxProtocol_h */
