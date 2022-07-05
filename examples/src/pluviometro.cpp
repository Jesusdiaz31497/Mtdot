#include "dot_util.h"
#include "RadioEvent.h"


#if ACTIVE_EXAMPLE == PLUVIOMETRO

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



//NODO 00800000000136cb
static uint8_t network_address[] =  { 0x02, 0x92, 0x45, 0x7f };  //Nodo1//{ 0x26, 0x00, 0x1B, 0xAC };
//TTN Network Session Key
static uint8_t network_session_key[] = { 0xaa, 0xc8, 0xd2, 0x9f, 0xf2, 0x31, 0x98, 0x64, 0xd1, 0x40, 0x14, 0x3b, 0xd9, 0x1f, 0x4f, 0x23 };
//TTN App Session Key
static uint8_t data_session_key[] = { 0xe2, 0xfe, 0x69, 0xdf, 0x52, 0xfa, 0x0c, 0x6d, 0xcd, 0x2e, 0x9a, 0x61, 0x5d, 0x87, 0x5a, 0x82 };



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

AnalogIn pot(PB_1); //PB_1
DigitalIn mypin(D1); //PA_2
//DigitalOut led( XBEE_RSSI); // D15
DHT sensor(D10,DHT22); //PA_4 -->D10
//static uint8_t counter = 0;
static bool myflag = false;
static bool rain_flag = false; //inicia False - No esta lloviendo
static bool timer_rain = false; //inicia False - No esta lloviendo
static bool tick_count = false; //bandera para indicar que ha pasado un minuto de medicion
static bool timer60s = false;
//LowPowerTicker flipper;
LowPowerTimeout flipper;
LowPowerTimeout flipper2;


void flip()
{
    tick_count = true;
}
void flip2()
{
    timer_rain = true;
}

void timer60seg (){
    timer60s = true;
}    

int main()
{
    
    int rain;
    int estado;
    int err;
    uint16_t analog_sensor;
    uint16_t temp;
    uint16_t humy;
    estado=3;

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

        if (deep_sleep) { // se verifica la bandera de deep_sleep
            logInfo("saving network session to NVM");
            dot->saveNetworkSession();
        }     

/////////////ESTADO -  SLEEP
        if (estado == 0) { 
            pc.printf("modo sleep\n");
            estado=3;
            sleep_wake_rtc_or_interrupt(deep_sleep);
            
////////////ESTADO - SENSAR            
        } else if (estado ==1) {

            pc.printf("modo sensor\n");
            rain=0;
            
            // get some dummy data and send it to the gateway
            analog_sensor = pot.read_u16(); // leo sensor analogico
            
                        //Tomar lectura de sensor de temperatura            
            err = sensor.readData();
            wait(0.5);                 
            if (err == 0) {
                logInfo("DHT readign OK");  // 0x%04X
                temp=sensor.ReadTemperature(CELCIUS);
                humy=sensor.ReadHumidity();

            } else {
                logError("DHT reading not OK");
                temp=0xFAAA;
                humy=0xFBBB;
            }
            
            flipper.attach(&flip, 60.0);// // Tiempo de muestreo de pluviometro
            tick_count = false;
            
            //Mientras tick_count=false se queda en el while contando ticks que se alamacenan en rain
            logInfo("Registrando lluvia");  // 0x%04X
            while(!tick_count) {                
                if(mypin) {
                    logInfo("ticks %d", rain);  // 0x%04X
                    rain+=1;                    
                }
                wait(0.2);
            }
            

            estado=2; //ir a envio de datos
            logInfo("Estado  %d", estado );
            logInfo("Counter: %lu [0x%04X]", rain, rain);  // 0x%04X
            logInfo("Temp: %lu [0x%04X]", temp, temp);  // 0x%04X
            logInfo("Humedad: %lu [0x%04X]", humy, humy);  // 0x%04X
            logInfo("Sensor: %lu [0x%04X]", analog_sensor, analog_sensor);  // 0x%04X

////////////ESTADO ENVIAR DATOS
        } else if (estado == 2) {


            pc.printf("modo LoRa_TX\n");

//Acomodo la trama
            tx_data.push_back((analog_sensor >> 8) & 0xFF);
            tx_data.push_back(analog_sensor & 0xFF);
            tx_data.push_back((temp >> 8) & 0xFF);
            tx_data.push_back(temp & 0xFF);
            tx_data.push_back((humy >> 8) & 0xFF);
            tx_data.push_back(humy & 0xFF);
            tx_data.push_back((rain >> 8) & 0xFF);
            tx_data.push_back(rain & 0xFF);

//Envio Data
            send_data(tx_data);
            
            estado = 3;
            logInfo("Estado  %d", estado );

        } else if (estado == 3) {            
            
            flipper.attach(&timer60seg, 60.0);// // setup flipper to call timer60seg after 60 seconds
            logInfo("Verifiacando si esta lloviendo");
            while(!timer60s){
                if(mypin) {
                    logInfo("...");  // 0x%04X
                    rain+=1;}
                if(rain > 2){
                    estado = 1;
                    break;
                    } 
                wait(0.2);
                }
            if(timer60s && rain ==0){
                estado = 0; //go to sleep
                }
                else{estado=estado;}
           
           timer60s = false;

        }



        
        // if going into deepsleep mode, save the session so we don't need to join again after waking up
        // not necessary if going into sleep mode since RAM is retained


        // ONLY ONE of the three functions below should be uncommented depending on the desired wakeup method
        //sleep_wake_rtc_only(deep_sleep);
        //sleep_wake_interrupt_only(deep_sleep);
        //sleep_wake_rtc_or_interrupt(deep_sleep);
    }

    return 0;
}

#endif