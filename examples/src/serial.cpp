#include "dot_util.h"
#include "RadioEvent.h"
#include "stdio.h"
#include "string.h"
#include "mbed.h"

#if ACTIVE_EXAMPLE == SERIAL

/////////////////////////////////////////////////////////////////////////////
// -------------------- DOT LIBRARY REQUIRED ------------------------------//
// * Because these example programs can be used for both mDot and xDot     //
//     devices, the LoRa stack is not included. The libmDot library should //
//     be imported if building for mDot devices. The libxDot library       //
//     should be imported if building for xDot devices.                    //
// * https://developer.mbed.org/teams/MultiTech/code/libmDot-dev-mbed5/    //
// * https://developer.mbed.org/teams/MultiTech/code/libmDot-mbed5/        //
// * https://developer.mbed.org/teams/MultiTech/code/libxDot-dev-mbed5/    //
// * https://developer.mbed.org/teams/MultiTech/code/libxDot-mbed5/        //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// * these options must match the settings on your gateway //
// * edit their values to match your configuration         //
// * frequency sub band is only relevant for the 915 bands //
/////////////////////////////////////////////////////////////

/*
//NODO 00800000000136f7
static uint8_t network_address[] =  { 0x01, 0x6e, 0x94, 0x2c};
//TTN Network Session Key
static uint8_t network_session_key[] = {0x3b, 0x03, 0xc2, 0x87, 0x63, 0x58, 0xd8, 0x66, 0xd7, 0xcb, 0x45, 0x31, 0xae, 0xc1, 0xcf, 0xba };
//TTN App Session Key
static uint8_t data_session_key[] = { 0xea, 0x9d, 0xfc, 0xdc, 0xb0, 0x43, 0x12, 0x24, 0xfa, 0x59, 0xae, 0xcc, 0xe6, 0x04, 0x91, 0xd1};
*/


//NODO 00800000000136cb
/*
static uint8_t network_address[] =  { 0x00, 0xb5, 0x60, 0xf6 };  //Nodo1//{ 0x26, 0x00, 0x1B, 0xAC };
//TTN Network Session Key
static uint8_t network_session_key[] = { 0xcc, 0x6c, 0x85, 0x23, 0x88, 0xb1, 0x87, 0xaf, 0xb7, 0x77, 0x24, 0x88, 0xd1, 0xd1, 0x1d, 0x72};
//TTN App Session Key
static uint8_t data_session_key[] = { 0x64, 0x3f, 0x5c, 0x5b, 0x8f, 0xa2, 0x94, 0xde, 0x22, 0x44, 0x8f, 0x4d, 0xc8, 0xe5, 0x27, 0xd4 };
*/
/*
//NODO 0080000000018a88
static uint8_t network_address[] =  { 0x30, 0xC4, 0xCF, 0x91  };
//TTN Network Session Key
static uint8_t network_session_key[] = {0xBC,0xC0,0xE2,0x88,0x05,0x45,0x8C,0xBF,0xF3,0x09,0x88,0xA4,0x4A,0xD6,0xB2,0x1E};
//TTN App Session Key
static uint8_t data_session_key[] = { 0x9B,0x06,0x9F,0xA3,0x18,0xF4,0x82,0x06,0x88,0x73,0xF1,0x27,0xE9,0x0E,0xD8,0xE2 };
*/

/*
//NODO 00:80:00:00:00:01:92:37
static uint8_t network_address[] =  {0x31, 0xFC, 0x23, 0x0C  };  //Nodo1//{ 0x26, 0x00, 0x1B, 0xAC };
//TTN Network Session Key
static uint8_t network_session_key[] = {0xC5,0x3E,0x89,0x91,0x5C,0x6E,0xF5,0xE3,0x0C,0x43,0xEA,0xEC,0x47,0x8D,0x00,0x02};
//TTN App Session Key
static uint8_t data_session_key[] = { 0xC9,0x2E,0x8C,0x61,0x4D,0x5A,0xEC,0x67,0x5D,0xB9,0x7E,0x0F,0x49,0xD1,0x41,0x37 };
*/


/*
// NODO 00:80:00:00:00:01:92:48
static uint8_t network_address[] =  {0x30, 0xCC, 0x61, 0x21   };  //Nodo1//{ 0x26, 0x00, 0x1B, 0xAC };
//TTN Network Session Key
static uint8_t network_session_key[] = {0x57,0x2D,0xA1,0x0A,0x64,0xD4,0x8A,0xFE,0xFB,0x41,0x16,0x5E,0x03,0x05,0x98,0x4C};
//TTN App Session Key
static uint8_t data_session_key[] = { 0x7C,0x3F,0x91,0x7B,0xB3,0x8C,0xB2,0x3B,0xE1,0xB0,0x7E,0x4D,0xFB,0x28,0x19,0x4D };
*/

