# ESP
in order to start with esp you need to download toolchain for the esp32,some of the options are:
1. platformio
2. esp-idf

### we will use platformio but esp-idf is the same

# Serial comunication to pc
platformio
to setup the monitoring in vscode and platformio add the line:
```
monitor_speed = 115200
```
to the file platformio.ini

in order to print from the esp to PC:
```c
#include <stdio.h>
void app_main(void) {
    printf("Hello world!\n");
}
```
to write from pc to esp32 for now: - needs testing -
```c
    while (1) {
        uint8_t ch;
        ch = fgetc(stdin); // input non blocking
        if (ch != 0xFF) {
            fputc(ch, stdout); // output
        }
    }
```

upload to esp32 cam:
connect pin IO0 to ground and restart or plug in


SD control:
The process is 3 steps process:
1. mount
2. use normal C file work
3. unmount

1 – mount:

// objects
#include <string.h>
#include <sys/unistd.h>

//functions
#include <sys/stat.h> //for checking if file exist
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

// static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"

void app_main(void)
{
esp_err_t ret;

// Options for mounting the filesystem.
// If format_if_mount_failed is set to true, SD card will be partitioned and
// formatted in case when mounting fails.
esp_vfs_fat_sdmmc_mount_config_t mount_config = {
.format_if_mount_failed = false,
.max_files = 5, //Max number of open files
.allocation_unit_size = 16 * 1024};

sdmmc_card_t *card; //the object that will hold the card
const char mount_point[] = MOUNT_POINT; //name of card root pos

// Use settings defined above to initialize SD card and mount FAT filesystem.

sdmmc_host_t host = SDMMC_HOST_DEFAULT(); //the HW config data of the sd

// This initializes the slot without card detect (CD) and write protect (WP) signals.
// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
slot_config.width = 4;

// Enable internal pullups on enabled pins.
slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

//the main mounting function:
//mount the card on card with mount_point position
ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

if (ret != ESP_OK) //checks for errors
{
if (ret == ESP_FAIL)
{
//Failed to mount filesystem
}
else
{
//Failed to initialize the card -> esp_err_to_name(ret)
}
return;
}
// ********************************************* //

2 – C filesystem:
// Use POSIX and C standard library functions to work with files:
// (normal C file usage)
// First create a file.
const char *file_hello = MOUNT_POINT "/hello.txt";

//Opening file
FILE *f = fopen(file_hello, "w");
if (f == NULL)
{
//Failed to open file for writing
return;
}
fprintf(f, "Hello %s!\n", card->cid.name);
fclose(f);
//File written

const char *file_foo = MOUNT_POINT "/foo.txt";

// Check if destination file exists before renaming
struct stat st;
if (stat(file_foo, &st) == 0)
{
// Delete it if it exists
unlink(file_foo);
}

// Rename original file
if (rename(file_hello, file_foo) != 0)
{
//Rename failed
return;
}

// Open renamed file for reading
f = fopen(file_foo, "r");
if (f == NULL)
{
//Failed to open file for reading
return;
}

// Read a line from file
char line[64];
fgets(line, sizeof(line), f);
fclose(f);

// Strip newline
char *pos = strchr(line, '\n');
if (pos)
{
*pos = '\0';
}
//******************************//

3 – unmount:
// All done, unmount partition and disable SDMMC peripheral
esp_vfs_fat_sdcard_unmount(mount_point, card);
}

-> one point: the led and data1 is shered on pin4
camera control:
esp32-camera-github
first we need to clone the git and add it to the project or in platformio:
It's probably easier to just skip the platform.io library registry version and link the git repo as a submodule. (i.e. using code outside the platform.io library management). In this example we will install this as a submodule inside the platform.io $project/lib folder:
cd $project\lib
git submodule add -b master https://github.com/espressif/esp32-camera.git

Then in platformio.ini file

build_flags =
   -I../lib/esp32-camera
After that #include "esp_camera.h" statement will be available.

Usage:

#include "esp_camera.h"

#define CAM_PIN_PWDN -1 //power down is not used
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 21
#define CAM_PIN_SIODa 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 19
#define CAM_PIN_D2 18
#define CAM_PIN_D1 5
#define CAM_PIN_D0 4
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

