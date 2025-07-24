/*
 * filesystem.c
 *
 *  Created on: Mar 2, 2025
 *      Author: CAO HIEU
 */
#include "ff.h"
#include "mmc_diskio.h"
#include "filesystem.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

#include "board.h"
#include "utils.h"
#include "SysLog/syslog.h"

//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"

SemaphoreHandle_t fsMutex;
SemaphoreHandle_t fsSlotMutex;
/*************************************************
 *                 FATFS Variable                *
 *************************************************/
__attribute__((section(".dma_buffer"))) FATFS MMC1FatFs;  /* File system object for SD card logical drive */
__attribute__((section(".dma_buffer"))) FIL MyFile1;     /* File object */
char MMC1Path[4]; /* SD card logical drive path */

/*************************************************
 *               Gate-keeper SDMMC               *
 *************************************************/
#define MAX_DATA_SIZE (8 * 1024) // 8KB%slot
#define NUM_SLOTS 10

typedef struct {
    char filename[32];           // File name (32 bytes)
    uint8_t* data;               // Real-data(8KB)
    uint32_t size;               // Real-size (4 bytes)
    TaskHandle_t requester;      // Handle task request (4 bytes)
    Std_ReturnType result;       // Return result (4 bytes)
} FS_WriteRequest_t;             // 8240 bytes per slot

QueueHandle_t fsWriteQueue;
StaticQueue_t fsWriteQueueStruct;
//__attribute__((section(".fs_buffer"))) static uint8_t fsQueueBuffer[90640]; // 10 slot * 8240 bytes + 1 backup
__attribute__((section(".fs_buffer"))) static uint8_t fsQueueBuffer[NUM_SLOTS * 64]; 			// 10 slot * 64 bytes = 640 bytes
__attribute__((section(".fs_buffer"))) static uint8_t fsDataBuffer[NUM_SLOTS][MAX_DATA_SIZE];   // 10 slot * 8KB = 80KB
static uint8_t slotInUse[NUM_SLOTS] = {0};

static int FindFreeSlot(void) {
    for (int i = 0; i < NUM_SLOTS; i++) {
        if (slotInUse[i] == 0) return i;
    }
    return -1;
}

/*************************************************
 *              Physical Status SDMMC            *
 *************************************************/
SDFS_StateTypedef SDFS_State = SDFS_READY;

void SD_Lockin(void)
{
    GPIO_SetHigh(SD_InOut_Port, SD_InOut);
    GPIO_SetHigh(SD_Detect_Port, SD_Detect);
}
void SD_Release(void)
{
    GPIO_SetLow(SD_InOut_Port, SD_InOut);
    GPIO_SetLow(SD_Detect_Port, SD_Detect);
}

/*************************************************
 *                 Queue implement               *
 *************************************************/
void FS_Init(void) {
    fsMutex = xSemaphoreCreateMutex();
    fsSlotMutex = xSemaphoreCreateMutex();
    if (fsMutex == NULL || fsSlotMutex == NULL) {
        SYSLOG_ERROR_POLL("Failed to create mutexes");
        return;
    }

    fsWriteQueue = xQueueCreateStatic(NUM_SLOTS, sizeof(FS_WriteRequest_t), fsQueueBuffer, &fsWriteQueueStruct);
    if (fsWriteQueue == NULL) {
        SYSLOG_ERROR_POLL("Failed to create FS write queue");
        return;
    }
}

void FS_Gatekeeper_Task(void *pvParameters) {
    FS_WriteRequest_t request;

    for (;;) {
        if (xQueueReceive(fsWriteQueue, &request, portMAX_DELAY) == pdTRUE) {
            Std_ReturnType result = E_ERROR;
            FIL file;
            FRESULT res;
            UINT byteswritten;

            if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                res = f_open(&file, request.filename, FA_OPEN_APPEND | FA_WRITE);
                if (res == FR_OK) {
                    res = f_write(&file, request.data, request.size, &byteswritten);
                    f_close(&file);
                    if (res == FR_OK && byteswritten == request.size) {
                        result = E_OK;
                    }
                }
                xSemaphoreGive(fsMutex);
            }
            xTaskNotify(request.requester, result, eSetValueWithOverwrite);
        }
    }
}