/*
//NODO 0080000000018a7d
static uint8_t network_address[] = {0x31, 0x80, 0x4E, 0x72};
// TTN Network Session Key
static uint8_t network_session_key[] = {0x3D, 0x61, 0x5F, 0xBE, 0xFA, 0x99, 0x74, 0xB8, 0xCA, 0x71, 0xA1, 0x1D, 0xEB, 0xD3, 0x5F, 0xE2};
// TTN App Session Key
static uint8_t data_session_key[] = {0xC7, 0x4E, 0x40, 0x43, 0xA9, 0x6D, 0x99, 0xC1, 0xB7, 0x03, 0xB4, 0x1C, 0x7E, 0x3E, 0x25, 0x45};
*/

/*
//NODO 0080000000018a74
static uint8_t network_address[] =  { 0x00, 0x1b, 0x3c, 0x3e};
//TTN Network Session Key
static uint8_t network_session_key[] = {0x4f, 0x3c, 0xfa, 0x21, 0xe0, 0xe6, 0x8f, 0x16, 0x63, 0x1c, 0x47, 0x83, 0xb0, 0x99, 0x57, 0x29 };
//TTN App Session Key
static uint8_t data_session_key[] = { 0x00, 0xa3, 0x97, 0xd7, 0x04, 0x47, 0xc6, 0xd5, 0xe3, 0x1b, 0x20, 0x69, 0xbf, 0x46, 0x34, 0xf9};
*/


// NODO 0080000000018a73
static uint8_t network_address[] = {0x31, 0x68, 0xF1, 0xD3};
// TTN Network Session Key
static uint8_t network_session_key[] = {0x0F, 0x83, 0x8A, 0xF0, 0xB2, 0x53,
                                        0xA7, 0xCD, 0xAE, 0x82, 0x9B, 0x76,
                                        0x22, 0x1C, 0x47, 0x4F};
// TTN App Session Key
static uint8_t data_session_key[] = {0x28, 0xD9, 0x40, 0xE3, 0x7A, 0x09,
                                     0x26, 0xE8, 0x3A, 0xF5, 0x98, 0x77,
                                     0x3A, 0x3A, 0x74, 0x12};



/*
static uint8_t network_address[] =  {0x01, 0xcf, 0xca, 0x76};
static uint8_t network_session_key[] =  {0xa2, 0x77, 0x2d, 0xfb, 0x59, 0x49, 0x98, 0xd0, 0xcd, 0xa8, 0x50, 0x44, 0x14, 0x29, 0xd8, 0xe8};
static uint8_t data_session_key[] = {0x1f, 0x19, 0xc0, 0xc7, 0xcf, 0xe2, 0xfe, 0x98, 0xc0, 0x86, 0x52, 0x74, 0xb8, 0xae, 0x60, 0xd4};
*/

/*
//NODO 00800000000136f7
static uint8_t network_address[] =  { 0x01, 0x6e, 0x94, 0xc2};
//TTN Network Session Key
static uint8_t network_session_key[] = {0x3b, 0x03, 0xc2, 0x87, 0x63, 0x58, 0xd8, 0x66, 0xd7, 0xcb, 0x45, 0x31, 0xae, 0xc1, 0xcf, 0xba };
//TTN App Session Key
static uint8_t data_session_key[] = { 0xea, 0x9d, 0xfc, 0xdc, 0xb0, 0x43, 0x12, 0x24, 0xfa, 0x59, 0xae, 0xcc, 0xe6, 0x04, 0x91, 0xd1};

*/

static uint8_t frequency_sub_band = 2;
static lora::NetworkType network_type = lora::PUBLIC_LORAWAN;
static uint8_t join_delay = 5;
static uint8_t ack = 0;
static bool adr = false;

// deepsleep consumes slightly less current than sleep
// in sleep mode, IO state is maintained, RAM is retained, and application will resume after waking up
// in deepsleep mode, IOs float, RAM is lost, and application will start from beginning after waking up
// if deep_sleep == true, device will enter deepsleep mode
static bool deep_sleep = false;

mDot* dot = NULL;
lora::ChannelPlan* plan = NULL;
Serial pc(USBTX, USBRX);
Serial device(PA_2,PA_3);
DigitalOut l_r(PB_0);
DigitalOut l_b(PA_4);
DigitalOut l_g(PA_6);


const int kMaxBufferSize = 100;
char      buffer[kMaxBufferSize];
int       len = 0;
//static uint8_t counter = 0;
static bool myflag = false;
static bool rain_flag = false; //inicia False - No esta lloviendo
static bool timer_rain = false; //inicia False - No esta lloviendo
static bool tick_count = false; //bandera para indicar que ha pasado un minuto de medicion
static bool timer60s = false;
//LowPowerTicker flipper;
LowPowerTimeout flipper;
LowPowerTimeout flipper2;