static camera_config_t camera_config = {
.pin_pwdn = CAM_PIN_PWDN,
.pin_reset = CAM_PIN_RESET,
.pin_xclk = CAM_PIN_XCLK,
.pin_sscb_sda = CAM_PIN_SIOD,
.pin_sscb_scl = CAM_PIN_SIOC,

.pin_d7 = CAM_PIN_D7,
.pin_d6 = CAM_PIN_D6,
.pin_d5 = CAM_PIN_D5,
.pin_d4 = CAM_PIN_D4,
.pin_d3 = CAM_PIN_D3,
.pin_d2 = CAM_PIN_D2,
.pin_d1 = CAM_PIN_D1,
.pin_d0 = CAM_PIN_D0,
.pin_vsync = CAM_PIN_VSYNC,
.pin_href = CAM_PIN_HREF,
.pin_pclk = CAM_PIN_PCLK,

.xclk_freq_hz = 20000000,
.ledc_timer = LEDC_TIMER_0,
.ledc_channel = LEDC_CHANNEL_0,

.pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
.frame_size = FRAMESIZE_UXGA, //QQVGA-QXGA Do not use sizes above QVGA when not JPEG

.jpeg_quality = 12, //0-63 lower number means higher quality
.fb_count = 1, //if more than one, i2s runs in continuous mode. Use only with JPEG
.grab_mode = CAMERA_GRAB_WHEN_EMPTY //CAMERA_GRAB_LATEST. Sets when buffers should be filled
};

esp_err_t camera_init()
{

//initialize the camera
esp_err_t err = esp_camera_init(&camera_config);
if (err != ESP_OK)
{
//Camera Init Failed
return err;
}

return ESP_OK;
}

esp_err_t camera_capture()
{
//acquire a frame
camera_fb_t *fb = esp_camera_fb_get();
if (!fb)
{
//Camera Capture Failed
return ESP_FAIL;
}
//replace this with your own function
//process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);

//return the frame buffer back to the driver for reuse
esp_camera_fb_return(fb);
return ESP_OK;
}
 
video is just lots of pics together ;)

wifi connectivity:
wifi examples
todo: wifi power save mode
ap scan:
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#define DEFAULT_SCAN_LIST_SIZE 15

static const char *TAG = "scan";

static void print_auth_mode(int authmode)
{
switch (authmode)
{
case WIFI_AUTH_OPEN:
ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
break;
case WIFI_AUTH_WEP:
ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
break;
case WIFI_AUTH_WPA_PSK:
ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
break;
case WIFI_AUTH_WPA2_PSK:
ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
break;
case WIFI_AUTH_WPA_WPA2_PSK:
ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
break;
case WIFI_AUTH_WPA2_ENTERPRISE:
ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
break;
case WIFI_AUTH_WPA3_PSK:
ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
break;
case WIFI_AUTH_WPA2_WPA3_PSK:
ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
break;
default:
ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
break;
}
}

/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{
//********* WIFI configurations ********
//Initialize the underlying TCP/IP stack
ESP_ERROR_CHECK(esp_netif_init());
//Create default event loop
ESP_ERROR_CHECK(esp_event_loop_create_default());
esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
assert(sta_netif);
//WIFI configuration and Init WiFi Alloc resource for WiFi driver
wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//********* end WIFI configurations ********

//********* WIFI scanning ********
//variables to catch the wifi info
uint16_t number = DEFAULT_SCAN_LIST_SIZE; //list len
wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE]; //the list
uint16_t ap_count = 0;
memset(ap_info, 0, sizeof(ap_info)); //zero the list

ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
ESP_ERROR_CHECK(esp_wifi_start());
esp_wifi_scan_start(NULL, true); //Scan all available APs
//********* get the data and print **********
ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++)
{
ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
print_auth_mode(ap_info[i].authmode);
ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
}
}

void app_main(void)
{
// Initialize NVS - needed for WIFI
esp_err_t ret = nvs_flash_init();
if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
{ //if no memory in NVS
ESP_ERROR_CHECK(nvs_flash_erase());
ret = nvs_flash_init();
}
ESP_ERROR_CHECK(ret);
//the scan:
wifi_scan();
}

connect to wifi:
The ESP can make an AP (acces point), STA (stationary) or eve both.
both but bad code(part of the code):
// initialize NVS (nvs_flash_init)
//********* WIFI configurations ********
tcpip_adapter_init();

wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA)); //ap or sta or both
//STA config
wifi_config_t sta_config = {
.sta = {
.ssid = "network to connect into",
.password = "the network password"},
};
// for STA:
ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
//AP config
wifi_config_t ap_config = {
.ap = {
.ssid = AP_SSID,
.password = AP_PASSWORD,
.max_connection = AP_MAX_CONN,
.channel = AP_CHANNEL,
.ssid_hidden = 0,
},
};
//for AP:
ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
ESP_ERROR_CHECK(esp_wifi_start()); // starts the wifi AP and or STA
ESP_ERROR_CHECK(esp_wifi_connect()); // connect to wifi (STA)




