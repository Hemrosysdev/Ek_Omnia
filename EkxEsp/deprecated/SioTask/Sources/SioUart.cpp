/*
 * uart_sio.c
 *
 *  Created on: 19.05.2020
 *      Author: gesser
 */

/*
 * This is the interface to the platform specific serial IO module
 * It needs to be implemented by those platforms which need SLIP or PPP
 */

#include <lwip/sio.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_log.h>

//static const sio_fd_t sioFdFail = NULL;
//static const sio_fd_t sioFdSuccess = ( void * ) 1;

const char * TAG = "SioUart";

/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open( u8_t devnum )
{
    ESP_LOGI( TAG, "sio_open %d", devnum );

    ESP_ERROR_CHECK( devnum >= UART_NUM_MAX );

    return ( sio_fd_t ) ( u32_t ) devnum;
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send( u8_t c,
               sio_fd_t fd )
{
    uart_port_t uartDevNum = ( uart_port_t ) fd;

    ESP_ERROR_CHECK( uartDevNum >= UART_NUM_MAX );

    uart_write_bytes( uartDevNum,
                      ( const char * ) &c,
                      1 );
}

/**
 * Receives a single character from the serial device.
 *
 * @param fd serial device handle
 *
 * @note This function will block until a character is received.
 */
//u8_t sio_recv(sio_fd_t fd)
//{
//    return 0;
//}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
    uart_port_t uartDevNum = ( uart_port_t ) fd;

    ESP_ERROR_CHECK( uartDevNum >= UART_NUM_MAX );

    int nReturn = uart_read_bytes( uartDevNum,
                                   data,
                                   len,
                                   portMAX_DELAY);

    ESP_LOGI( TAG, "sio_read %d", nReturn );

    if ( nReturn < 0 )
        nReturn = 0;

    return ( u32_t ) nReturn;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
    uart_port_t uartDevNum = ( uart_port_t ) fd;

    ESP_ERROR_CHECK( uartDevNum >= UART_NUM_MAX );

    int nReturn = uart_read_bytes( uartDevNum,
                                   data,
                                   len,
                                   10 );

//    if ( nReturn > 0 )
//    {
//        ESP_LOGI( TAG, "sio_tryread %d %d %d", uartDevNum, len, nReturn );
//    }

    if ( nReturn < 0 )
        nReturn = 0;

    return ( u32_t ) nReturn;
}

/**
 * Writes to the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data to send
 * @param len length (in bytes) of data to send
 * @return number of bytes actually sent
 *
 * @note This function will block until all data can be sent.
 */
u32_t sio_write(sio_fd_t fd, u8_t *data, u32_t len)
{
    uart_port_t uartDevNum = ( uart_port_t ) fd;

    ESP_ERROR_CHECK( uartDevNum >= UART_NUM_MAX );

    ESP_LOGI( TAG, "sio_write %d", static_cast<int>( len ) );

    int nReturn = uart_write_bytes( uartDevNum,
                                    ( const char * ) data,
                                    len );

    if ( nReturn < 0 )
        nReturn = 0;

    return ( u32_t ) nReturn;
}

/**
 * Aborts a blocking sio_read() call.
 *
 * @param fd serial device handle
 */
//void sio_read_abort(sio_fd_t fd)
//{
//
//}

