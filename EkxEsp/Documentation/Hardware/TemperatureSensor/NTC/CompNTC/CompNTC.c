/*
 * CompNTC.c
 *
 *  Created on: 30.07.2020
 *      Author: mschmidl
 */

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include "CompNTC.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
/*
 * For a 10K NTC (B=3988K) combined with a 10K Pullup to 3V3, the following applies between 1°C and 70°C:
 *
 *  T = ( (segOffset - AINmV) * 10 / segInclination ) + segStartTemp
 *
 *  AIN [mV]    segOffset   segInclination  segStartTemp
 *  ==========+===========+===============+==============
 *  2496        2464        266             2
 *  2194        2159        326             11
 *  1831        1795        323             21
 *  1474        1440        291             31
 *  1154        1124        244             41
 *  885         862         194             51
 *  672         654         149             61
 *
 *  AIN will then be 2,496V @1°C down to 0,509V @70°C with max 1°C deviation
 *  If higher temperatures will be measured the error will increase if not
 *  another segment gets introduced. At 90°C the error will be -5°C.
 *
 *  ATTENTION: the pullup should be choosen to get a maximum signal swing and
 *             not overloading the AIN (currently 3V0 max can be measured).
 *
 *  For a pullup of 3K3 - with less signal swing - the following parameters must be used:

 *  AIN [mV]    segOffset   segSteigung segStartT
 *  ==========+===========+===============+==============
 *  2983        2968        134             2
 *  2830        2810        196             11
 *  2609        2585        238             21
 *  2343        2314        267             31
 *  2045        2014        277             41
 *  1737        1707        267             51
 *  1441        1413        242             61
 *
 */

uint32_t getNtcTemperature( uint32_t mV )
{
    // mv < 2983//2496
    uint32_t segOffsetmV = 2968;//2464;
    uint32_t segInclination = 134;//266;
    uint32_t segStartTempC = 2;
    if ( mV < 2830 )//2194 )
    {
       segOffsetmV = 2810;//2159;
       segInclination = 196;//326;
       segStartTempC = 11;
    }
    if ( mV < 2609 )//1831 )
    {
       segOffsetmV = 2585;//1795;
       segInclination = 238;//323;
       segStartTempC = 21;
    }
    if ( mV < 2343 )//1474 )
    {
       segOffsetmV = 2314;//1440;
       segInclination = 267;//291;
       segStartTempC = 31;
    }
    if ( mV < 2045 )//1154 )
    {
       segOffsetmV = 2014;//1124;
       segInclination = 277;//244;
       segStartTempC = 41;
    }
    if ( mV < 1737 )//885 )
    {
       segOffsetmV = 1707;//862;
       segInclination = 267;//194;
       segStartTempC = 51;
    }
    if ( mV < 1441 )//672 )
    {
       segOffsetmV = 1413;//653;
       segInclination = 242;//149;
       segStartTempC = 61;
    }

    uint32_t T = ( ( ( ( segOffsetmV - mV ) * 10 ) / segInclination ) + segStartTempC ) - 1;
    printf( "mV=%d offset=%d inclination=%d start=%d -> T=%d\n", mV, segOffsetmV, segInclination, segStartTempC, T);
    return T;
}
