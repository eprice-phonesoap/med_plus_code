/**
@file			  iap.c
@author			  Jey
@brief			  IAP Module related functions

@copyright		  Pura Scents

@attention		  The information contained herein is confidential property of
                  Pura. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Pura.
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.

#include "iap.h"

//_____D E F I N E __________________________________________________________
// local private defines, enums, macros
#define TAG "iap"

#define IAP_STATE_INITIALIZED   (1 << 0)
#define IAP_STATE_SESSION_OPEN  (1 << 1)

// While the session is open ('iap_begin' called), this module uses a
// heap-allocated page buffer to accumulate data for writing.
#define IAP_PAGE_SIZE 4096

#define MIN(a, b) ((a) < (b) ? (a) : (b))

//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions
// Internal state of this module.
typedef struct iap_internal_state_
{
    // Set after init_iap has completed successfully.
    uint8_t module_state_flags;

    // Partition which will contain the new firmware image.
    const esp_partition_t *partition_to_program;

    // Handle for OTA functions.
    esp_ota_handle_t ota_handle;

    // Pointer to the next byte in flash memory that will be written by iap_write.
    uint32_t cur_flash_address;

    // Pointer to a 4k block to accumulate data for page writes.
    uint8_t page_buffer[4096];

    // Index into the page buffer.
    uint16_t page_buffer_ix;

} iap_internal_state_t;

//_____ P R O T O T Y P E S _________________________________________________
// local function declarations
static iap_internal_state_t iap_state;


static iap_err_t iap_write_page_buffer();
static iap_err_t iap_finish(int commit);
static const esp_partition_t *iap_find_next_boot_partition();

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)

//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions

//_____ L O C A L  F U N C T I O N S ________________________________________
// local interface functions
iap_err_t iap_init()
{
    ESP_LOGD(TAG, "iap_init");

    // Only allowed once.
    if (iap_state.module_state_flags & IAP_STATE_INITIALIZED) {
        ESP_LOGE(TAG, "iap_init: The module has already been initialized!");
        return IAP_ERR_ALREADY_INITIALIZED;
    }

    iap_state.module_state_flags = IAP_STATE_INITIALIZED;

    return IAP_OK;
}

iap_err_t iap_begin()
{
    ESP_LOGD(TAG, "iap_begin");

    // The module needs to be initialized for this method to work.
    if (!(iap_state.module_state_flags & IAP_STATE_INITIALIZED)) {
        ESP_LOGE(TAG, "iap_begin: the module hasn't been initialized!");
        return IAP_ERR_NOT_INITIALIZED;
    }

    // It's not permitted to call iap_begin if the previous programming session is still open.
    if (iap_state.module_state_flags & IAP_STATE_SESSION_OPEN) {
        ESP_LOGE(TAG, "iap_begin: Session already open!");
        return IAP_ERR_SESSION_ALREADY_OPEN;
    }

    // We use a 4k page buffer to accumulate bytes for writing.
    iap_state.page_buffer_ix = 0;

#if 0
    iap_state.page_buffer = malloc(IAP_PAGE_SIZE);
    if (!iap_state.page_buffer) {
        ESP_LOGE(TAG, "iap_begin: not enough heap memory to allocate the page buffer!");
        return IAP_ERR_OUT_OF_MEMORY;
    }
#endif

    iap_state.partition_to_program = iap_find_next_boot_partition();
    if (!iap_state.partition_to_program) {
        ESP_LOGE(TAG, "iap_begin: partition for firmware update not found!");
        //free(iap_state.page_buffer);
        return IAP_ERR_PARTITION_NOT_FOUND;
    }

    ESP_LOGD(TAG, "iap_begin: next boot partition is '%s'.", iap_state.partition_to_program->label);

    iap_state.cur_flash_address = iap_state.partition_to_program->address;

    esp_err_t result = esp_ota_begin(iap_state.partition_to_program, 0, &iap_state.ota_handle);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "iap_begin: esp_ota_begin failed (%d)!", result);
        //free(iap_state.page_buffer);
        return IAP_FAIL;
    }

    ESP_LOGI(TAG, "iap_begin: opened IAP session for partition '%s', address 0x%08x.",
             iap_state.partition_to_program->label, iap_state.cur_flash_address);

    iap_state.module_state_flags |= IAP_STATE_SESSION_OPEN;
    return IAP_OK;
}

iap_err_t iap_write(uint8_t *bytes, uint16_t len)
{
    ESP_LOGD(TAG, "iap_write(bytes = %p, len = %u)", bytes, len);

    // The module needs to be initialized for this method to work.
    if (!(iap_state.module_state_flags & IAP_STATE_INITIALIZED)) {
        ESP_LOGE(TAG, "iap_write: the module hasn't been initialized!");
        return IAP_ERR_NOT_INITIALIZED;
    }

    // The session needs to be open for this method to work.
    if (!(iap_state.module_state_flags & IAP_STATE_SESSION_OPEN)) {
        ESP_LOGE(TAG, "iap_write: programming session not open!");
        return IAP_ERR_NO_SESSION;
    }

    ESP_LOGD(TAG, "iap_write: cur_flash_address = 0x%08x", iap_state.cur_flash_address);

    while (len > 0) {

        uint16_t spaceRemaining = IAP_PAGE_SIZE - iap_state.page_buffer_ix;
        uint16_t nofBytesToCopy = MIN(spaceRemaining, len);

        memcpy(&iap_state.page_buffer[iap_state.page_buffer_ix], bytes, nofBytesToCopy);

        iap_state.page_buffer_ix += nofBytesToCopy;
        bytes += nofBytesToCopy;
        len -= nofBytesToCopy;

        // Page buffer full?
        if (iap_state.page_buffer_ix == IAP_PAGE_SIZE) {

            // Erase flash pages at 4k boundary.
            //int flashSectorToErase = iap_state.cur_flash_address / 0x1000;
            //ESP_LOGD(TAG, "iap_write: Erasing flash sector %d (0x%08x)",
            //         flashSectorToErase, iap_state.cur_flash_address);
            //spi_flash_erase_sector(flashSectorToErase);

            // Write page buffer to flash memory.
            esp_err_t result = iap_write_page_buffer();

            if (result != ESP_OK) {
                ESP_LOGE(TAG, "iap_write: write failed (%d)!", result);
                return IAP_ERR_WRITE_FAILED;
            }
        }
    }

    return IAP_OK;
}

iap_err_t iap_commit()
{
    ESP_LOGD(TAG, "iap_commit");

    iap_err_t result = iap_write_page_buffer();
    if (result != IAP_OK) {
        ESP_LOGE(TAG, "iap_commit: programming session failed in final write.");
    }

    result = iap_finish(1);
    if (result != IAP_OK) {
        ESP_LOGE(TAG, "iap_commit: programming session failed in iap_finish.");
    }

    ESP_LOGI(TAG, "iap_commit: programming session successfully completed, partition activated.");
    return result;
}

iap_err_t iap_abort()
{
    ESP_LOGD(TAG, "iap_abort");

    iap_err_t result = iap_finish(0);
    if (result == IAP_OK) {
        ESP_LOGI(TAG, "iap_abort: programming session successfully aborted.");
    }

    return result;
}

static iap_err_t iap_write_page_buffer()
{
    ESP_LOGD(TAG, "iap_write_page_buffer");
    if (iap_state.page_buffer_ix == 0) {
        return IAP_OK;
    }

    ESP_LOGI(TAG, "iap_write_page_buffer: writing %u bytes to address 0x%08x",
             iap_state.page_buffer_ix, iap_state.cur_flash_address);
    esp_err_t result = esp_ota_write(iap_state.ota_handle, iap_state.page_buffer, iap_state.page_buffer_ix);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "iap_write_page_buffer: write failed in esp_ota_write (%d)!", result);
        return IAP_ERR_WRITE_FAILED;
    }

    iap_state.cur_flash_address += iap_state.page_buffer_ix;

    // Set page buffer index back to the start of the page to store more bytes.
    iap_state.page_buffer_ix = 0;

    return IAP_OK;
}

static iap_err_t iap_finish(int commit)
{
    // The module needs to be initialized for this method to work.
    if (!(iap_state.module_state_flags & IAP_STATE_INITIALIZED)) {
        ESP_LOGE(TAG, "iap_finish: the module hasn't been initialized!");
        return IAP_ERR_NOT_INITIALIZED;
    }

    // The session needs to be open for this method to work.
    if (!(iap_state.module_state_flags & IAP_STATE_SESSION_OPEN)) {
        ESP_LOGE(TAG, "iap_finish: programming session not open!");
        return IAP_ERR_NO_SESSION;
    }

    //free(iap_state.page_buffer);
#if 0
    iap_state.page_buffer = NULL;
    iap_state.page_buffer_ix = 0;
    iap_state.cur_flash_address = 0;
#endif
    // TODO
    // There's currently no way to abort an on-going OTA update.
    // http://www.esp32.com/viewtopic.php?f=14&t=1093

    esp_err_t result = esp_ota_end(iap_state.ota_handle);

    if (commit) {
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "iap_finish: esp_ota_end failed (%d)!", result);
            return IAP_FAIL;
        }

        result = esp_ota_set_boot_partition(iap_state.partition_to_program);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "iap_finish: esp_ota_set_boot_partition failed (%d)!", result);
            return IAP_FAIL;
        }
    }

    iap_state.ota_handle = 0;
    iap_state.partition_to_program = NULL;
    iap_state.module_state_flags = iap_state.module_state_flags & ~IAP_STATE_SESSION_OPEN;

    return IAP_OK;
}

static const esp_partition_t *iap_find_next_boot_partition()
{
    // Factory -> OTA_0
    // OTA_0   -> OTA_1
    // OTA_1   -> OTA_0

    const esp_partition_t *currentBootPartition = esp_ota_get_boot_partition();
    const esp_partition_t *nextBootPartition = NULL;

    if (!strcmp("factory", currentBootPartition->label)) {
        nextBootPartition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, "ota_0");
    }

    if (!strcmp("ota_0", currentBootPartition->label)) {
        nextBootPartition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, "ota_1");
    }

    if (!strcmp("ota_1", currentBootPartition->label)) {
        nextBootPartition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, "ota_0");
    }

    return nextBootPartition;
}