sta connection:
void app_main(void)
{ // initialize NVS (nvs_flash_init)
ESP_ERROR_CHECK(nvs_flash_init());
//********* WIFI configurations ********
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());
esp_netif_create_default_wifi_sta();

wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));

wifi_config_t sta_config = {
.sta = {
.ssid = "the name of the network",
.password = "password"},
};
ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); //ap or sta or both
ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
ESP_ERROR_CHECK(esp_wifi_start()); // starts the wifi AP
//********* WIFI connection ********
ESP_ERROR_CHECK(esp_wifi_connect()); // connect to wifi (STA) need to check
}










Free RTOS:
It stand for:
Free Real Time Operation System
and it give the option to run more then one task, semaphors, interapts and more.
it runs on esp stm atmel and even arduino.
freeRTOS doc
tasks example:
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t myTask1Handle = NULL;
TaskHandle_t myTask2Handle = NULL;

void task1(void *arg)
{
printf("the task2 has name: %s \n", pcTaskGetTaskName(myTask2Handle));
int c = 0;
while (1)
{ //The count of ticks since vTaskStartScheduler was called
c = xTaskGetTickCount();
printf("[%d] hello from task1 \n", c);
//Delay a task for a given number of ticks
vTaskDelay(1000 / portTICK_RATE_MS);
if (c == 300)
{
vTaskSuspend(myTask2Handle);
printf("task2 is suspended! %d \n", eTaskGetState(myTask2Handle));
}
if (c == 500)
{
vTaskResume(myTask2Handle);
printf("task2 is resumed! %d \n", eTaskGetState(myTask2Handle));
}
if (c == 700)
{
vTaskDelete(myTask2Handle);
printf("task2 is deleted! %d \n", eTaskGetState(myTask2Handle));
}
/*
eTaskGetState returns:
Running = 0,
Ready = 1,
Blocked = 2,
Suspended= 3,
Deleted = 4,
Invalid = 5
*/
}
}
void task2(void *arg) //task 2 function
{
for (int i = 0; i < 10; i++)
{
printf("hello from task2 \n");
vTaskDelay(1000 / portTICK_RATE_MS);
}
}
void app_main()
{
// creates task 1
xTaskCreate(task1, "task1", 4096, NULL, 10, &myTask1Handle);
//create task 2 but pin it to core 1
xTaskCreatePinnedToCore(task2, "task2", 4096, NULL, 10, &myTask2Handle, 1);
}

networking:
github of examples
The examples include:
Http server and client, Sockets and more.
gpio and interrapt:
Example for interapt, task and gpio. pressing botton calling the ISR and turn on the task that turn on the led:
#include <stdio.h>
#include "driver/gpio.h" //for GPIO
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define CONFIG_LED_PIN 2 //IO2
#define ESP_INTR_FLAG_DEFAULT 0
#define CONFIG_BUTTON_PIN 0 //IO0 i.e. boot

TaskHandle_t ISR = NULL;

// interrupt function
void IRAM_ATTR button_isr_handler(void *arg)
{
//make task 1 go back from sleep
xTaskResumeFromISR(ISR);
//portYIELD_FROM_ISR( );
}

// task that turn on led
void button_task(void *arg)
{
bool led_status = false;
while (1)
{
//go to sleep
vTaskSuspend(NULL);
led_status = !led_status;
gpio_set_level(CONFIG_LED_PIN, led_status);
printf("Button pressed\n");
}
}
//**************************************
void app_main()
{
//(set the MUX to get data from GPIOs)
gpio_pad_select_gpio(CONFIG_BUTTON_PIN); //IO0
gpio_pad_select_gpio(CONFIG_LED_PIN); //IO2

// set input or output
gpio_set_direction(CONFIG_BUTTON_PIN, GPIO_MODE_INPUT);
gpio_set_direction(CONFIG_LED_PIN, GPIO_MODE_OUTPUT);

//***** interupt setup ****
// enable interrupt on falling edge (1->0) for button pin
gpio_set_intr_type(CONFIG_BUTTON_PIN, GPIO_INTR_NEGEDGE);
//Install the driver’s GPIO ISR handler service, which allows per-pin GPIO interrupt handlers.
// install ISR service with default configuration
gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
// attach the interrupt to a function
gpio_isr_handler_add(CONFIG_BUTTON_PIN, button_isr_handler, NULL);
//***** end interupt setup ****

//Create and start a task
xTaskCreate(button_task, "button_task", 4096, NULL, 10, &ISR);
}

