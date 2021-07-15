
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

    sdmmc_card_t *card;                     //the object that will hold the card
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
    // All done, unmount partition and disable SDMMC peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
}