UTIMER counter DMA trigger
**************************

Compare channel 0 drives ``UT0_T0`` high at match. The event router
connects that edge to a PL330 request. The application arms DMA, then
calls ``counter_set_channel_alarm()`` with a NULL callback.
``counter_cancel_channel_alarm()`` pulls the request low and re-arms
the next rising edge.

Requires ``CONFIG_COUNTER_ALIF_UTIMER_DMA`` and
``CONFIG_COUNTER_ALIF_UTIMER_DMA_REARM_FORCE_LOW``.

The ``utimer-dma`` snippet applies the overlay. E7 and E8 use
``boards/utimer_dma0_e7_e8.overlay``. B1 and E1C use
``boards/utimer_dma2_b1_e1c.overlay``.

E8 HE::

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S utimer-dma \
     alif/samples/drivers/utimer_counter/counter_dma

The same command works for the other boards. Change only ``-b``:

* E8 HP: ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
* E7 HE: ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
* E7 HP: ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
* B1 HE: ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he``
* E1C HE: ``alif_e1c_dk/ae1c1f4051920hh/rtss_he``

E7/E8 HE on DMA2 also applies ``boards/utimer_dma2_e7_e8_he.overlay``.
For E7 HE, change ``-b`` to ``alif_e7_dk/ae722f80f55d5xx/rtss_he``::

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     -S utimer-dma -S utimer-dma2 \
     alif/samples/drivers/utimer_counter/counter_dma

Sample output
*************

.. code-block:: console

   UTIMER counter DMA trig

   Set alarm in 2 sec (800000000 ticks)
   !!! Alarm !!!
   Now: 2
   UTIMER DMA trig: PASS
   Set alarm in 4 sec (1600000000 ticks)
   !!! Alarm !!!
   Now: 6
   UTIMER DMA trig: PASS
   Set alarm in 8 sec (3200000000 ticks)
   !!! Alarm !!!
   Now: 14
   UTIMER DMA trig: PASS
   Set alarm in 5 sec (2105032704 ticks)
   !!! Alarm !!!
   Now: 19
   UTIMER DMA trig: PASS
   Set alarm in 10 sec (4210065408 ticks)
   !!! Alarm !!!
   Now: 29
   UTIMER DMA trig: PASS
   Set alarm in 10 sec (4125163520 ticks)
   !!! Alarm !!!
   Now: 39
   UTIMER DMA trig: PASS
   Set alarm in 9 sec (3955359744 ticks)
   !!! Alarm !!!
   Now: 48
   UTIMER DMA trig: PASS
   Set alarm in 9 sec (3615752192 ticks)
   !!! Alarm !!!
   Now: 57
   UTIMER DMA trig: PASS
   Set alarm in 7 sec (2936537088 ticks)
   !!! Alarm !!!
   Now: 64
   UTIMER DMA trig: PASS
   Set alarm in 3 sec (1578106880 ticks)
   !!! Alarm !!!
   Now: 67
   UTIMER DMA trig: PASS
   Set alarm in 7 sec (3156213760 ticks)
   !!! Alarm !!!
   Now: 74
   UTIMER DMA trig: PASS
   Set alarm in 5 sec (2017460224 ticks)
   !!! Alarm !!!
   Now: 79
   UTIMER DMA trig: PASS
   Set alarm in 10 sec (4034920448 ticks)
   !!! Alarm !!!
   Now: 89
   UTIMER DMA trig: PASS
   Set alarm in 9 sec (3774873600 ticks)
   !!! Alarm !!!
   Now: 98
   UTIMER DMA trig: PASS
   Set alarm in 8 sec (3254779904 ticks)
   !!! Alarm !!!
   Now: 106
   UTIMER DMA trig: PASS
   Set alarm in 5 sec (2214592512 ticks)
   !!! Alarm !!!
   Now: 111
   UTIMER DMA trig: PASS
   Set alarm in 0 sec (134217728 ticks)
   !!! Alarm !!!
   Now: 111
   UTIMER DMA trig: PASS
   Set alarm in 0 sec (268435456 ticks)
   !!! Alarm !!!
   Now: 111
   UTIMER DMA trig: PASS
   Set alarm in 1 sec (536870912 ticks)
   !!! Alarm !!!
   Now: 112
   UTIMER DMA trig: PASS
   Set alarm in 2 sec (1073741824 ticks)
   !!! Alarm !!!
   Now: 114
   UTIMER DMA trig: PASS
   Set alarm in 5 sec (2147483648 ticks)
   !!! Alarm !!!
   Now: 119
   UTIMER DMA trig: PASS
   Set alarm in 2 sec (800000000 ticks)
   !!! Alarm !!!
   Now: 121
   UTIMER DMA trig: PASS
   Set alarm in 4 sec (1600000000 ticks)
   !!! Alarm !!!
   Now: 125
   UTIMER DMA trig: PASS
   Set alarm in 8 sec (3200000000 ticks)
   !!! Alarm !!!
   Now: 133
   UTIMER DMA trig: PASS
   Set alarm in 5 sec (2105032704 ticks)
   !!! Alarm !!!
   Now: 138
   UTIMER DMA trig: PASS
   Set alarm in 10 sec (4210065408 ticks)
   !!! Alarm !!!
   Now: 148
   UTIMER DMA trig: PASS
   Set alarm in 10 sec (4125163520 ticks)
   !!! Alarm !!!
   Now: 158
   UTIMER DMA trig: PASS
   Set alarm in 9 sec (3955359744 ticks)
   !!! Alarm !!!
   Now: 167
   UTIMER DMA trig: PASS
   Set alarm in 9 sec (3615752192 ticks)
