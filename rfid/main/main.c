#include <esp_log.h>
#include  <inttypes.h>
#include "rc522.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "esp_adc/adc_continuous.h"


static const char* TAG = "rc522";
;
static rc522_handle_t scanner;
long long int allowed_tag = 967306403452;

void sweepServo_task(void *ignore) {
    static const char* servo_tag = "servo1";
	int bitSize         = 15;
	int minValue        = 500;  // micro seconds (uS)
	int maxValue        = 2500; // micro seconds (uS)
	int sweepDuration   = 3000; // milliseconds (ms)
	int duty            = (1<<bitSize) * minValue / 20000 ;//1638
	int direction       = 1; // 1 = up, -1 = down
	int valueChangeRate = 20; // msecs
	ESP_LOGD(servo_tag, ">> task_servo1");
	ledc_timer_config_t timer_conf;
	timer_conf.duty_resolution    = LEDC_TIMER_15_BIT;
	timer_conf.freq_hz    = 50;
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.timer_num  = LEDC_TIMER_0;
	ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf;
	ledc_conf.channel    = LEDC_CHANNEL_0;
	ledc_conf.duty       = duty;
	ledc_conf.gpio_num   = 12;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_conf.timer_sel  = LEDC_TIMER_0;
	ledc_channel_config(&ledc_conf);

	int changesPerSweep = sweepDuration / valueChangeRate;// 1500/20 -> 75
	int changeDelta = (maxValue-minValue) / changesPerSweep;// 2000/75 -> 26
	int i;
	ESP_LOGD(servo_tag, "sweepDuration: %d seconds", sweepDuration);
	ESP_LOGD(servo_tag, "changesPerSweep: %d", changesPerSweep);
	ESP_LOGD(servo_tag, "changeDelta: %d", changeDelta);
	ESP_LOGD(servo_tag, "valueChangeRate: %d", valueChangeRate);
	for(int j = 0; j < 2; j++) {
        for (i=0; i<changesPerSweep; i++) {
            if (direction > 0) {
                duty += changeDelta;
            } else {
                duty -= changeDelta;
            }
            
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(valueChangeRate/portTICK_PERIOD_MS);
        }
        direction = -direction;
        ESP_LOGD(servo_tag, "Direction now %d", direction);
        vTaskDelay(100);
    

	} // End loop forever

	vTaskDelete(NULL);
}
//..........................................................................................


static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data)
{
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;

    switch(event_id) {
        case RC522_EVENT_TAG_SCANNED: {
                rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
                ESP_LOGI(TAG, "Tag scanned (sn: %" PRIu64 ")", tag->serial_number);
                if(tag->serial_number == allowed_tag ){
                    ESP_LOGI(TAG, "Access Granted");

                    

                    xTaskCreate(&sweepServo_task,"sweepServo_task",2048,NULL,5,NULL);

                    ESP_LOGI(TAG, "Door Closed");
                }
                else{
                    ESP_LOGI(TAG, "Access Denied");

                }

            }
            break;
    }
}

void app_main(){
  
    rc522_config_t config = {
        .spi.host = VSPI_HOST,
        .spi.miso_gpio = 25,
        .spi.mosi_gpio = 23,
        .spi.sck_gpio = 19,
        .spi.sda_gpio = 22,
    };

    
    // nvs_flash_init();
    rc522_create(&config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_ANY, rc522_handler, NULL);
    rc522_start(scanner);


}