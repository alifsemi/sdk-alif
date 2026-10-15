#ifndef EXERCISER_APP_H
#define EXERCISER_APP_H

#include <zephyr/devicetree.h>

#define BLINKY_NODE_OKAY  DT_NODE_HAS_STATUS_OKAY(DT_ALIAS(led0))
#define OSPI_NODE_OKAY    DT_NODE_HAS_STATUS_OKAY(DT_ALIAS(spi_flash0))
#define SPI_NODE_OKAY     DT_NODE_HAS_STATUS_OKAY(DT_ALIAS(master_spi))
#define SD_NODE_OKAY      DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(sdhc))
#define USB_NODE_OKAY     DT_HAS_COMPAT_STATUS_OKAY(zephyr_cdc_acm_uart)
#define BMI323_NODE_OKAY  DT_HAS_COMPAT_STATUS_OKAY(bosch_bmi323)
#define CODEC_NODE_OKAY   DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(audio_codec))
#define VIDEO_NODE_OKAY   DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(isp))
#define ETH_NODE_OKAY     DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(ethernet))

#if BLINKY_NODE_OKAY
void blinky_thread(void);
#endif
#if OSPI_NODE_OKAY
void ospi_thread(void);
#endif
#if SPI_NODE_OKAY
void spi_thread(void);
#endif
#if SD_NODE_OKAY
void sd_thread(void);
#endif
#if USB_NODE_OKAY
void usb_thread(void);
#endif
#if CODEC_NODE_OKAY
void codec_thread(void);
#endif
#if BMI323_NODE_OKAY
void bmi323_thread(void);
#endif
#if VIDEO_NODE_OKAY
void video_thread(void);
#endif
#if ETH_NODE_OKAY
void eth_thread(void);
#endif
void start_all_exerciser_threads(void);
#endif /* EXERCISER_APP_H */
