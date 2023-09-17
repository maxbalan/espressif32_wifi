#include <esp_wifi.h>
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "nvs_flash.h"
#include "WifiHelper.h"

static const char *TAG_AP = "WiFi SoftAP";
static const char *TAG_STA = "WiFi Sta";
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data) {
   if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "Station started");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_STA, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void connectWifi(wifi_config_t wifiConfig) {
    esp_event_loop_create_default();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());

    /* Initialize event group */
    s_wifi_event_group = xEventGroupCreate();

    /* Register Event handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    //     init wifi
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiInitConfig));


    // configure WIFI AP
    // wifi_config_t config = {
    //     .sta = {
    //         .ssid = wifiConfig->ssid,
    //         .password = wifiConfig->password,
    //     },
    // };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));


    // connect WIFI
    ESP_ERROR_CHECK(esp_wifi_start());


    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_STA, "connected to ap SSID:%s password:%s", wifiConfig.sta.ssid, wifiConfig.sta.password);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_STA, "Failed to connect to SSID:%s, password:%s", wifiConfig.sta.ssid, wifiConfig.sta.password);
    } else {
        ESP_LOGE(TAG_STA, "UNEXPECTED EVENT");
        return;
    }
}