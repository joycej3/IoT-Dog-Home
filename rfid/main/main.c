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


#include <string.h>
#include <sys/param.h>

#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "addr_from_stdin.h"
#include "lwip/err.h"
#include "lwip/sockets.h"


// #include "py/compile.h"
// #include "py/runtime.h"
// #include "py/gc.h"
// #include "py/mpstate.h"
// #include "py/mphal.h"
// #include "client.py"##

#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_IPV6)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
#define HOST_IP_ADDR "172.20.10.9 8080"
#endif

#define PORT CONFIG_EXAMPLE_PORT

static const char *TAG = "example";
static const char *payload = "Door Open";

static void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {
#if defined(CONFIG_EXAMPLE_IPV4)
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(host_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_IPV6)
        struct sockaddr_in6 dest_addr = { 0 };
        inet6_aton(host_ip, &dest_addr.sin6_addr);
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
        struct sockaddr_storage dest_addr = { 0 };
        ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_STREAM, &ip_protocol, &addr_family, &dest_addr));
#endif
        int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Successfully connected");

        while (1) {
        int err = send(sock, payload, strlen(payload), 0);
        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            break;
        }

        int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        // Error occurred during receiving
        if (len < 0) {
            ESP_LOGE(TAG, "recv failed: errno %d", errno);
            break;
        }
        // Data received
        else {
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
            ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
            ESP_LOGI(TAG, "%s", rx_buffer);
            break;
        }

        // vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}


static const char* nfc_TAG = "rc522";
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
                ESP_LOGI(nfc_TAG, "Tag scanned (sn: %" PRIu64 ")", tag->serial_number);
                if(tag->serial_number == allowed_tag ){
                    ESP_LOGI(nfc_TAG, "Access Granted");

                    

                    xTaskCreate(&sweepServo_task,"sweepServo_task",2048,NULL,5,NULL);
                    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);

                    ESP_LOGI(nfc_TAG, "Door Closed");
                }
                else{
                    ESP_LOGI(nfc_TAG, "Access Denied");

                }

            }
            break;
    }
}

void app_main(){

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    
  
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