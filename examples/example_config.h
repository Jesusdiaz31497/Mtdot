#ifndef __EXAMPLE__CONFIG_H__
#define __EXAMPLE__CONFIG_H__

#define OTA_EXAMPLE              1  // see ota_example.cpp
#define CONDE_PM         2  // see auto_ota_example.cpp
#define MANUAL_EXAMPLE           3  // see manual_example.cpp
#define CENILOGGER_RS485     4  // see peer_to_peer_example.cpp
#define CLASS_C_EXAMPLE          5  // see class_c_example.cpp
#define SERIAL          6  // see class_b_example.cpp
#define SERIAL_FP             7  // see fota_example.cpp
#define PLUVIOMETRO              8
#define CLASS_C                  9

// the active example is the one that will be compiled
#if !defined(ACTIVE_EXAMPLE)
#define ACTIVE_EXAMPLE  SERIAL
#endif

// the active channel plan is the one that will be compiled
// options are :
//      CP_US915
//      CP_AU915
//      CP_EU868
//      CP_KR920
//      CP_AS923
//      CP_AS923_JAPAN
//      CP_IN865
#if !defined(CHANNEL_PLAN)
#define CHANNEL_PLAN CP_AU915
#endif

#endif