Std_ReturnType FS_Request_Write(const char* filename, uint8_t* buffer, uint32_t size) {
    if (size > MAX_DATA_SIZE) {
        return E_ERROR;
    }

    if (xSemaphoreTake(fsSlotMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return E_ERROR;
    }

    int slot = FindFreeSlot();
    if (slot == -1) {
        xSemaphoreGive(fsSlotMutex);
        return E_ERROR;
    }
    slotInUse[slot] = 1;
    xSemaphoreGive(fsSlotMutex);

    FS_WriteRequest_t req = {
        .data = fsDataBuffer[slot],
        .size = size,
        .requester = xTaskGetCurrentTaskHandle()
    };
    strncpy(req.filename, filename, sizeof(req.filename) - 1);
    memcpy(fsDataBuffer[slot], buffer, size);

    if (xQueueSend(fsWriteQueue, &req, pdMS_TO_TICKS(1000)) != pdPASS) {
        if (xSemaphoreTake(fsSlotMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            slotInUse[slot] = 0;
            xSemaphoreGive(fsSlotMutex);
        }
        return E_ERROR;
    }

    uint32_t result;
    xTaskNotifyWait(0, 0, &result, pdMS_TO_TICKS(5000));

    if (xSemaphoreTake(fsSlotMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        slotInUse[slot] = 0;
        xSemaphoreGive(fsSlotMutex);
    }

    return (Std_ReturnType)result;
}

/*************************************************
 *                   Low layer API               *
 *************************************************/
Std_ReturnType Link_SDFS_Driver(void) {
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
    const Diskio_drvTypeDef *mmc1_driver = MMC1_GetDriver();
    if (FATFS_LinkDriver(mmc1_driver, MMC1Path) == 0) {
        int ret1 = f_mount(&MMC1FatFs, (TCHAR const*)MMC1Path, 1);
        if (ret1 != FR_OK) {
            return E_ERROR;
        }
    }
#endif
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
    const Diskio_drvTypeDef *mmc2_driver = MMC2_GetDriver();
    if (FATFS_LinkDriver(mmc2_driver, MMC2Path) == 0) {
        int ret2 = f_mount(&MMC2FatFs, (TCHAR const*)MMC2Path, 1);
        if (ret2 != FR_OK) {
            return E_ERROR;
        }
    }
#endif
    return E_OK;
}

int Cat_SDFS(EmbeddedCli *cli, const char *filename) {
    FIL file;
    FRESULT res;
    ALIGN_32BYTES(uint8_t rtext[96]);
    UINT bytesread;
    char buffer[128];

    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        embeddedCliPrint(cli, "Failed to acquire FS mutex");
        return -1;
    }

    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        snprintf(buffer, sizeof(buffer), "Failed to open file %s: %d", filename, res);
        embeddedCliPrint(cli, buffer);
        xSemaphoreGive(fsMutex);
        return -1;
    }

    embeddedCliPrint(cli, "");
    do {
        memset(rtext, 0, sizeof(rtext));
        res = f_read(&file, rtext, sizeof(rtext) - 1, &bytesread);
        if (res != FR_OK) {
            snprintf(buffer, sizeof(buffer), "Failed to read file %s: %d", filename, res);
            embeddedCliPrint(cli, buffer);
            f_close(&file);
            xSemaphoreGive(fsMutex);
            return -1;
        }
        rtext[bytesread] = '\0';
        embeddedCliPrint(cli, (char *)rtext);
    } while (bytesread > 0);

    embeddedCliPrint(cli, "");
    f_close(&file);
    xSemaphoreGive(fsMutex);
    return 0;
}

int Vim_SDFS(EmbeddedCli *cli, const char *filename, const char *content) {
    FIL file;
    FRESULT res;
    char buffer[128];
    UINT byteswritten;

    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        embeddedCliPrint(cli, "Failed to acquire FS mutex");
        return -1;
    }

    res = f_open(&file, filename, FA_OPEN_APPEND | FA_WRITE);
    if (res != FR_OK) {
        snprintf(buffer, sizeof(buffer), "Failed to open or create file %s: %d", filename, res);
        embeddedCliPrint(cli, buffer);
        xSemaphoreGive(fsMutex);
        return -1;
    }

    if (content && strlen(content) > 0) {
        res = f_write(&file, content, strlen(content), &byteswritten);
        if (res != FR_OK || byteswritten != strlen(content)) {
            snprintf(buffer, sizeof(buffer), "Failed to write to file %s: %d", filename, res);
            embeddedCliPrint(cli, buffer);
            f_close(&file);
            xSemaphoreGive(fsMutex);
            return -1;
        }
    }

    f_close(&file);
    xSemaphoreGive(fsMutex);
    snprintf(buffer, sizeof(buffer), "Successfully wrote to %s", filename);
    embeddedCliPrint(cli, buffer);
    return 0;
}

void FS_ListFiles_path(EmbeddedCli *cli) {
    FRESULT res;
    DIR dir;
    FILINFO fno;
    char *path = MMC1Path;
    char buffer[384];

    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        embeddedCliPrint(cli, "Failed to acquire FS mutex");
        return;
    }

    snprintf(buffer, sizeof(buffer), "Listing files in %s...", path);
    embeddedCliPrint(cli, buffer);

    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        while (1) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            if (fno.fattrib & AM_DIR) {
                snprintf(buffer, sizeof(buffer), "  [DIR]  %s", fno.fname);
                embeddedCliPrint(cli, buffer);
            } else {
                snprintf(buffer, sizeof(buffer), "  [FILE] %s  %lu bytes",
                         fno.fname, (unsigned long)fno.fsize);
                embeddedCliPrint(cli, buffer);
            }
        }
        f_closedir(&dir);
    } else {
        snprintf(buffer, sizeof(buffer), "Failed to open directory %s: %d", path, res);
        embeddedCliPrint(cli, buffer);
    }
    xSemaphoreGive(fsMutex);
}
