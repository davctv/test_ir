/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "esp_log.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "driver/rmt.h"


#define TAG_RC5 "example"

#define RMT_RX_GPIO_NUM  23   			   /*!< GPIO number for receiver */

#define RMT_CLK_DIV      		80    			/*!< RMT counter clock divider, 100 */
#define RMT_TICK_10_US    		(80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */
#define rmt_item32_tIMEOUT_US  4000   /*!< RMT receiver timeout value(us) */



static TaskHandle_t IR_TaskHandle;

/**
 * @brief RMT receiver demo, this task will print each received NEC data.
 *
 */
static void IR_RC5_task()
{

	//int channel = RMT_RX_CHANNEL;
	RingbufHandle_t rb = NULL;

    rmt_config_t rmt_rx;
    rmt_rx.channel = RMT_CHANNEL_1;
    rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
    rmt_rx.clk_div = RMT_CLK_DIV;
    rmt_rx.mem_block_num = 1;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en = true;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
    esp_err_t ret = rmt_config(&rmt_rx);

    ESP_LOGD(TAG_RC5, "RC5 cfg:%d", ret);

    ret = rmt_driver_install(rmt_rx.channel, 1000, 0);

    ESP_LOGD(TAG_RC5, "RC5 inst:%d", ret);


	//get RMT RX ringbuffer
	ret = rmt_get_ringbuf_handle(RMT_CHANNEL_1, &rb);

	ESP_LOGD(TAG_RC5, "RC5 ring:%d", ret);

	ret = rmt_rx_start(RMT_CHANNEL_1, 1);

	ESP_LOGD(TAG_RC5, "RC5 start:%d", ret);

	while(rb)
	{
		size_t rx_size = 0;
		//try to receive data from ringbuffer.
		//RMT driver will push all the data it receives to its ringbuffer.
		//We just need to parse the value and return the spaces of ringbuffer.
		rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 1000);


		if(item)
		{

			//after parsing the data, return spaces to ringbuffer.
			vRingbufferReturnItem(rb, (void*) item);
		}
	}


    ESP_LOGD(TAG_RC5, "exit RC5 task");

    vTaskDelete(NULL);
}


//Inizializza la task per l'infrarosso
void DeInit_IR_Service(void)
{
	ESP_LOGD(TAG_RC5, "DeInit");

    ESP_ERROR_CHECK(rmt_rx_stop(RMT_CHANNEL_1));

    ESP_ERROR_CHECK(rmt_driver_uninstall(RMT_CHANNEL_1));

    vTaskDelete(IR_TaskHandle);
}



//Inizializza la task per l'infrarosso
void app_main(void)
{
	ESP_LOGD(TAG_RC5, "Init");

    xTaskCreate(IR_RC5_task, "ir_t", 2048, NULL, 8, &IR_TaskHandle);


    for (uint8_t i = 0; i < 10; i++)
    {
    	vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    DeInit_IR_Service();
}

