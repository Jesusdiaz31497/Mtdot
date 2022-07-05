#include "dot_util.h"
#include "RadioEvent.h"
#include "stdio.h"
#include "string.h"
#include "mbed.h"

#if ACTIVE_EXAMPLE == SERIAL_FP

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


//NODO 0080000000018a7d
static uint8_t network_address[] =  { 0x01, 0x20, 0x56, 0xee };
//TTN Network Session Key
static uint8_t network_session_key[] = {0xf3, 0xe9, 0xee, 0x80, 0xd1, 0x00, 0x5e, 0x6e, 0x0a, 0xe0, 0x6f, 0x22, 0x7f, 0xe8, 0xd2, 0x0a};
//TTN App Session Key
static uint8_t data_session_key[] = { 0xca, 0x07, 0x0a, 0x2b, 0xb0, 0xe5, 0x1b, 0xed, 0x71, 0x93, 0xb5, 0x89, 0x47, 0xea, 0x68, 0xe8 };




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
//Serial pc(USBTX, USBRX);

Serial device(PA_2,PA_3);
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
    uint16_t temp1,temp2;

    device.baud(115200);
    //pc.baud(115200);

    buffer[0] = '\0';

    device.printf("Start...\n\n");




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

    //sleep_wake_interrupt_only(deep_sleep);

    while (true) {
        std::vector<uint8_t> tx_data;

        //device.printf("...");
        while (device.readable()) {
            char new_char = device.getc();
            char t1[2]="";
            char t2[2]="";
            uint16_t foo [100];
            buffer[len++] = new_char;
            buffer[len] = '\0';
            device.printf("in_buf %s",buffer);
            // new message received, handle
            if (new_char == 'O') {
                //device.printf("%x ",buffer);
                for (int i = 0; i <= len; i++) {
                    device.printf("%x %d", buffer[i],buffer[i]);
                    foo [i] = buffer[i];
                    tx_data.push_back((foo [i] >> 8) & 0xFF);
                    tx_data.push_back(foo [i] & 0xFF);

                    wait(0.5);
                }

                len = 0;

                send_data(tx_data);


            }
        }
    }
    return 0;
}

#endif