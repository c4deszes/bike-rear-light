#ifndef APP_CONFIG_PRIV_H_
#define APP_CONFIG_PRIV_H_

#include "uds_gen.h"

#define CONFIG_ATTR __attribute__((section(".config_data")))
#define CONFIG_FLASH_ADDRESS 0x3F000u
#define CONFIG_FLASH_SIZE (64u * 4u)

typedef struct {
    UDS_Properties_RearLight_t properties;
    uint8_t padding[CONFIG_FLASH_SIZE - sizeof(UDS_Properties_RearLight_t) - sizeof(uint32_t)];
    uint32_t crc32;
} config_memlayout_t;

#endif /* APP_CONFIG_PRIV_H_ */