/**
 *	LoRa Sender Example
 *	This code sends a simple packet containing a 'Hello World' using Lora.
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

	// Message to lora
	std::string message = "Hello World in LoRa!";

    while (true) {
    	// Sending outgoing to LoRa receiver
		if(LoRa.sendPacket(message)) {
			ESP_LOGI(pcTaskGetName(NULL), "Sent: '%s'.", message.c_str());
		} else {
			ESP_LOGI(pcTaskGetName(NULL), "Error in transmission.");
		}

		// Informs if packets were lost during transmission
		int lost = LoRa.packetLost();
		if (lost != 0) {
			ESP_LOGW(pcTaskGetName(NULL), "%d packets lost.", lost);
		}

		vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
