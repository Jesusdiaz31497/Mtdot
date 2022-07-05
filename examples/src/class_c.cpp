#include "dot_util.h"
#include "RadioEvent.h"

#if ACTIVE_EXAMPLE == CLASS_C

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
// * either the network name and passphrase can be used or //
//     the network ID (8 bytes) and KEY (16 bytes)         //
/////////////////////////////////////////////////////////////
static std::string network_name = "MultiTech";
static std::string network_passphrase = "MultiTech";
static uint8_t network_id[] = { 0x6C, 0x4E, 0xEF, 0x66, 0xF4, 0x79, 0x86, 0xA6 };
static uint8_t network_key[] = { 0x1F, 0x33, 0xA1, 0x70, 0xA5, 0xF1, 0xFD, 0xA0, 0xAB, 0x69, 0x7A, 0xAE, 0x2B, 0x95, 0x91, 0x6B };

//////////////////ABP CREDENTIALS
//NODO 2
static uint8_t network_address[] =  { 0x26, 0x00, 0x12, 0x18 };  //Nodo1//{ 0x26, 0x00, 0x1B, 0xAC };
//TTN Network Session Key
static uint8_t network_session_key[] = { 0xA0, 0x02, 0xC0, 0x73, 0x37, 0xAD, 0xE3, 0x6C, 0x2C, 0xB3, 0xF4, 0x78, 0x51, 0x48, 0x39, 0x97 };
//TTN App Session Key
static uint8_t data_session_key[] = { 0xBF, 0xB4, 0xEE, 0x94, 0xC2, 0x0A, 0x73, 0x5C, 0x12, 0xD6, 0x4B, 0x27, 0x7C, 0xEF, 0xDD, 0x98 };


static uint8_t frequency_sub_band = 2;
static lora::NetworkType network_type = lora::PUBLIC_LORAWAN;
static uint8_t join_delay = 5;
static uint8_t ack = true;
static bool adr = true;

mDot* dot = NULL;
lora::ChannelPlan* plan = NULL;
///////////////////////////
//RadioEvent* rmdot = NULL;
uint8_t port;
uint8_t *payload;
uint16_t size;
int16_t rssi;
int8_t snr;
lora::DownlinkControl ctrl;
uint8_t slot;
uint8_t retries;
uint32_t address;
bool dupRx;
//////////////////////////////
Serial pc(USBTX, USBRX);

AnalogIn lux(XBEE_AD0);

static bool tick_count = false; //bandera para indicar que ha pasado un minuto de medicion
LowPowerTimeout flipper;





void flip()
{
    tick_count = true;
}

int main()
{
    // Custom event handler for automatically displaying RX data
    RadioEvent events;
    LoRaMacEventInfo* info;
    pc.baud(115200);


    mts::MTSLog::setLogLevel(mts::MTSLog::TRACE_LEVEL);

#if CHANNEL_PLAN == CP_US915
    plan = new lora::ChannelPlan_US915();
#elif CHANNEL_PLAN == CP_AU915
    plan = new lora::ChannelPlan_AU915();
#elif CHANNEL_PLAN == CP_EU868
    plan = new lora::ChannelPlan_EU868();
#elif CHANNEL_PLAN == CP_KR920
    plan = new lora::ChannelPlan_KR920();
#elif CHANNEL_PLAN == CP_AS923
    plan = new lora::ChannelPlan_AS923();
#elif CHANNEL_PLAN == CP_AS923_JAPAN
    plan = new lora::ChannelPlan_AS923_Japan();
#elif CHANNEL_PLAN == CP_IN865
    plan = new lora::ChannelPlan_IN865();
#endif
    assert(plan);

    dot = mDot::getInstance(plan);
    assert(dot);

    logInfo("mbed-os library version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    // start from a well-known state
    logInfo("defaulting Dot configuration");
    dot->resetConfig();
    dot->resetNetworkSession();

    // make sure library logging is turned on
    dot->setLogLevel(mts::MTSLog::TRACE_LEVEL);

    // attach the custom events handler
    dot->setEvents(&events);
    /*
        // update configuration if necessary
        if (dot->getJoinMode() != mDot::OTA) {
            logInfo("changing network join mode to OTA");
            if (dot->setJoinMode(mDot::OTA) != mDot::MDOT_OK) {
                logError("failed to set network join mode to OTA");
            }
        }
        // in OTA and AUTO_OTA join modes, the credentials can be passed to the library as a name and passphrase or an ID and KEY
        // only one method or the other should be used!
        // network ID = crc64(network name)
        // network KEY = cmac(network passphrase)
        update_ota_config_name_phrase(network_name, network_passphrase, frequency_sub_band, network_type, ack);
        //update_ota_config_id_key(network_id, network_key, frequency_sub_band, network_type, ack);
    */

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


    // configure the Dot for class C operation
    // the Dot must also be configured on the gateway for class C
    // use the lora-query application to do this on a Conduit: http://www.multitech.net/developer/software/lora/lora-network-server/
    // to provision your Dot for class C operation with a 3rd party gateway, see the gateway or network provider documentation
    logInfo("changing network mode to class C");
    if (dot->setClass("C") != mDot::MDOT_OK) {
        logError("failed to set network mode to class C");
    }

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

    while (true) {
        uint16_t light;
        std::vector<uint8_t> tx_data;
        std::vector<uint8_t> rx_data;
        // join network if not joined
        if (!dot->getNetworkJoinStatus()) {
            join_network();
        }

        // get some dummy data and send it to the gateway
        light = lux.read_u16();
        tx_data.push_back((light >> 8) & 0xFF);
        tx_data.push_back(light & 0xFF);
        logInfo("light: %lu [0x%04X]", light, light);
        send_data(tx_data);
        /**
         * Fetch data received from the gateway
         * this function only checks to see if a packet has been received - it does not open a receive window
         * send() must be called before recv()
         * @param data a vector to put the received data into
         * @returns MDOT_OK if packet was successfully received
         */

        logInfo("waiting for 30s");
        flipper.attach(&flip, 30.0);// // Tiempo de muestreo de pluviometro
        tick_count = false;

        // the Dot can't sleep in class C mode
        // it must be waiting for data from the gateway
        // send data every 30s
        //logInfo("waiting for 30s");
        wait(30);
    }

    return 0;
}

#endif