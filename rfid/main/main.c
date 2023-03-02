#include <esp_log.h>
#include  <inttypes.h>
#include "rc522.h"
#include "driver/gpio.h"
#include "stepper.h"

static const char* TAG = "rc522";
static rc522_handle_t scanner;
bool door_open;
long long int allowed_tag = 967306403452;



static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data)
{
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;

    switch(event_id) {
        case RC522_EVENT_TAG_SCANNED: {
                rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
                ESP_LOGI(TAG, "Tag scanned (sn: %" PRIu64 ")", tag->serial_number);
                if(tag->serial_number == allowed_tag ){
                    ESP_LOGI(TAG, "Access Granted");
                    struct stepper_pins stepper0;
                    stepper0.pin1 = 12;
                    stepper0.pin2 = 14;
                    stepper0.pin3 = 27;
                    stepper0.pin4 = 26;    
   
                    stepper_init(&stepper0);
                    

                    full_steps(&stepper0, 100, CW);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);   
                    full_steps(&stepper0, 100, CCW); 
                    vTaskDelay(1000 / portTICK_PERIOD_MS);  


                }
                else{
                    ESP_LOGI(TAG, "Access Denied");

                }

            }
            break;
    }
}

void app_main(){
    door_open = false;
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