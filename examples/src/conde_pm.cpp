#include "RadioEvent.h"
#include "dot_util.h"
#include "mbed.h"
#include "stdio.h"
#include "string.h"

#if ACTIVE_EXAMPLE == CONDE_PM

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
// NODO 0080000000018a7d
static uint8_t network_address[] = {0x31, 0x80, 0x4E, 0x72};
// TTN Network Session Key
static uint8_t network_session_key[] = {0x3D, 0x61, 0x5F, 0xBE, 0xFA, 0x99,
                                        0x74, 0xB8, 0xCA, 0x71, 0xA1, 0x1D,
                                        0xEB, 0xD3, 0x5F, 0xE2};
// TTN App Session Key
static uint8_t data_session_key[] = {0xC7, 0x4E, 0x40, 0x43, 0xA9, 0x6D,
                                     0x99, 0xC1, 0xB7, 0x03, 0xB4, 0x1C,
                                     0x7E, 0x3E, 0x25, 0x45};
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

static uint8_t frequency_sub_band = 2;
static lora::NetworkType network_type = lora::PUBLIC_LORAWAN;
static uint8_t join_delay = 5;
static uint8_t ack = 0;
static bool adr = false;

// deepsleep consumes slightly less current than sleep
// in sleep mode, IO state is maintained, RAM is retained, and application will
// resume after waking up in deepsleep mode, IOs float, RAM is lost, and
// application will start from beginning after waking up if deep_sleep == true,
// device will enter deepsleep mode
static bool deep_sleep = false;

mDot *dot = NULL;
lora::ChannelPlan *plan = NULL;
// Serial pc(USBTX, USBRX);

Serial device(PA_2, PA_3);
DigitalOut led(PB_1);

const int kMaxBufferSize = 300;
char buffer[kMaxBufferSize];
int len = 0;
// static uint8_t counter = 0;
static bool myflag = false;
static bool read_pm = false;

LowPowerTimeout flipper;

char new_char;
char old_char = 'a';
uint8_t mybuffer[100];
char cmd_data[4] = {2, 'D', 'A', 3};

void flip() { read_pm = true; }

int main() {

  device.baud(9600);

  buffer[0] = '\0';

  device.printf("\r\n Init Mdot \r\n");

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
    logInfo("mbed-os library version: %d.%d.%d", MBED_MAJOR_VERSION,
            MBED_MINOR_VERSION, MBED_PATCH_VERSION);

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
    // as long as the Dot is configured correctly and provisioned correctly on
    // the gateway, it should be able to communicate network address - 4 bytes
    // (00000001 - FFFFFFFE) network session key - 16 bytes data session key -
    // 16 bytes to provision your Dot with a Conduit gateway, follow the
    // following steps
    //   * ssh into the Conduit
    //   * provision the Dot using the lora-query application:
    //   http://www.multitech.net/developer/software/lora/lora-network-server/
    //      lora-query -a 01020304 A 0102030401020304 <your Dot's device ID>
    //      01020304010203040102030401020304 01020304010203040102030401020304
    //   * if you change the network address, network session key, or data
    //   session key, make sure you update them on the gateway
    // to provision your Dot with a 3rd party gateway, see the gateway or
    // network provider documentation
    update_manual_config(network_address, network_session_key, data_session_key,
                         frequency_sub_band, network_type, ack);

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
    // useful to use with deepsleep because session info is otherwise lost when
    // the dot enters deepsleep
    logInfo("restoring network session from NVM");
    dot->restoreNetworkSession();
  }

  // sleep_wake_interrupt_only(deep_sleep);
  // flipper.attach(&flip, 15.0); // // Tiempo de muestreo de pluviometro

  while (true) {
    std::vector<uint8_t> tx_data;
    int count = 0;
    while (count < 4) {
      device.putc(cmd_data[count]);
      count++;
    }
    if (device.readable()) {

      //while (device.readable()) {
        new_char = device.getc();
        buffer[len++] = new_char;
        buffer[len] = '\0';
        if (new_char == 32) {
          old_char = new_char;
        } else {
          old_char = old_char;
        }
        device.printf("%c", new_char);

        if ((new_char == 3) && (old_char == 32)) {

          int16_t dummy_var;
          for (int i = 0; i < 255; i = i + 30) {
            dummy_var = (((buffer[i + 11]) - 48) * 1000) +
                        ((buffer[i + 12] - 48) * 100) +
                        ((buffer[i + 13] - 48) * 10) + (buffer[i + 14] - 48);
            mybuffer[i] = (dummy_var >> 8) & 0xff;
            mybuffer[i + 1] = dummy_var & 0xFF;

            tx_data.push_back((dummy_var >> 8) & 0xFF);
            tx_data.push_back(dummy_var & 0xFF);

            device.printf(
                "\r\n dummy_var 0x%02x        mybuffer_msb 0x%02x      "
                "mybuffer_lsb 0x%02x   \r\n",
                dummy_var, mybuffer[i], mybuffer[i + 1]);
          }

          len = 0;
          device.printf("\r\n Sending Data \r\n");
          send_data(tx_data);
          myflag = true;

          for (int l = 0; l <= 5; l++) {
            led = 1; // set LED1 pin to high
            wait(0.1);
            // device.putc('A');
            led.write(0); // set LED1 pin to low
            wait(0.1);
          }

          break;
        }
      //}
    }

    sleep_wake_rtc_or_interrupt(deep_sleep);
  }
  return 0;
}
#endif