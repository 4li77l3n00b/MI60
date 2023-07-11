/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @version        : v1.0_Cube
  * @brief          : USB Device Custom HID interface file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_custom_hid_if.h"

/* USER CODE BEGIN INCLUDE */
#include "common_inc.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
void HID_RxCpltCallback(uint8_t* _data);
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @addtogroup USBD_CUSTOM_HID
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions USBD_CUSTOM_HID_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Defines USBD_CUSTOM_HID_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Macros USBD_CUSTOM_HID_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */
#define LSB(_x) ((_x) & 0xFF)
#define MSB(_x) ((_x) >> 8)

#define RAWHID_USAGE_PAGE	0xFFC0
#define RAWHID_USAGE		0x0C00
#define RAWHID_TX_SIZE 64
#define RAWHID_RX_SIZE 64
/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
  * @brief Private variables.
  * @{
  */

/** Usb custom HID report descriptor. */
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_HS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
  /* USER CODE BEGIN 1 */
        0x05, 0x01,         //   Usage Page (Generic Desktop),
        0x09, 0x06,         //   Usage (Keyboard),
        0xA1, 0x01,         //   Collection (Application),
        0x85, 0x01,                    //     REPORT_ID (1)
        // bitmap of modifiers
        0x75, 0x01,         //   Report Size (1),
        0x95, 0x08,         //   Report Count (8),
        0x05, 0x07,       //   Usage Page (Key Codes),
        0x19, 0xE0,       //   Usage Minimum (224),
        0x29, 0xE7,       //   Usage Maximum (231),
        0x15, 0x00,       //   Logical Minimum (0),
        0x25, 0x01,       //   Logical Maximum (1),
        0x81, 0x02,       //   Input (Data, Variable, Absolute), ;Modifier byte
        // bitmap of keys
        0x95, 0x78,       //   Report Count (120),
        0x75, 0x01,       //   Report Size (1),
        0x15, 0x00,       //   Logical Minimum (0),
        0x25, 0x01,       //   Logical Maximum(1),
        0x05, 0x07,       //   Usage Page (Key Codes),
        0x19, 0x00,       //   Usage Minimum (0),
        0x29, 0x77,       //   Usage Maximum (),
        0x81, 0x02,       //   Input (Data, Variable, Absolute),
#if 1
                // LED output report
        0x95, 0x05,       //   Report Count (5),
        0x75, 0x01,       //   Report Size (1),
        0x05, 0x08,       //   Usage Page (LEDs),
        0x19, 0x01,       //   Usage Minimum (1),
        0x29, 0x05,       //   Usage Maximum (5),
        0x91, 0x02,       //   Output (Data, Variable, Absolute),
        0x95, 0x01,       //   Report Count (1),
        0x75, 0x03,       //   Report Size (3),
        0x91, 0x03,       //   Output (Constant),
#endif
        0xC0  ,                 //   End Collection

#if 1
                //	RAW HID
        0x06, LSB(RAWHID_USAGE_PAGE), MSB(RAWHID_USAGE_PAGE),	// 30
        0x0A, LSB(RAWHID_USAGE), MSB(RAWHID_USAGE),

        0xA1, 0x01,				// Collection 0x01
        0x85, 0x02,             // REPORT_ID (3)
        0x75, 0x08,				// report size = 8 bits
        0x15, 0x00,				// logical minimum = 0
        0x26, 0xFF, 0x00,		// logical maximum = 255

        0x95, RAWHID_TX_SIZE,				// report count TX
        0x09, 0x01,				// usage
        0x81, 0x02,				// Input (array)

        0x95, RAWHID_RX_SIZE,				// report count RX
        0x09, 0x02,				// usage
        0x91, 0x02,				// Output (array)
#endif
  /* USER CODE END 1 */
   0xC0    /*     END_COLLECTION             */
};
/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceHS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CUSTOM_HID_Init_HS(void);
static int8_t CUSTOM_HID_DeInit_HS(void);
static int8_t CUSTOM_HID_OutEvent_HS(uint8_t event_idx, uint8_t state);

/**
  * @}
  */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_HS =
{
  CUSTOM_HID_ReportDesc_HS,
  CUSTOM_HID_Init_HS,
  CUSTOM_HID_DeInit_HS,
  CUSTOM_HID_OutEvent_HS
};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
  * @brief Private functions.
  * @{
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_Init_HS(void)
{
  /* USER CODE BEGIN 8 */
  return (USBD_OK);
  /* USER CODE END 8 */
}

/**
  * @brief  DeInitializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_DeInit_HS(void)
{
  /* USER CODE BEGIN 9 */
  return (USBD_OK);
  /* USER CODE END 9 */
}

/**
  * @brief  Manage the CUSTOM HID class events
  * @param  event_idx: Event index
  * @param  state: Event state
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_OutEvent_HS(uint8_t event_idx, uint8_t state)
{
  /* USER CODE BEGIN 10 */
  UNUSED(event_idx);
  UNUSED(state);
    USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)hUsbDeviceHS.pClassData;
    switch (hhid->Report_buf[0]) {
        case 1:
            CapsLock((bool)(hhid->Report_buf[1] & 0x02));
            ScrollLock((bool)(hhid->Report_buf[1] & 0x04));
        case 2:
            switch (hhid->Report_buf[1]) {
                case 0:
                    SyncAll();
                case 1:
                    StartCalibration(hhid->Report_buf[2]);
                case 2:
                    ChangeKeyArg(hhid->Report_buf + 2);
                case 3:
                    ChangeConfKeyMap(hhid->Report_buf + 2);
                case 4:
                    ChangeKeyMap(hhid->Report_buf + 2);
                case 5:
                    ChangeRGBMap(hhid->Report_buf + 2);
                case 6:
                    ChangeRGBFXArg(hhid->Report_buf + 2);
            }
    }
    /* Start next USB packet transfer once data processing is completed */
  if (USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceHS) != (uint8_t)USBD_OK)
  {
    return -1;
  }

  return (USBD_OK);
  /* USER CODE END 10 */
}

/* USER CODE BEGIN 11 */
/**
  * @brief  Send the report to the Host
  * @param  report: The report to be sent
  * @param  len: The report length
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*
static int8_t USBD_CUSTOM_HID_SendReport_HS(uint8_t *report, uint16_t len)
{
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS, report, len);
}
*/
/* USER CODE END 11 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
