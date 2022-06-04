// Software SPI emulation
//
// Copyright (C) 2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "autoconf.h"
#include "board/gpio.h" // gpio_out_setup
#include "ctr.h" // DECL_CTR

DECL_CTR("DECL_SWSPI_MISO_PIN " __stringify(CONFIG_SWSPI_MISO_PIN));
DECL_CTR("DECL_SWSPI_MOSI_PIN " __stringify(CONFIG_SWSPI_MOSI_PIN));
DECL_CTR("DECL_SWSPI_SCLK_PIN " __stringify(CONFIG_SWSPI_SCLK_PIN));
extern uint32_t swspi_miso_gpio; // Generated by buildcommands.py
extern uint32_t swspi_mosi_gpio; // Generated by buildcommands.py
extern uint32_t swspi_sclk_gpio; // Generated by buildcommands.py

static struct spi_software {
    struct gpio_in miso;
    struct gpio_out mosi, sclk;
    uint8_t mode;
} ss;

void
spi_software_setup(uint8_t mode)
{
    ss.miso = gpio_in_setup(swspi_miso_gpio, 1);
    ss.mosi = gpio_out_setup(swspi_mosi_gpio, 0);
    ss.sclk = gpio_out_setup(swspi_sclk_gpio, 0);
    ss.mode = mode;
}

void
spi_software_prepare(void)
{
    gpio_out_write(ss.sclk, ss.mode & 0x02);
}

void
spi_software_transfer(uint8_t receive_data, uint16_t len, uint8_t *data)
{
    while (len--) {
        uint8_t outbuf = *data;
        uint8_t inbuf = 0;
        for (uint_fast8_t i = 0; i < 8; i++) {
            if (ss.mode & 0x01) {
                // MODE 1 & 3
                gpio_out_toggle(ss.sclk);
                gpio_out_write(ss.mosi, outbuf & 0x80);
                outbuf <<= 1;
                gpio_out_toggle(ss.sclk);
                inbuf <<= 1;
                inbuf |= gpio_in_read(ss.miso);
            } else {
                // MODE 0 & 2
                gpio_out_write(ss.mosi, outbuf & 0x80);
                outbuf <<= 1;
                gpio_out_toggle(ss.sclk);
                inbuf <<= 1;
                inbuf |= gpio_in_read(ss.miso);
                gpio_out_toggle(ss.sclk);
            }
        }

        if (receive_data)
            *data = inbuf;
        data++;
    }
}
