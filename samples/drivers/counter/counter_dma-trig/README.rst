Counter DMA trigger (LPTIMER)
*****************************

LPTIMER timeout as "LPTIMERn_DMA_REQ". Overlay selects the Event Router
mux. CPU IRQ is off ("dma-trig"). First period is a wrong-request
negative test, then the same looping alarm as the counter alarm
sample (2 s, 4 s, 8 s, ...). Main arms and re-arms each period;
"dma_cb" only signals completion. Each DMA alarm prints
"LPTIMER DMA trig: PASS".

UTIMER is not in this sample yet.

From "~/ZAS_RELEASE/2.3.0-rc4"::

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     alif/samples/drivers/counter/counter_dma-trig
   west flash

HP (E8)::

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     alif/samples/drivers/counter/counter_dma-trig
   west flash

CMake picks one overlay from the board family:

- E7 / E8 (HE and HP): "boards/lptimer_dma0_e7_e8.overlay"
  (DMA0 EVTRTR0 group 1 req 0)
- B1 / E1C (HE): "boards/lptimer_dma2_b1_e1c.overlay"
  (DMA2 EVTRTR2 group 1 req 0)

Console is **115200 8N1**. HE prints on **UART2**; HP prints on **UART4**.
Minicom on the HE port shows nothing for an HP image (not even
"Booting Zephyr OS"). Use the other USB serial for HP.

E7 / E8 HE DMA2 (group 0)::

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     alif/samples/drivers/counter/counter_dma-trig -- \
     -DEXTRA_DTC_OVERLAY_FILE=boards/lptimer_dma2_e7_e8_he.overlay

Pass: "LPTIMER DMA trig: PASS".