int main()
{
    char id_char ='a';

    device.baud(9600);
    pc.baud(9600);

    buffer[0] = '\0';

    //device.printf("Start...\n\n");


    // Custom event handler for automatically displaying RX data
    RadioEvent events;



    mts::MTSLog::setLogLevel(mts::MTSLog::TRACE_LEVEL);

#if CHANNEL_PLAN == CP_US915
    plan = new lora::ChannelPlan_US915();
#elif CHANNEL_PLAN == CP_AU915
    plan = new lora::ChannelPlan_AU915();
#endif
    assert(plan);

    dot = mDot::getInstance(plan);
    assert(dot);

    // attach the custom events handler
    dot->setEvents(&events);

    if (!dot->getStandbyFlag()) {
        logInfo("mbed-os library version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

        // start from a well-known state
        logInfo("defaulting Dot configuration");
        dot->resetConfig();
        dot->resetNetworkSession();

        // make sure library logging is turned on
        dot->setLogLevel(mts::MTSLog::DEBUG_LEVEL);

        // update configuration if necessary
        if (dot->getJoinMode() != mDot::MANUAL) {
            logInfo("changing network join mode to MANUAL");
            if (dot->setJoinMode(mDot::MANUAL) != mDot::MDOT_OK) {
                logError("failed to set network join mode to MANUAL");
            }
        }
        // in MANUAL join mode there is no join request/response transaction
        // as long as the Dot is configured correctly and provisioned correctly on the gateway, it should be able to communicate
        // network address - 4 bytes (00000001 - FFFFFFFE)
        // network session key - 16 bytes
        // data session key - 16 bytes
        // to provision your Dot with a Conduit gateway, follow the following steps
        //   * ssh into the Conduit
        //   * provision the Dot using the lora-query application: http://www.multitech.net/developer/software/lora/lora-network-server/
        //      lora-query -a 01020304 A 0102030401020304 <your Dot's device ID> 01020304010203040102030401020304 01020304010203040102030401020304
        //   * if you change the network address, network session key, or data session key, make sure you update them on the gateway
        // to provision your Dot with a 3rd party gateway, see the gateway or network provider documentation
        update_manual_config(network_address, network_session_key, data_session_key, frequency_sub_band, network_type, ack);

        // enable or disable Adaptive Data Rate
        dot->setAdr(adr);

        // Configure the join delay
        dot->setJoinDelay(join_delay);

        // save changes to configuration
        logInfo("saving configuration");
        if (!dot->saveConfig()) {
            logError("failed to save configuration");
        }

        // display configuration
        display_config();
    } else {
        // restore the saved session if the dot woke from deepsleep mode
        // useful to use with deepsleep because session info is otherwise lost when the dot enters deepsleep
        logInfo("restoring network session from NVM");
        dot->restoreNetworkSession();
    }
    l_r = 1;
    l_b=1;
    for (int l = 0; l <= 10; l++) {
        l_g = 1;          // set LED1 pin to high
        wait(0.1);
        //device.putc('A');
        l_g.write(0);     // set LED1 pin to low
        wait(0.1);
    }
    l_r = 0;

    sleep_wake_interrupt_only(deep_sleep);

    while (true) {

        std::vector<uint8_t> tx_data;

        //device.printf("...");
        while (device.readable()) {
           
            char new_char = device.getc();
            uint8_t foo [100];
            buffer[len++] = new_char;
            buffer[len] = '\0';

            if (new_char == 'm') {
                id_char = new_char;
            } else {
                id_char = id_char;
            }

            if (id_char == 'm' && new_char == 'p') {

                for (int i = 0; i <= len; i++) {
                    //device.printf("%x %d", buffer[i],buffer[i]);
                    foo [i] = buffer[i];
                    //tx_data.push_back((foo [i] >> 8) & 0xFF);
                    tx_data.push_back(foo [i] & 0xFF);

                    wait(0.1);

                }

                len = 0;


                send_data(tx_data);
                myflag = true;

                l_r = 1;
                l_g=1;
                for (int l = 0; l <= 5; l++) {
                    l_b = 1;          // set LED1 pin to high
                    wait(0.1);
                    //device.putc('A');
                    l_b.write(0);     // set LED1 pin to low
                    wait(0.1);

                }
                l_r = 0;
                l_g=0;
                l_b=0;

                break;

            }
        }



        // ONLY ONE of the three functions below should be uncommented depending on the desired wakeup method
        //sleep_wake_rtc_only(deep_sleep);

        if(myflag) {
            myflag = false;
            
            sleep_wake_interrupt_only(deep_sleep);
            //sleep_wake_rtc_or_interrupt(deep_sleep);
        }
    }
    return 0;
}

#endif