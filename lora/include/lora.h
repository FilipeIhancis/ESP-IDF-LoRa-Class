#ifndef __LORA_H__
#define __LORA_H__

#include <string>
#include <string.h>
#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// Registers map
#define REG_FIFO                       0x00
#define REG_OP_MODE                    0x01
#define REG_FRF_MSB                    0x06
#define REG_FRF_MID                    0x07
#define REG_FRF_LSB                    0x08
#define REG_PA_CONFIG                  0x09
#define REG_OCP                        0x0b
#define REG_LNA                        0x0c
#define REG_FIFO_ADDR_PTR              0x0d
#define REG_FIFO_TX_BASE_ADDR          0x0e
#define REG_FIFO_RX_BASE_ADDR          0x0f
#define REG_FIFO_RX_CURRENT_ADDR       0x10
#define REG_IRQ_FLAGS                  0x12
#define REG_RX_NB_BYTES                0x13
#define REG_PKT_SNR_VALUE              0x19
#define REG_PKT_RSSI_VALUE             0x1a
#define REG_RSSI_VALUE                 0x1b
#define REG_MODEM_CONFIG_1             0x1d
#define REG_MODEM_CONFIG_2             0x1e
#define REG_PREAMBLE_MSB               0x20
#define REG_PREAMBLE_LSB               0x21
#define REG_PAYLOAD_LENGTH             0x22
#define REG_MODEM_CONFIG_3             0x26
#define REG_FREQ_ERROR_MSB             0x28
#define REG_FREQ_ERROR_MID             0x29
#define REG_FREQ_ERROR_LSB             0x2a
#define REG_RSSI_WIDEBAND              0x2c
#define REG_DETECTION_OPTIMIZE         0x31
#define REG_INVERTIQ                   0x33
#define REG_DETECTION_THRESHOLD        0x37
#define REG_SYNC_WORD                  0x39
#define REG_INVERTIQ2                  0x3b
#define REG_TEMP                       0x3C
#define REG_DIO_MAPPING_1              0x40
#define REG_DIO_MAPPING_2              0x41
#define REG_VERSION                    0x42
#define MODE_LONG_RANGE_MODE           0x80
#define MODE_SLEEP                     0x00
#define MODE_STDBY                     0x01
#define MODE_TX                        0x03
#define MODE_RX_CONTINUOUS             0x05
#define MODE_RX_SINGLE                 0x06

#define PA_BOOST                       0x80
#define PA_OUTPUT_RFO_PIN              0
#define PA_OUTPUT_PA_BOOST_PIN         1

#define IRQ_TX_DONE_MASK               0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK     0x20
#define IRQ_RX_DONE_MASK               0x40

#define RF_MID_BAND_THRESHOLD          525E6
#define RSSI_OFFSET_HF_PORT            157
#define RSSI_OFFSET_LF_PORT            164
#define TIMEOUT_RESET                  100

// SPI stuff
#if CONFIG_SPI2_HOST
#define HOST_ID SPI2_HOST
#elif CONFIG_SPI3_HOST
#define HOST_ID SPI3_HOST
#endif

// Default TAG to debug
#define TAG "LoRa"

namespace LORA
{
class LoRaClass 
{
public:

    LoRaClass();

    int init(void);
    int init(long freq, int miso, int mosi, int sck, int cs, int rst);
    void asyncSPI(bool spiChoose);
    void sleep_(void); 
    void enableInvertIQ(void);
    void disableInvertIQ(void);
    void idle(void);
    void receive(void);
    void txModeNode(void);
    void rxModeNode(void);
    void txModeGateway(void);
    void rxModeGateway(void);
    int getIRQ(void);
    void setTxPower(int level);
    void setLnaGain(int gain);
    void setFrequency(long frequency);
    long getFrequency();
    void setSpreadingFactor(int sf);
    int getSpreadingFactor(void);
    void setBandwidth(int sbw);
    long getBandwidth(void);
    void setCodingRate(int denominator);
    int getCodingRate(void);
    void setPreambleLength(long length);
    long getPreambleLength(void);
    void setSyncWord(int sw);
    void enableCrc(void);
    void disableCrc(void);
    void setOverCurrentProtection(uint8_t maxCurrent);
    int sendPacket(std::string outgoing);
    std::string receivePacket();
    int parsePacket(void);
    int packetLost(void);
    int rssi(void);
    int packetRssi(void);
    float packetSnr(void);
    long packetFrequencyError(void);

private:

    static spi_device_handle_t __spi;
    static int __implicit;
    static long __frequency;
    static int __send_packet_lost;
    static bool __async_Spi_transmission;

    void reset(void);
    void explicitHeaderMode(void);
    void implicitHeaderMode(int size);
    int readReg(int reg);
    void readRegBuffer(int reg, uint8_t *val, int len);
    void writeReg(int reg, int val);
    void writeRegBufffer(int reg, uint8_t *val, int len);
};

extern LoRaClass LoRa;

}
#endif
