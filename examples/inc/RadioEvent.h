#ifndef __RADIO_EVENT_H__
#define __RADIO_EVENT_H__

#include "dot_util.h"
#include "mDotEvent.h"
//#include "Fota.h"

//DigitalOut myled( D15); // D15  XBEE_RSSI

class RadioEvent : public mDotEvent
{

public:

    RadioEvent() {} //Constructor de la clase

    virtual ~RadioEvent() {} // Destructor de la clase

    virtual void PacketRx(uint8_t port, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, lora::DownlinkControl ctrl, uint8_t slot, uint8_t retries, uint32_t address, bool dupRx)
    {
        mDotEvent::PacketRx(port, payload, size, rssi, snr, ctrl, slot, retries, address, dupRx);
    }
    /*!
     * MAC layer event callback prototype.
     *
     * \param [IN] flags Bit field indicating the MAC events occurred
     * \param [IN] info  Details about MAC events occurred
     */
    virtual void MacEvent(LoRaMacEventFlags* flags, LoRaMacEventInfo* info)
    {

        if (mts::MTSLog::getLogLevel() == mts::MTSLog::TRACE_LEVEL) {
            std::string msg = "OK";
            switch (info->Status) {
                case LORAMAC_EVENT_INFO_STATUS_ERROR:
                    msg = "ERROR";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT:
                    msg = "TX_TIMEOUT";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_RX_TIMEOUT:
                    msg = "RX_TIMEOUT";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_RX_ERROR:
                    msg = "RX_ERROR";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL:
                    msg = "JOIN_FAIL";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_DOWNLINK_FAIL:
                    msg = "DOWNLINK_FAIL";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL:
                    msg = "ADDRESS_FAIL";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_MIC_FAIL:
                    msg = "MIC_FAIL";
                    break;
                default:
                    break;
            }
            logTrace("Event: %s", msg.c_str());

            logTrace("Flags Tx: %d Rx: %d RxData: %d RxSlot: %d LinkCheck: %d JoinAccept: %d",
                     flags->Bits.Tx, flags->Bits.Rx, flags->Bits.RxData, flags->Bits.RxSlot, flags->Bits.LinkCheck, flags->Bits.JoinAccept);
            logTrace("Info: Status: %d ACK: %d Retries: %d TxDR: %d RxPort: %d RxSize: %d RSSI: %d SNR: %d Energy: %d Margin: %d Gateways: %d",
                     info->Status, info->TxAckReceived, info->TxNbRetries, info->TxDatarate, info->RxPort, info->RxBufferSize,
                     info->RxRssi, info->RxSnr, info->Energy, info->DemodMargin, info->NbGateways);
        }

        if (flags->Bits.Rx) {

            logDebug("Rx %d bytes", info->RxBufferSize);

            if (info->RxBufferSize > 0) {
                if(info->RxPort == 10 &&info->RxBufferSize == 3) {
                    logInfo("Reciviendo Data port10");
                    // print RX data as string and hexadecimal
                    std::string rx((const char*)info->RxBuffer, info->RxBufferSize);
                    printf("Rx data: %s [%s]\r\n", rx.c_str(), mts::Text::bin2hexString(info->RxBuffer, info->RxBufferSize).c_str());
                    if(info->RxBuffer[0] < 50) {
                        printf ("Led ON");
                    } else if (info->RxBuffer[0] > 50) {
                        printf ("Led OFF");
                    } else {
                        logWarning("Received Rx, but cannot handle it. port=%d, size=%d", info->RxPort, info->RxBufferSize);
                    }

                } else if (info->RxPort =! 10) {
                    logWarning("Received Rx, but cannot handle it. port=%d, size=%d", info->RxPort, info->RxBufferSize);

                } else {

                }

            }

        }
    }
};

#endif

