/**
 *	LoRa Receiver Example
 *	This is a simple code for reading Lora packets
 *
 *	created 19 June 2024
 *	by Filipe I. Teixeira
 */


#include <stdio.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "lora.h"
using namespace LORA;


extern "C" void app_main()
{
	// Initialize LoRa
	if (!LoRa.init()) {
		ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the module");
		while(1) vTaskDelay(1);
	}
	// Changes sync word byte
	LoRa.setSyncWord(0x12);

	std::string msg;

	while(1) {
		// Put into receive mode
		LoRa.receive();

		// Reads packet
		if (LoRa.parsePacket()) {
			msg = LoRa.receivePacket();
			ESP_LOGI(pcTaskGetName(NULL), "Received Message: %s (RSSI: %d)", msg.c_str(), LoRa.packetRssi());
		}
		vTaskDelay(1);
	}
}
