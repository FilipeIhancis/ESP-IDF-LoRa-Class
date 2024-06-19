# ESP-IDF LoRa Class API

The idea of ​​the developed class is to be as close as possible to the Arduino library class "[Arduino-LoRa](https://github.com/sandeepmistry/arduino-LoRa)". Some methods are very similar.

See **[examples folder](examples)**.

## Include
~~~cpp
#include <string>
#include "lora.h"
using namespace LORA;
~~~

## Begin (init)

~~~cpp
LoRa.init();
LoRa.init(long freq, int miso, int mosi, int sck, int cs, int rst);
~~~

The parameters ``freq``, ``miso``, ``mosi``, ``sck``, ``cs`` and ``rst`` are optional. Replaces standard pins.

Returns `1` if radio is ready to transmit, `0` if busy or on failure.

## SPI Transmission Type

Decides type of management of SPI data transmissions. By default ``spiChoose`` equals ``true``.

~~~cpp
LoRa.asyncSPI(bool spiChoose);
~~~

- If **``spiChoose``** equals ``true``, the SPI transmissions will be performed asynchronously.
- If **``spiChoose``** equals ``false``, the SPI transmissions will be performed synchronously.

## Radio Mode

### Continuous Receive Mode / Idle Mode

Put the radio in continuous receive mode or idle (standby) mode.

~~~cpp
LoRa.receive();
LoRa.idle();
~~~

### Receive / Transmission Mode (Node-Gateway)

Puts the Lora radio in continuous receive mode or packet transmission mode by enabling/disabling the inversion of the Lora I and Q signals **(invertIQ)**. 

**Used on communication between devices and gateway.**

#### Rx Mode
~~~cpp
LoRa.rxModeNode();
LoRa.rxModeGateway();
~~~
Puts **node** in receives mode with InvertIQ and **gateway** in receive mode without InvertIQ.

#### Tx Mode
~~~cpp
LoRa.txModeNode();
LoRa.txModeGateway();
~~~
Puts **node** in transmission mode without InvertIQ and **gateway** in transmission mode with InvertIQ.

### 

## Sending Data

### Send Packet

~~~cpp
LoRa.sendPacket(std::string message);
~~~
- `message`: Message that will be sent via radio Lora.

Returns ``1`` if the transmission completed successfully or ``0`` if the transmission did not complete within the expected time.

### Packet Lost

Number of packets lost during transmission.

~~~cpp
int pktLost = LoRa.packetLost();
~~~

## Receiving Data

### Parse Packet

Analogous to Arduino LoRa's "_parsePacket()_" method.

~~~cpp
LoRa.parsePacket();
~~~

Returns ``1`` if any packet was received or ``0`` if no packet was received.

### Received Packet

~~~cpp
std::string msg = LoRa.receivePacket();
~~~

Returns the received packet message in a string or `""` if an error occurred.

### Packet RSSI (dBm), Current RSSI (dBm) and Packet SNR (dB)

~~~cpp
int rssi = LoRa.packetRssi();
int rssi = LoRa.rssi();
float snr = LoRa.packetSnr();
~~~

## Over Current Protection

~~~cpp
Lora.setOverCurrentProtection(uint8_t maxCurrent);
~~~

Sets a limit on the current drain of the Power Amplifier. Checks **[RFM9X(W) datasheet](https://cdn.sparkfun.com/assets/learn_tutorials/8/0/4/RFM95_96_97_98W.pdf)**.

## Radio Parameters

the user can change parameters of the lora radio, respecting the possible value ranges of the arguments

~~~cpp
LoRa.setFrequency(long frequency);      // Changes Frequency
LoRa.setTxPower(int level);             // Changes Tx Power (dB)
LoRa.setLnaGain(int gain);              // Changes LNA gain
LoRa.setSyncWord(int sw);               // Sets Sync Word byte
LoRa.setBandwidth(int sbw);             // Changes Bandwidth
LoRa.setSpreadingFactor(int sf);        // Changes Spreading Factor
LoRa.setCodingRate(int denominator);    // Changes coding rate denominator
LoRa.enableCrc();                       // Enable CRC usage
LoRa.disableCrc();                      // Disable CRC usage
LoRa.setPreambleLength(long length);    // Changes the preamble length
~~~

- **``frequency``**: LoRa radio frequency. Check the possible frequencies of your module and whether they are compatible with local regulations

- **``level``**: Tx Power in dB. Supported values are ``2`` to ``17`` (RFM9XX modules)

- **``gain``**: LNA gain for better Rx sensitivity. Supported values are ``0`` to ``6``. If ``gain`` equals ``0``, enable *Automatic Gain Control*.
- **``sbw``**: Bandwidth selector. Supported values are ``1`` to ``10``. Checks the *[RFM9XX datasheet](https://cdn.sparkfun.com/assets/learn_tutorials/8/0/4/RFM95_96_97_98W.pdf)*

    <div align="center">

    |sbw| Bandwidth (kHz) |sbw| Bandwidth (kHz) |
    |:-:|:-:|:-:|:-:| 
    |1| 7.8 |6 |41.7 |
    |2| 10.4 |7 |62.5 |
    |3| 15.6 |8 |125 | 
    |4| 20.8 |9 |250 |
    |5| 31.25 |10 |500 |
    
    </div>

- **``sf``**: Spreading Factor of the radio. Supported values are between ``6`` and ``12``. If a spreading factor of ``6`` is set, *implicit header* mode must be used to transmit and receive packets

- **``denominator``**: Denominator of the coding rate. Supported values are between ``5`` and ``8``, these correspond to coding rates of ``4/5`` and ``4/8``

- **``length``**: Preamble length in symbols. Supported values are between `6` and `65535`
