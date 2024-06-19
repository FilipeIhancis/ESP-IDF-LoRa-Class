#include "lora.h"

namespace LORA
{
spi_device_handle_t LoRaClass:: __spi;
int LoRaClass:: __implicit = 0;
long LoRaClass:: __frequency;
int LoRaClass:: __send_packet_lost = 0;
bool LoRaClass:: __async_Spi_transmission = true;

LoRaClass::LoRaClass() {}

int LoRaClass::init(void)
{
   esp_err_t ret;

   gpio_reset_pin(static_cast<gpio_num_t>(CONFIG_RST_GPIO));
   gpio_set_direction(static_cast<gpio_num_t>(CONFIG_RST_GPIO), GPIO_MODE_OUTPUT);
   gpio_reset_pin(static_cast<gpio_num_t>(CONFIG_CS_GPIO));
   gpio_set_direction(static_cast<gpio_num_t>(CONFIG_CS_GPIO), GPIO_MODE_OUTPUT);
   gpio_set_level(static_cast<gpio_num_t>(CONFIG_CS_GPIO), 1);

   spi_bus_config_t bus = {};
      bus.miso_io_num = CONFIG_MISO_GPIO;
      bus.mosi_io_num = CONFIG_MOSI_GPIO;
      bus.sclk_io_num = CONFIG_SCK_GPIO;
      bus.quadwp_io_num = -1;
      bus.quadhd_io_num = -1;
      bus.max_transfer_sz = 0;

   ret = spi_bus_initialize(HOST_ID, &bus, SPI_DMA_CH_AUTO);
   assert(ret == ESP_OK);

   spi_device_interface_config_t dev = {};
      dev.clock_speed_hz = 9000000;
      dev.mode = 0;
      dev.spics_io_num = CONFIG_CS_GPIO;
      dev.queue_size = 7;
      dev.flags = 0;
      dev.pre_cb = NULL;

   ret = spi_bus_add_device(HOST_ID, &dev, &__spi);
   assert(ret == ESP_OK);

   this->reset();

   // Checks if the module is connected correctly (checks expected version)
   uint8_t i = 0;
   while(1) {
      if(readReg(REG_VERSION) == 0x12) break;
      if(i >= TIMEOUT_RESET) return 0;
      i++; vTaskDelay(2);
   }

   this->sleep_();
   this->writeReg(REG_FIFO_RX_BASE_ADDR, 0);
   this->writeReg(REG_FIFO_TX_BASE_ADDR, 0);
   this->writeReg(REG_LNA, readReg(REG_LNA) | 0x03);
   this->writeReg(REG_MODEM_CONFIG_3, 0x04);
   this->idle();

   // setting default frequency:
   #if CONFIG_169MHZ
		this->setFrequency(169e6);
	#elif CONFIG_433MHZ
		this->setFrequency(433e6);
	#elif CONFIG_470MHZ
		this->setFrequency(470e6);
	#elif CONFIG_866MHZ
		this->setFrequency(866e6);
	#elif CONFIG_915MHZ
		LoRa.setFrequency(915e6);
	#elif CONFIG_OTHER
		long frequency = CONFIG_OTHER_FREQUENCY * 1000000;
		LoRa.setFrequency(frequency);
	#endif

   // Set default settings:
   this->setTxPower(10);
   this->enableCrc();
	this->setCodingRate(CONFIG_CODING_RATE);
	this->setBandwidth(CONFIG_BANDWIDTH);
	this->setSpreadingFactor(CONFIG_SF_RATE);

   return 1;
}

int LoRaClass::init(long freq, int miso, int mosi, int sck, int cs, int rst)
{
   esp_err_t ret;

   gpio_reset_pin(static_cast<gpio_num_t>(rst));
   gpio_set_direction(static_cast<gpio_num_t>(rst), GPIO_MODE_OUTPUT);
   gpio_reset_pin(static_cast<gpio_num_t>(cs));
   gpio_set_direction(static_cast<gpio_num_t>(cs), GPIO_MODE_OUTPUT);
   gpio_set_level(static_cast<gpio_num_t>(cs), 1);

   spi_bus_config_t bus = {};
      bus.miso_io_num = miso;
      bus.mosi_io_num = mosi;
      bus.sclk_io_num = sck;
      bus.quadwp_io_num = -1;
      bus.quadhd_io_num = -1;
      bus.max_transfer_sz = 0;

   ret = spi_bus_initialize(HOST_ID, &bus, SPI_DMA_CH_AUTO);
   assert(ret == ESP_OK);

   spi_device_interface_config_t dev = {};
      dev.clock_speed_hz = 9000000;
      dev.mode = 0;
      dev.spics_io_num = cs;
      dev.queue_size = 7;
      dev.flags = 0;
      dev.pre_cb = NULL;

   ret = spi_bus_add_device(HOST_ID, &dev, &__spi);
   assert(ret == ESP_OK);
   this->reset();

   // Checks if the module is connected correctly (checks expected version)
   uint8_t i = 0;
   while(1) {
      if(readReg(REG_VERSION) == 0x12) break;
      if(i >= TIMEOUT_RESET) return 0;
      i++; vTaskDelay(2);
   }

   this->sleep_();
   this->writeReg(REG_FIFO_RX_BASE_ADDR, 0);
   this->writeReg(REG_FIFO_TX_BASE_ADDR, 0);
   this->writeReg(REG_LNA, readReg(REG_LNA) | 0x03);
   this->writeReg(REG_MODEM_CONFIG_3, 0x04);
   this->idle();

   // Set default settings:
   this->setFrequency(freq);
   this->setTxPower(10);
   this->enableCrc();
	this->setCodingRate(CONFIG_CODING_RATE);
	this->setBandwidth(CONFIG_BANDWIDTH);
	this->setSpreadingFactor(CONFIG_SF_RATE);
   
   return 1;
}

void LoRaClass::asyncSPI(bool spiChoose)
{
   __async_Spi_transmission = spiChoose;
}

void LoRaClass::writeReg(int reg, int val)
{
   uint8_t out[2] = { static_cast<uint8_t>(0x80 | reg), static_cast<uint8_t>(val) };
   uint8_t in[2];

   spi_transaction_t t = {};
      t.flags = 0;
      t.length = 8 * sizeof(out);
      t.tx_buffer = out;
      t.rx_buffer = in;

   if(__async_Spi_transmission) spi_device_transmit(__spi, &t);
   else spi_device_polling_transmit(__spi, &t);
}

void LoRaClass::writeRegBufffer(int reg, uint8_t *val, int len)
{
   uint8_t *out;
   out = (uint8_t *)malloc(len+1);

   out[0] = 0x80 | reg;

   for (int i=0;i<len;i++) { out[i+1] = val[i]; }

   spi_transaction_t t = {};
      t.flags = 0;
      t.length = 8 * (len+1);
      t.tx_buffer = out;
      t.rx_buffer = NULL;

   if(__async_Spi_transmission) spi_device_transmit(__spi, &t);
   else spi_device_polling_transmit(__spi, &t);

   free(out);
}

int LoRaClass::readReg(int reg)
{
   uint8_t out[2] = { static_cast<uint8_t>(reg), 0xff };
   uint8_t in[2];

   spi_transaction_t t = {};
      t.flags = 0;
      t.length = 8 * sizeof(out);
      t.tx_buffer = out;
      t.rx_buffer = in;

   if(__async_Spi_transmission) spi_device_transmit(__spi, &t);
   else spi_device_polling_transmit(__spi, &t);

   return in[1];
}

void LoRaClass::readRegBuffer(int reg, uint8_t *val, int len)
{
   uint8_t *out;
   uint8_t *in;

   out = (uint8_t *)malloc(len+1);
   in = (uint8_t *)malloc(len+1);
   out[0] = reg;

   for (int i = 0; i< len; i++) out[i+1] = 0xff;

   spi_transaction_t t = {};
      t.flags = 0;
      t.length = 8 * (len+1);
      t.tx_buffer = out;
      t.rx_buffer = in;

   if(__async_Spi_transmission) spi_device_transmit(__spi, &t);
   else spi_device_polling_transmit(__spi, &t);

   for (int i = 0; i < len; i++) val[i] = in[i+1];
   free(out); free(in);
}

void LoRaClass::reset(void)
{
   gpio_set_level(static_cast<gpio_num_t>(CONFIG_RST_GPIO), 0);
   vTaskDelay(pdMS_TO_TICKS(1));
   gpio_set_level(static_cast<gpio_num_t>(CONFIG_RST_GPIO), 1);
   vTaskDelay(pdMS_TO_TICKS(10));
}

void LoRaClass::explicitHeaderMode(void) {
   __implicit = 0;
   this->writeReg(REG_MODEM_CONFIG_1, readReg(REG_MODEM_CONFIG_1) & 0xfe);
}

void LoRaClass::implicitHeaderMode(int size) {
   __implicit = 1;
   this->writeReg(REG_MODEM_CONFIG_1, readReg(REG_MODEM_CONFIG_1) | 0x01);
   this->writeReg(REG_PAYLOAD_LENGTH, size);
}

void LoRaClass::sleep_(void) { 
   this->writeReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}

void LoRaClass::idle(void) {
   this->writeReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
}

void LoRaClass::receive(void) {
   this->writeReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

void LoRaClass::enableInvertIQ(void)
{
  this->writeReg(REG_INVERTIQ,  0x66);
  this->writeReg(REG_INVERTIQ2, 0x19);
}

void LoRaClass::disableInvertIQ(void)
{
  this->writeReg(REG_INVERTIQ,  0x27);
  this->writeReg(REG_INVERTIQ2, 0x1d);
}

void LoRaClass::txModeNode(void)
{
   this->idle();
   this->disableInvertIQ();
}

void LoRaClass::rxModeNode(void)
{
   this->enableInvertIQ();
   this->receive();
}

void LoRaClass::txModeGateway(void)
{
   this->idle();
   this->enableInvertIQ();
}

void LoRaClass::rxModeGateway(void)
{
   this->disableInvertIQ();
   this->receive();
}

void LoRaClass::setTxPower(int level)
{
   if (level < 2) level = 2;
   else if (level > 17) level = 17;

   this->writeReg(REG_PA_CONFIG, PA_BOOST | (level - 2));
}

void LoRaClass::setLnaGain(int gain)
{
   if(gain < 0 || gain > 6) gain = 6;

   this->idle();
  
  if (gain == 0) {
    this->writeReg(REG_MODEM_CONFIG_3, 0x04);                   // if gain = 0, enable AGC
  } else {
    this->writeReg(REG_MODEM_CONFIG_3, 0x00);                   // disable AGC
    this->writeReg(REG_LNA, 0x03);                              // clear Gain and set LNA boost
    this->writeReg(REG_LNA, readReg(REG_LNA) | (gain << 5));    // Set gain
  }
}

void LoRaClass::setOverCurrentProtection(uint8_t maxCurrent)
{
   // OBS: formulas based on RFM9XX datasheet
   uint8_t ocpTrim = 27;

   if(maxCurrent < 45 || maxCurrent > 240) maxCurrent = 140;
   if(maxCurrent > 120 && maxCurrent < 130) maxCurrent = 140;

   if (maxCurrent <= 120)        ocpTrim = (maxCurrent - 45) / 5;
   else if (maxCurrent <= 240)   ocpTrim = (maxCurrent + 30) / 10;

   this->writeReg(REG_OCP, 0x20 | (0x1F & ocpTrim));
}

void LoRaClass::setFrequency(long frequency)
{
   __frequency = frequency;
   uint64_t frf = ((uint64_t)frequency << 19) / 32000000;
   this->writeReg(REG_FRF_MSB, (uint8_t)(frf >> 16));
   this->writeReg(REG_FRF_MID, (uint8_t)(frf >> 8));
   this->writeReg(REG_FRF_LSB, (uint8_t)(frf >> 0));
}

long LoRaClass::getFrequency()
{
   return __frequency;
}

void LoRaClass::setSpreadingFactor(int sf)
{
   if (sf < 6) sf = 6;
   else if (sf > 12) sf = 12;

   if (sf == 6) {
      this->writeReg(REG_DETECTION_OPTIMIZE, 0xc5);
      this->writeReg(REG_DETECTION_THRESHOLD, 0x0c);
   } else {
      this->writeReg(REG_DETECTION_OPTIMIZE, 0xc3);
      this->writeReg(REG_DETECTION_THRESHOLD, 0x0a);
   }
   this->writeReg(REG_MODEM_CONFIG_2, (readReg(REG_MODEM_CONFIG_2) & 0x0f) | ((sf << 4) & 0xf0));
}

int LoRaClass::getSpreadingFactor(void)
{
   return (readReg(REG_MODEM_CONFIG_2) >> 4);
}

void LoRaClass::setBandwidth(int sbw) 
{
   if(sbw < 10) this->writeReg(REG_MODEM_CONFIG_1, (readReg(REG_MODEM_CONFIG_1) & 0x0f) | (sbw << 4));
}

long LoRaClass::getBandwidth(void) 
{
   unsigned char bw = (readReg(REG_MODEM_CONFIG_1) >> 4);

   switch (bw) {
      case 0: return 7.8E3;
      case 1: return 10.4E3;
      case 2: return 15.6E3;
      case 3: return 20.8E3;
      case 4: return 31.25E3;
      case 5: return 41.7E3;
      case 6: return 62.5E3;
      case 7: return 125E3;
      case 8: return 250E3;
      case 9: return 500E3;
   }
   return -1;
}

void LoRaClass::setCodingRate(int denominator)
{
   if (denominator < 5) denominator = 5;
   else if (denominator > 8) denominator = 8;

   int cr = denominator - 4;
   this->writeReg(REG_MODEM_CONFIG_1, (readReg(REG_MODEM_CONFIG_1) & 0xf1) | (cr << 1));
}

int LoRaClass::getCodingRate(void) 
{
   return ((readReg(REG_MODEM_CONFIG_1) & 0x0E) >> 1);
}

void LoRaClass::setPreambleLength(long length) 
{
   this->writeReg(REG_PREAMBLE_MSB, (uint8_t)(length >> 8));
   this->writeReg(REG_PREAMBLE_LSB, (uint8_t)(length >> 0));
}

long LoRaClass::getPreambleLength(void)
{
   long preamble;
   preamble = readReg(REG_PREAMBLE_MSB) << 8;
   preamble = preamble + readReg(REG_PREAMBLE_LSB);
   return preamble;
}

void LoRaClass::setSyncWord(int sw) 
{
   this->writeReg(REG_SYNC_WORD, sw);
}

void LoRaClass::enableCrc(void) 
{
   this->writeReg(REG_MODEM_CONFIG_2, readReg(REG_MODEM_CONFIG_2) | 0x04);
}

void LoRaClass::disableCrc(void) 
{
   this->writeReg(REG_MODEM_CONFIG_2, readReg(REG_MODEM_CONFIG_2) & 0xfb);
}

int LoRaClass::sendPacket(std::string message)
{
   // Buffer aux
   uint8_t buf[256];
   int size = snprintf((char*)buf, sizeof(buf), "%s", message.c_str());

   // Transmission mode (idle)
   this->idle();

   // Sets FIFO address pointer to 0
   this->writeReg(REG_FIFO_ADDR_PTR, 0);

   // Writes formatted data to the device's FIFO buffer
   this->writeRegBufffer(REG_FIFO, buf, size);

   // Sets the length of data to be transmitted
   this->writeReg(REG_PAYLOAD_LENGTH, size);

   // Sets the LoRa device operating mode to transmit mode
   this->writeReg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);

   int intCounter = 0;  // Counter

   // Check transmission status
   while(1) 
   {
      // Checks if the transmission is complete
      if((readReg(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) == IRQ_TX_DONE_MASK) {
         this->writeReg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK); return 1;
      }
      // Check if have reached attempt limits
      if(intCounter == 10) {
         __send_packet_lost++; return 0;
      }
      // Update counter
      intCounter++;
      vTaskDelay(2);
   }
}

std::string LoRaClass::receivePacket()
{
   // Buffer aux
   uint8_t buf[256];
   int size = sizeof(buf);

   // Resets interrupt flags
   int len = 0, irq = readReg(REG_IRQ_FLAGS);
   this->writeReg(REG_IRQ_FLAGS, irq);

   // Checks if any packet was received
   if((irq & IRQ_RX_DONE_MASK) == 0) return "";

   // Checks if the packet have CRC errors
   if(irq & IRQ_PAYLOAD_CRC_ERROR_MASK) return "";

   // Determines the size of the packet 
   if (__implicit) len = readReg(REG_PAYLOAD_LENGTH);
   else len = readReg(REG_RX_NB_BYTES);

   // Sets FIFO buffer pointer to current read address
   this->idle();
   this->writeReg(REG_FIFO_ADDR_PTR, readReg(REG_FIFO_RX_CURRENT_ADDR));
   if(len > size) len = size;

   // FIFO buffer reading with packet size
   this->readRegBuffer(REG_FIFO, buf, len);

   // Puts the message in a string
   std::string receivedMesssage;
   receivedMesssage.assign(reinterpret_cast<char*>(buf), len);

   return receivedMesssage;
}

int LoRaClass::parsePacket(void) 
{
   if(readReg(REG_IRQ_FLAGS) & IRQ_RX_DONE_MASK) return 1;
   return 0;
}

int LoRaClass::getIRQ(void) 
{
   return (readReg(REG_IRQ_FLAGS));
}

int LoRaClass::packetLost(void) 
{
   return (__send_packet_lost);
}

int LoRaClass::rssi(void)
{
   return (readReg(REG_RSSI_VALUE) - (__frequency < RF_MID_BAND_THRESHOLD ? RSSI_OFFSET_LF_PORT : RSSI_OFFSET_HF_PORT));
}

int LoRaClass::packetRssi(void) 
{
   return (readReg(REG_PKT_RSSI_VALUE) - (__frequency < RF_MID_BAND_THRESHOLD ? RSSI_OFFSET_LF_PORT : RSSI_OFFSET_HF_PORT));
}

float LoRaClass::packetSnr(void) 
{
   return ((int8_t)readReg(REG_PKT_SNR_VALUE)) * 0.25;
}

long LoRaClass::packetFrequencyError(void)
{  
   int32_t freqError = 0;
   freqError = static_cast<int32_t>(readReg(REG_FREQ_ERROR_MSB) & 0b111);
   freqError <<= 8L;
   freqError += static_cast<int32_t>(readReg(REG_FREQ_ERROR_MID));
   freqError <<= 8L;
   freqError += static_cast<int32_t>(readReg(REG_FREQ_ERROR_LSB));

   // Sign bit is on
   if (readReg(REG_FREQ_ERROR_MSB) & 0b1000) freqError -= 524288; // 0b1000'0000'0000'0000'0000

   // FXOSC: crystal oscillator (XTAL) frequency (2.5. Chip Specification, p. 14)
   const float fXtal = 32E6;
   const float fError = ((static_cast<float>(freqError) * (1L << 24)) / fXtal) * (getBandwidth() / 500000.0f); // p. 37

   return static_cast<long>(fError);
}

LoRaClass LoRa;

}