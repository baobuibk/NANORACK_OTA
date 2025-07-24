/**
  ******************************************************************************
  * @file    FatFs/FatFs_Shared_Device/Common/Inc/mmc_diskio.h
  * @author  MCD Application Team
  * @brief   Header for mmc_diskio.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef __MMC_DISKIO_H
#define __MMC_DISKIO_H

#include "emmc_bsp_driver.h"
#include "ff_gen_drv.h"

/* Configuration options */
// #define DUAL_MMC         // Use 2 MMC at the same time
#define ONLY_MMC1         // Use 1: MMC1
// #define ONLY_MMC2       // Use 1: MMC2

#if !defined(DUAL_MMC) && !defined(ONLY_MMC1) && !defined(ONLY_MMC2)
#warning "MMC haven't been selected yet! Define DUAL_MMC, ONLY_MMC1, or ONLY_MMC2."
#endif

/* Function prototypes for MMC drivers */
#if defined(DUAL_MMC) || defined(ONLY_MMC1)
const Diskio_drvTypeDef* MMC1_GetDriver(void);
#endif
#if defined(DUAL_MMC) || defined(ONLY_MMC2)
const Diskio_drvTypeDef* MMC2_GetDriver(void);
#endif

#endif /* __MMC_DISKIO_H */
