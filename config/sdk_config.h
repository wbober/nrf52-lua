#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

#define APP_TIMER_ENABLED 1
#define BUTTON_ENABLED    1

#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif

#ifndef PERIPHERAL_RESOURCE_SHARING_ENABLED
#define PERIPHERAL_RESOURCE_SHARING_ENABLED 0
#endif

#define GPIOTE_ENABLED                        1
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 1
#define GPIOTE_CONFIG_IRQ_PRIORITY            6

#define UART_ENABLED                          1
#define UART_DEFAULT_CONFIG_HWFC              0
// <0=> Excluded
// <14=> Included
#define UART_DEFAULT_CONFIG_PARITY            0

// <323584=> 1200 baud
// <643072=> 2400 baud
// <1290240=> 4800 baud
// <2576384=> 9600 baud
// <3862528=> 14400 baud
// <5152768=> 19200 baud
// <7716864=> 28800 baud
// <10289152=> 38400 baud
// <15400960=> 57600 baud
// <20615168=> 76800 baud
// <30801920=> 115200 baud
// <61865984=> 230400 baud
// <67108864=> 250000 baud
// <121634816=> 460800 baud
// <251658240=> 921600 baud
// <268435456=> 57600 baud
#define UART_DEFAULT_CONFIG_BAUDRATE 30801920

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7
#define UART_DEFAULT_CONFIG_IRQ_PRIORITY  6
#define UART0_CONFIG_USE_EASY_DMA         1
#define UART_EASY_DMA_SUPPORT             1
#define UART_LEGACY_SUPPORT               1

#define APP_FIFO_ENABLED                  1
#define APP_UART_ENABLED                  1
#define APP_UART_DRIVER_INSTANCE          0

#define RETARGET_ENABLED                  1

#define NRF_LOG_ENABLED                   1
#define NRF_LOG_BACKEND_SERIAL_USES_RTT   1
#define NRF_LOG_USES_COLORS               0

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug
#define NRF_LOG_DEFAULT_LEVEL             3
#define NRF_LOG_DEFERRED                  0
#define NRF_LOG_USES_TIMESTAMP            0
#define NRF_LOG_TIMESTAMP_DIGITS          8
#define NRF_LOG_BACKEND_MAX_STRING_LENGTH 256

#endif //SDK_CONFIG_H
