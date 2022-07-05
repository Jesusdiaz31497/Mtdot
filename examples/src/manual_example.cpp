#include "dot_util.h"
#include "RadioEvent.h"
#include "DHT.h"

#if ACTIVE_EXAMPLE == MANUAL_EXAMPLE

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
/*
static uint8_t network_address[] =  { 0x01, 0x20, 0x56, 0xee };  //Nodo1//{ 0x26, 0x00, 0x1B, 0xAC };
//TTN Network Session Key
static uint8_t network_session_key[] = {0xf3, 0xe9, 0xee, 0x80, 0xd1, 0x00, 0x5e, 0x6e, 0x0a, 0xe0, 0x6f, 0x22, 0x7f, 0xe8, 0xd2, 0x0a};
//TTN App Session Key
static uint8_t data_session_key[] = { 0xca, 0x07, 0x0a, 0x2b, 0xb0, 0xe5, 0x1b, 0xed, 0x71, 0x93, 0xb5, 0x89, 0x47, 0xea, 0x68, 0xe8 };
*/


//NODO 00800000000136cb
static uint8_t network_address[] =  { 0x00, 0xb5, 0x60, 0xf6 };  //Nodo1//{ 0x26, 0x00, 0x1B, 0xAC };
//TTN Network Session Key
static uint8_t network_session_key[] = { 0xcc, 0x6c, 0x85, 0x23, 0x88, 0xb1, 0x87, 0xaf, 0xb7, 0x77, 0x24, 0x88, 0xd1, 0xd1, 0x1d, 0x72};
//TTN App Session Key
static uint8_t data_session_key[] = { 0x64, 0x3f, 0x5c, 0x5b, 0x8f, 0xa2, 0x94, 0xde, 0x22, 0x44, 0x8f, 0x4d, 0xc8, 0xe5, 0x27, 0xd4 };



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

AnalogIn pot(PB_0); //PB_1
//DigitalIn mypin(D1); //PA_2
//DigitalOut led( XBEE_RSSI); // D15
DHT sensor(PB_1,DHT22); //PB_1 -->A0
//static uint8_t counter = 0;
static bool myflag = false;
//LowPowerTicker flipper;
LowPowerTimeout flipper;

int main()
{

    //flipper.attach(&flip, 2.0); // the address of the function to be attached (flip) and the interval (2 seconds)

    int err;
    uint16_t light,temp,humy,counter;
    uint16_t d1,d2,d3,d4;

    // Custom event handler for automatically displaying RX data
    RadioEvent events;

    pc.baud(115200);

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
        dot->setLogLevel(mts::MTSLog::INFO_LEVEL);

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

    while (true) {

        std::vector<uint8_t> tx_data;
        // get some dummy data and send it to the gateway
        light = pot.read_u16();
        err = sensor.readData();
        counter++;
        if (err == 0) {
            logInfo("DHT readign OK");  // 0x%04X
            temp=sensor.ReadTemperature(CELCIUS);
            humy=sensor.ReadHumidity();

        } else {
            logError("DHT reading not OK %d", err);

            //temp=counter*10;
            //humy=light/counter;

            temp = 0x00af;
            humy = 0x050A;
            counter = 0x140A;
            d1 = 0x0515;
            d2 = 0x4004;
            d3 = 0x4e00;
            d4 = 0x6d70;

        }

        //logInfo("Temp: %lu [0x%04X]", temp, temp);  // 0x%04X
        //logInfo("Humedad: %lu [0x%04X]", humy, humy);  // 0x%04X
        //logInfo("Sensor: %lu [0x%04X]", light, light);  // 0x%04X

        //tx_data.push_back((light >> 8) & 0xFF);
        //tx_data.push_back(light & 0xFF);





        tx_data.push_back((temp >> 8) & 0xFF);
        tx_data.push_back(temp & 0xFF);


        tx_data.push_back((humy >> 8) & 0xFF);
        tx_data.push_back(humy & 0xFF);

        tx_data.push_back((counter >> 8) & 0xFF);
        tx_data.push_back(counter & 0xFF);

        tx_data.push_back((d1 >> 8) & 0xFF);
        tx_data.push_back(d1 & 0xFF);

        tx_data.push_back((d2 >> 8) & 0xFF);
        tx_data.push_back(d2 & 0xFF);

        tx_data.push_back((d3 >> 8) & 0xFF);
        tx_data.push_back(d3 & 0xFF);
        tx_data.push_back((d4 >> 8) & 0xFF);
        tx_data.push_back(d4 & 0xFF);


//00af05001400000040004e006d7000

        send_data(tx_data);

        // if going into deepsleep mode, save the session so we don't need to join again after waking up
        // not necessary if going into sleep mode since RAM is retained

        // pc.printf("counter %d\n\r", counter );

        if (deep_sleep) {
            logInfo("saving network session to NVM");
            dot->saveNetworkSession();
        }


        // ONLY ONE of the three functions below should be uncommented depending on the desired wakeup method
        //sleep_wake_rtc_only(deep_sleep);
        //sleep_wake_interrupt_only(deep_sleep);
        sleep_wake_rtc_or_interrupt(deep_sleep);
    }

    return 0;
}

#endif
