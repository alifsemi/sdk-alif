.. zephyr:code-sample:: i2s-duplex
   :name: I2S duplex
   :relevant-api: i2s_interface

   Demonstrate simultaneous I2S RX and TX traffic.

Overview
********

This sample demonstrates a basic full-duplex I2S application. It keeps RX and
TX active at the same time, generates a transmit pattern, and separately
monitors received audio blocks.

Requirements
************

The I2S device to be used by the sample is specified by defining a devicetree
node label named ``i2s_rxtx`` or ``i2s2``/``i2s3`` for a shared controller, or
separate node labels ``i2s_rx`` and ``i2s_tx`` if distinct devices are used.

For meaningful RX data, connect the TX and RX signals externally or route the
I2S stream through a direct loopback setup.

Building and Running
********************

Example command to build:

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex -p
   OR
   west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he ../alif/samples/drivers/i2s_duplex -p -- -DSNIPPET=i2s-duplex

Sample Output
=============
*** Booting Zephyr OS build 5043b8e9d93b ***
I2S full duplex sample
Sample setup: slab=0x2000a1b0 block_size=1764 bytes, block_count=12, samples_per_block=882, sample_frequency=44100 Hz
Streams started
RX block 0
TX: 0 1 2 3
RX: 0 1 2 3
Verification: PASS
RX block 1
TX: 882 883 884 885
RX: 882 883 884 885
Verification: PASS
RX block 2
TX: 1764 1765 1766 1767
RX: 1764 1765 1766 1767
Verification: PASS
RX block 3
TX: 2646 2647 2648 2649
RX: 2646 2647 2648 2649
Verification: PASS
RX block 4
TX: 3528 3529 3530 3531
RX: 3528 3529 3530 3531
Verification: PASS
RX block 5
TX: 4410 4411 4412 4413
RX: 4410 4411 4412 4413
Verification: PASS
RX block 6
TX: 5292 5293 5294 5295
RX: 5292 5293 5294 5295
Verification: PASS
RX block 7
TX: 6174 6175 6176 6177
RX: 6174 6175 6176 6177
Verification: PASS
RX block 8
TX: 7056 7057 7058 7059
RX: 7056 7057 7058 7059
Verification: PASS
RX block 9
TX: 7938 7939 7940 7941
RX: 7938 7939 7940 7941
Verification: PASS
RX block 10
TX: 8820 8821 8822 8823
RX: 8820 8821 8822 8823
Verification: PASS
RX block 11
TX: 9702 9703 9704 9705
RX: 9702 9703 9704 9705
Verification: PASS
RX block 12
TX: 10584 10585 10586 10587
RX: 10584 10585 10586 10587
Verification: PASS
RX block 13
TX: 11466 11467 11468 11469
RX: 11466 11467 11468 11469
Verification: PASS
RX block 14
TX: 12348 12349 12350 12351
RX: 12348 12349 12350 12351
Verification: PASS
RX block 15
TX: 13230 13231 13232 13233
RX: 13230 13231 13232 13233
Verification: PASS
RX block 16
TX: 14112 14113 14114 14115
RX: 14112 14113 14114 14115
Verification: PASS
RX block 17
TX: 14994 14995 14996 14997
RX: 14994 14995 14996 14997
Verification: PASS
RX block 18
TX: 15876 15877 15878 15879
RX: 15876 15877 15878 15879
Verification: PASS
RX block 19
TX: 16758 16759 16760 16761
RX: 16758 16759 16760 16761
Verification: PASS
RX block 20
TX: 17640 17641 17642 17643
RX: 17640 17641 17642 17643
Verification: PASS
RX block 21
TX: 18522 18523 18524 18525
RX: 18522 18523 18524 18525
Verification: PASS
RX block 22
TX: 19404 19405 19406 19407
RX: 19404 19405 19406 19407
Verification: PASS
RX block 23
TX: 20286 20287 20288 20289
RX: 20286 20287 20288 20289
Verification: PASS
RX block 24
TX: 21168 21169 21170 21171
RX: 21168 21169 21170 21171
Verification: PASS
