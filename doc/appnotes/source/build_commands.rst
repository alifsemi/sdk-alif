.. _build-commands:

Build Commands
==============

Each application note shows an example build command.
This page lists that command for every supported target.
Copy one code block into the terminal.
Targets come from the sample ``boards`` directory or ``sample.yaml``
and from the board list in the ZAS User Guide.

.. _build-commands-adc:

ADC12/24
--------

Sample path: ``../alif/samples/drivers/adc`` Snippet: ``-S alif-adc``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-adc ../alif/samples/drivers/adc

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-adc ../alif/samples/drivers/adc

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-adc ../alif/samples/drivers/adc

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-adc ../alif/samples/drivers/adc

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-adc ../alif/samples/drivers/adc

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-adc ../alif/samples/drivers/adc

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-adc ../alif/samples/drivers/adc

.. _build-commands-apss-a32:

Running Zephyr on the Cortex-A32 based APSS
-------------------------------------------

Sample path: ``samples/hello_world``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for APSS:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/apss samples/hello_world

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for APSS:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/apss samples/hello_world

.. _build-commands-ble:

BLE
---

Sample path: ``../alif/samples/bluetooth/le_periph_hr``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/bluetooth/le_periph_hr

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/bluetooth/le_periph_hr

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he ../alif/samples/bluetooth/le_periph_hr

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/bluetooth/le_periph_hr

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/bluetooth/le_periph_hr

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he ../alif/samples/bluetooth/le_periph_hr

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he ../alif/samples/bluetooth/le_periph_hr

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he ../alif/samples/bluetooth/le_periph_hr

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he ../alif/samples/bluetooth/le_periph_hr

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he ../alif/samples/bluetooth/le_periph_hr

.. _build-commands-can:

CAN (Controller Area Network)
-----------------------------

Sample path: ``samples/drivers/can/counter`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-dk samples/drivers/can/counter

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-dk samples/drivers/can/counter

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-dk samples/drivers/can/counter

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-dk samples/drivers/can/counter

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-dk samples/drivers/can/counter

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-dk samples/drivers/can/counter

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-dk samples/drivers/can/counter

.. _build-commands-ch201-tof:

CH201 Time-of-Flight (ToF) Sensor
---------------------------------

Sample path: ``../alif/samples/sensor/ch201`` Snippet: ``-S alif-ak``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-ak ../alif/samples/sensor/ch201

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-ak ../alif/samples/sensor/ch201

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-ak ../alif/samples/sensor/ch201

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-ak ../alif/samples/sensor/ch201

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-ak ../alif/samples/sensor/ch201

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-ak ../alif/samples/sensor/ch201

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-ak ../alif/samples/sensor/ch201

.. _build-commands-cmp:

CMP
---

Sample path: ``../alif/samples/drivers/cmp`` Snippet: ``-S alif-cmp``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-cmp ../alif/samples/drivers/cmp

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-cmp ../alif/samples/drivers/cmp

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-cmp ../alif/samples/drivers/cmp

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-cmp ../alif/samples/drivers/cmp

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-cmp ../alif/samples/drivers/cmp

.. _build-commands-cpu-freq-pstate:

RTSS CPU Frequency P-states
---------------------------

Sample path: ``../alif/samples/drivers/pm/pstate_set`` Snippet: ``-S pstate-he`` ``-S pstate-hp``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S pstate-he -S pstate-hp ../alif/samples/drivers/pm/pstate_set

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S pstate-he -S pstate-hp ../alif/samples/drivers/pm/pstate_set

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S pstate-he -S pstate-hp ../alif/samples/drivers/pm/pstate_set

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S pstate-he -S pstate-hp ../alif/samples/drivers/pm/pstate_set

.. _build-commands-crc:

CRC
---

Sample path: ``../alif/samples/drivers/crc``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/crc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/crc

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/crc

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/crc

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/crc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/crc

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/crc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/crc

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/crc

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/crc

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/crc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/crc

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he ../alif/samples/drivers/crc

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he ../alif/samples/drivers/crc

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he ../alif/samples/drivers/crc

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he ../alif/samples/drivers/crc

Sample path: ``../alif/samples/drivers/pm/alif_crc`` Snippet: ``-S crc-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p auto -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S crc-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_crc -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/alif_crc`` Snippet: ``-S crc-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p auto -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S crc-pm-mram ../alif/samples/drivers/pm/alif_crc

.. _build-commands-dac:

DAC
---

Sample path: ``../alif/samples/drivers/dac``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/dac

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/dac

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/dac

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/dac

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/dac

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/dac

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/dac

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/dac

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/dac

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/dac

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/dac

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/dac

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he ../alif/samples/drivers/dac

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he ../alif/samples/drivers/dac

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he ../alif/samples/drivers/dac

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he ../alif/samples/drivers/dac

.. _build-commands-dma:

DMA
---

Sample path: ``../alif/samples/drivers/spi_dw`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

.. _build-commands-entropy:

Entropy
-------

Sample path: ``tests/drivers/entropy/api``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he tests/drivers/entropy/api

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp tests/drivers/entropy/api

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he tests/drivers/entropy/api

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp tests/drivers/entropy/api

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he tests/drivers/entropy/api

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp tests/drivers/entropy/api

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he tests/drivers/entropy/api

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp tests/drivers/entropy/api

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he tests/drivers/entropy/api

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp tests/drivers/entropy/api

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he tests/drivers/entropy/api

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp tests/drivers/entropy/api

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he tests/drivers/entropy/api

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he tests/drivers/entropy/api

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he tests/drivers/entropy/api

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he tests/drivers/entropy/api

.. _build-commands-ethernet:

Ethernet
--------

Sample path: ``samples/net/dhcpv4_client`` Snippet: ``-S alif-dhcpv4-client``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-dhcpv4-client samples/net/dhcpv4_client

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-dhcpv4-client samples/net/dhcpv4_client

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-dhcpv4-client samples/net/dhcpv4_client

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-dhcpv4-client samples/net/dhcpv4_client

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-dhcpv4-client samples/net/dhcpv4_client

.. _build-commands-ethos:

Ethos NPU U-55/U85
------------------

Sample path: ``../alif/samples/modules/tflite-micro/tflm_ethosu`` Snippet: ``-S ethos-u55-enable`` Extra options: ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Sample path: ``../alif/samples/modules/tflite-micro/tflm_ethosu`` Snippet: ``-S ethos-u55-enable`` Extra options: ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S ethos-u55-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Sample path: ``../alif/samples/modules/tflite-micro/tflm_ethosu`` Snippet: ``-S ethos-u85-enable`` Extra options: ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S ethos-u85-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S ethos-u85-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S ethos-u85-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Sample path: ``../alif/samples/modules/tflite-micro/tflm_ethosu`` Snippet: ``-S ethos-u85-apss-enable`` Extra options: ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for APSS:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/apss -S ethos-u85-apss-enable ../alif/samples/modules/tflite-micro/tflm_ethosu -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Sample path: ``alif/samples/modules/executorch/kws_ethosu`` Snippet: ``-S ethos-u55-enable`` Extra options: ``-DET_PTE_FILE_PATH=./kws_u55_256.pte`` ``-DET_PTE_SECTION=.rodata.model`` ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Sample path: ``alif/samples/modules/executorch/kws_ethosu`` Snippet: ``-S ethos-u85-enable`` Extra options: ``-DET_PTE_FILE_PATH=./kws_u85_256.pte`` ``-DET_PTE_SECTION=.rodata.model`` ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S ethos-u85-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Sample path: ``alif/samples/modules/executorch/kws_ethosu`` Snippet: ``-S ethos-u55-enable`` Extra options: ``-DET_PTE_FILE_PATH=./kws_u55_128.pte`` ``-DET_PTE_SECTION=.rodata.model`` ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S ethos-u55-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Sample path: ``alif/samples/modules/executorch/kws_ethosu`` Snippet: ``-S ethos-u85-apss-enable`` Extra options: ``-DET_PTE_FILE_PATH=./kws_u85_256.pte`` ``-DET_PTE_SECTION=.rodata.model`` ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for APSS:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/apss -S ethos-u85-apss-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for APSS:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/apss -S ethos-u85-apss-enable alif/samples/modules/executorch/kws_ethosu -- -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

.. _build-commands-gpio:

General-Purpose Input/Output (GPIO)
-----------------------------------

Sample path: ``samples/basic/blinky``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/basic/blinky

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/basic/blinky

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/basic/blinky

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/basic/blinky

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he samples/basic/blinky

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp samples/basic/blinky

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/basic/blinky

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/basic/blinky

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he samples/basic/blinky

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/basic/blinky

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he samples/basic/blinky

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp samples/basic/blinky

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he samples/basic/blinky

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he samples/basic/blinky

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he samples/basic/blinky

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he samples/basic/blinky

Sample path: ``samples/basic/button`` Snippet: ``-S alif-button``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-button samples/basic/button

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-button samples/basic/button

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-button samples/basic/button

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-button samples/basic/button

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-button samples/basic/button

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-button samples/basic/button

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-button samples/basic/button

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-button samples/basic/button

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-button samples/basic/button

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-button samples/basic/button

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-button samples/basic/button

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-button samples/basic/button

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-button samples/basic/button

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-button samples/basic/button

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-button samples/basic/button

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-button samples/basic/button

.. _build-commands-helloworld-ospi:

Building Hello World for OSPI NOR Flash
---------------------------------------

Sample path: ``samples/hello_world`` Extra options: ``-DCONFIG_ARM_MPU=n`` ``-DCONFIG_FLASH_BASE_ADDRESS=0xC0200000``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he samples/hello_world -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Sample path: ``samples/drivers/uart/echo_bot`` Extra options: ``-DCONFIG_ARM_MPU=n`` ``-DCONFIG_FLASH_BASE_ADDRESS=0xC0000000`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/drivers/uart/echo_bot -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 -DCONFIG_FLASH_LOAD_OFFSET=0x0

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/drivers/uart/echo_bot -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 -DCONFIG_FLASH_LOAD_OFFSET=0x0

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp samples/drivers/uart/echo_bot -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 -DCONFIG_FLASH_LOAD_OFFSET=0x0

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/uart/echo_bot -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 -DCONFIG_FLASH_LOAD_OFFSET=0x0

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/drivers/uart/echo_bot -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 -DCONFIG_FLASH_LOAD_OFFSET=0x0

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp samples/drivers/uart/echo_bot -- -DCONFIG_ARM_MPU=n -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 -DCONFIG_FLASH_LOAD_OFFSET=0x0

.. _build-commands-hwsem:

HWSEM
-----

Sample path: ``../alif/samples/drivers/ipm/ipm_alif_hwsem``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_alif_hwsem

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_alif_hwsem

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_alif_hwsem

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_alif_hwsem

Sample path: ``../alif/samples/drivers/ipm/ipm_alif_hwsem`` Extra options: ``-DHWSEM_ALL=ON``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_alif_hwsem -- -DHWSEM_ALL=ON

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_alif_hwsem -- -DHWSEM_ALL=ON

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_alif_hwsem -- -DHWSEM_ALL=ON

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_alif_hwsem -- -DHWSEM_ALL=ON

.. _build-commands-i2c:

I2C
---

Sample path: ``../alif/samples/drivers/i2c_dw`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/i2c_dw

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/i2c_dw

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-dk ../alif/samples/drivers/i2c_dw

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-dk ../alif/samples/drivers/i2c_dw

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-dk ../alif/samples/drivers/i2c_dw

Sample path: ``../alif/samples/sensor/bme680`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/sensor/bme680

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/sensor/bme680

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-dk ../alif/samples/sensor/bme680

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/sensor/bme680

.. _build-commands-i2s:

I2S
---

Sample path: ``samples/drivers/i2s/echo`` Snippet: ``-S alif-i2s-echo``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-i2s-echo samples/drivers/i2s/echo

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-i2s-echo samples/drivers/i2s/echo

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-i2s-echo samples/drivers/i2s/echo

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-i2s-echo samples/drivers/i2s/echo

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-i2s-echo samples/drivers/i2s/echo

Sample path: ``samples/drivers/i2s/output`` Snippet: ``-S alif-i2s-output``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-i2s-output samples/drivers/i2s/output

Sample path: ``../alif/samples/drivers/i2s_codec`` Snippet: ``-S i2s-codec``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S i2s-codec ../alif/samples/drivers/i2s_codec

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S i2s-codec ../alif/samples/drivers/i2s_codec

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S i2s-codec ../alif/samples/drivers/i2s_codec

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S i2s-codec ../alif/samples/drivers/i2s_codec

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S i2s-codec ../alif/samples/drivers/i2s_codec

Sample path: ``../alif/samples/drivers/i2s_duplex`` Snippet: ``-S i2s-duplex``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S i2s-duplex ../alif/samples/drivers/i2s_duplex

Sample path: ``../alif/samples/drivers/pm/i2s_dw`` Snippet: ``-S i2s-dw-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S i2s-dw-pm-s2ram-tcm ../alif/samples/drivers/pm/i2s_dw -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S i2s-dw-pm-s2ram-tcm ../alif/samples/drivers/pm/i2s_dw -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/i2s_dw`` Snippet: ``-S i2s-dw-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S i2s-dw-pm-mram ../alif/samples/drivers/pm/i2s_dw

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S i2s-dw-pm-mram ../alif/samples/drivers/pm/i2s_dw

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S i2s-dw-pm-mram ../alif/samples/drivers/pm/i2s_dw

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S i2s-dw-pm-mram ../alif/samples/drivers/pm/i2s_dw

.. _build-commands-i3c:

I3C
---

Sample path: ``samples/sensor/bmi323`` Snippet: ``-S alif-dk-ak``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-dk-ak samples/sensor/bmi323

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-dk-ak samples/sensor/bmi323

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-dk-ak samples/sensor/bmi323

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-dk-ak samples/sensor/bmi323

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-dk-ak samples/sensor/bmi323

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-dk-ak samples/sensor/bmi323

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-dk-ak samples/sensor/bmi323

Sample path: ``../alif/samples/sensor/bmi323_pm`` Snippet: ``-S alif-dk-ak`` ``-S pm-system-off-s2ram-tcm``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk-ak -S pm-system-off-s2ram-tcm ../alif/samples/sensor/bmi323_pm

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-dk-ak -S pm-system-off-s2ram-tcm ../alif/samples/sensor/bmi323_pm

Sample path: ``../alif/samples/sensor/bmi323_pm`` Snippet: ``-S alif-dk-ak`` ``-S pm-system-off-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dk-ak -S pm-system-off-mram ../alif/samples/sensor/bmi323_pm

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-dk-ak -S pm-system-off-mram ../alif/samples/sensor/bmi323_pm

.. _build-commands-isp:

ISP
---

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/video/boards/isp.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.conf"

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_selfie.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/video/boards/isp.conf;$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_selfie.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.conf;$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_selfie.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.conf;$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_selfie.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.conf;$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_selfie.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.conf;$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

.. _build-commands-jpeg:

JPEG Encoder
------------

Sample path: ``../alif/samples/drivers/jpeg`` Snippet: ``-S alif-dk-ak``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-dk-ak ../alif/samples/drivers/jpeg

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-dk-ak ../alif/samples/drivers/jpeg

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-dk-ak ../alif/samples/drivers/jpeg

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-dk-ak ../alif/samples/drivers/jpeg

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overlay  $PWD/../alif/samples/drivers/video/boards/jpeg.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/video/boards/isp.conf  $PWD/../alif/samples/drivers/video/boards/jpeg.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overla" $PWD/../alif/samples/drivers/video/boards/jpeg.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.con" $PWD/../alif/samples/drivers/video/boards/jpeg.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overla" $PWD/../alif/samples/drivers/video/boards/jpeg.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.con" $PWD/../alif/samples/drivers/video/boards/jpeg.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overla" $PWD/../alif/samples/drivers/video/boards/jpeg.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.con" $PWD/../alif/samples/drivers/video/boards/jpeg.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overla" $PWD/../alif/samples/drivers/video/boards/jpeg.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.con" $PWD/../alif/samples/drivers/video/boards/jpeg.conf"

.. _build-commands-lp-i2c:

LPI2C
-----

Sample path: ``../alif/samples/drivers/lpi2c`` Snippet: ``-S alif-lpi2c``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-lpi2c ../alif/samples/drivers/lpi2c

.. _build-commands-lp-rtc:

LPRTC
-----

Sample path: ``samples/drivers/counter/alarm`` Extra options: ``-DOVERLAY_CONFIG=boards/alif_rtc.conf`` ``-DDTC_OVERLAY_FILE=boards/alif_rtc.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/drivers/counter/alarm -- -DOVERLAY_CONFIG=boards/alif_rtc.conf -DDTC_OVERLAY_FILE=boards/alif_rtc.overlay

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/drivers/counter/alarm -- -DOVERLAY_CONFIG=boards/alif_rtc.conf -DDTC_OVERLAY_FILE=boards/alif_rtc.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/drivers/counter/alarm -- -DOVERLAY_CONFIG=boards/alif_rtc.conf -DDTC_OVERLAY_FILE=boards/alif_rtc.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/drivers/counter/alarm -- -DOVERLAY_CONFIG=boards/alif_rtc.conf -DDTC_OVERLAY_FILE=boards/alif_rtc.overlay

.. _build-commands-lp-timer:

LP Timer
--------

Sample path: ``samples/drivers/counter/alarm`` Extra options: ``-DDTC_OVERLAY_FILE=boards/alif_lptimer.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/drivers/counter/alarm -- -DDTC_OVERLAY_FILE=boards/alif_lptimer.overlay

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/drivers/counter/alarm -- -DDTC_OVERLAY_FILE=boards/alif_lptimer.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/drivers/counter/alarm -- -DDTC_OVERLAY_FILE=boards/alif_lptimer.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/drivers/counter/alarm -- -DDTC_OVERLAY_FILE=boards/alif_lptimer.overlay

.. _build-commands-lvgl:

LVGL
----

Sample path: ``../alif/samples/modules/lvgl/gui``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/modules/lvgl/gui

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/modules/lvgl/gui

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/modules/lvgl/gui

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/modules/lvgl/gui

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he ../alif/samples/modules/lvgl/gui

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp ../alif/samples/modules/lvgl/gui

Sample path: ``../alif/samples/modules/lvgl/benchmark``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/modules/lvgl/benchmark

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/modules/lvgl/benchmark

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/modules/lvgl/benchmark

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/modules/lvgl/benchmark

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_ak/ae822fa0e5597xx0/rtss_he ../alif/samples/modules/lvgl/benchmark

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp ../alif/samples/modules/lvgl/benchmark

.. _build-commands-mhu:

MHU
---

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhuv2`` Extra options: ``-DCONFIG_HE_HP_S=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HE_HP_S=y

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HE_HP_S=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HE_HP_S=y

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HE_HP_S=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhuv2`` Extra options: ``-DCONFIG_HP_HE_R=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HP_HE_R=y

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HP_HE_R=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HP_HE_R=y

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HP_HE_R=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhuv2`` Extra options: ``-DCONFIG_HE_HP_S=y`` ``-DCONFIG_USE_MHU1=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HE_HP_S=y -DCONFIG_USE_MHU1=y

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HE_HP_S=y -DCONFIG_USE_MHU1=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HE_HP_S=y -DCONFIG_USE_MHU1=y

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HE_HP_S=y -DCONFIG_USE_MHU1=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhuv2`` Extra options: ``-DCONFIG_HP_HE_R=y`` ``-DCONFIG_USE_MHU1=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HP_HE_R=y -DCONFIG_USE_MHU1=y

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HP_HE_R=y -DCONFIG_USE_MHU1=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HP_HE_R=y -DCONFIG_USE_MHU1=y

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhuv2 -- -DCONFIG_HP_HE_R=y -DCONFIG_USE_MHU1=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell`` Extra options: ``-DCONFIG_HE_HP_S=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HE_HP_S=y

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HE_HP_S=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HE_HP_S=y

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HE_HP_S=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell`` Extra options: ``-DCONFIG_HP_HE_R=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HP_HE_R=y

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HP_HE_R=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HP_HE_R=y

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HP_HE_R=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell`` Extra options: ``-DCONFIG_HE_A32_S=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HE_A32_S=y

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HE_A32_S=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HE_A32_S=y

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_HE_A32_S=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell`` Extra options: ``-DCONFIG_A32_HE_R=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for APSS:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/apss ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_A32_HE_R=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for APSS:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/apss ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell -- -DCONFIG_A32_HE_R=y

.. _build-commands-mipi-camera:

MIPI Camera
-----------

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0.overlay"

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=boards/serial_camera_ov5675_selfie.overlay`` ``-DOVERLAY_CONFIG=boards/isp.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE=boards/serial_camera_ov5675_selfie.overlay -DOVERLAY_CONFIG=boards/isp.conf

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE=boards/serial_camera_ov5675_selfie.overlay -DOVERLAY_CONFIG=boards/isp.conf

Sample path: ``../alif/samples/drivers/pm/cam_pm`` Snippet: ``-S devkit-he-arx3a0-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S devkit-he-arx3a0-tcm ../alif/samples/drivers/pm/cam_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S devkit-he-arx3a0-tcm ../alif/samples/drivers/pm/cam_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S devkit-he-arx3a0-tcm ../alif/samples/drivers/pm/cam_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S devkit-he-arx3a0-tcm ../alif/samples/drivers/pm/cam_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/cam_pm`` Snippet: ``-S devkit-hp-arx3a0-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S devkit-hp-arx3a0-mram ../alif/samples/drivers/pm/cam_pm

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S devkit-hp-arx3a0-mram ../alif/samples/drivers/pm/cam_pm

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S devkit-hp-arx3a0-mram ../alif/samples/drivers/pm/cam_pm

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S devkit-hp-arx3a0-mram ../alif/samples/drivers/pm/cam_pm

Sample path: ``../alif/samples/drivers/pm/cam_pm`` Snippet: ``-S devkit-he-ov5675-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S devkit-he-ov5675-tcm ../alif/samples/drivers/pm/cam_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S devkit-he-ov5675-tcm ../alif/samples/drivers/pm/cam_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S devkit-he-ov5675-tcm ../alif/samples/drivers/pm/cam_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S devkit-he-ov5675-tcm ../alif/samples/drivers/pm/cam_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/cam_pm`` Snippet: ``-S devkit-hp-ov5675-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S devkit-hp-ov5675-mram ../alif/samples/drivers/pm/cam_pm

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S devkit-hp-ov5675-mram ../alif/samples/drivers/pm/cam_pm

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S devkit-hp-ov5675-mram ../alif/samples/drivers/pm/cam_pm

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S devkit-hp-ov5675-mram ../alif/samples/drivers/pm/cam_pm

.. _build-commands-mram:

MRAM
----

Sample path: ``samples/subsys/fs/littlefs`` Snippet: ``-S alif-lfs-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-lfs-mram samples/subsys/fs/littlefs

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-lfs-mram samples/subsys/fs/littlefs

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-lfs-mram samples/subsys/fs/littlefs

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-lfs-mram samples/subsys/fs/littlefs

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-lfs-mram samples/subsys/fs/littlefs

.. _build-commands-ospi-flash:

OSPI Flash
----------

Sample path: ``../alif/samples/drivers/spi_flash`` Snippet: ``-S ospi-flash``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash

Sample path: ``../alif/samples/drivers/spi_flash`` Snippet: ``-S ospi-flash`` Extra options: ``-DCONFIG_ALIF_OSPI_FLASH_XIP=y`` ``-DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

.. _build-commands-parallel-camera:

Parallel Camera
---------------

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_lpcam.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_lpcam.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_lpcam.overlay"

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/video -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay"

Sample path: ``../alif/samples/drivers/video``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/video

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/video

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he ../alif/samples/drivers/video

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he ../alif/samples/drivers/video

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he ../alif/samples/drivers/video

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he ../alif/samples/drivers/video

.. _build-commands-parallel-display:

Parallel Display
----------------

Sample path: ``../alif/samples/drivers/display`` Snippet: ``-S parallel-display``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S parallel-display ../alif/samples/drivers/display

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S parallel-display ../alif/samples/drivers/display

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S parallel-display ../alif/samples/drivers/display

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S parallel-display ../alif/samples/drivers/display

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S parallel-display ../alif/samples/drivers/display

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S parallel-display ../alif/samples/drivers/display

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S parallel-display ../alif/samples/drivers/display

Sample path: ``../alif/samples/drivers/pm/display_pm`` Snippet: ``-S display-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S display-pm-s2ram-tcm ../alif/samples/drivers/pm/display_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S display-pm-s2ram-tcm ../alif/samples/drivers/pm/display_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S display-pm-s2ram-tcm ../alif/samples/drivers/pm/display_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S display-pm-s2ram-tcm ../alif/samples/drivers/pm/display_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S display-pm-s2ram-tcm ../alif/samples/drivers/pm/display_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/display_pm`` Snippet: ``-S display-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S display-pm-mram ../alif/samples/drivers/pm/display_pm

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S display-pm-mram ../alif/samples/drivers/pm/display_pm

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S display-pm-mram ../alif/samples/drivers/pm/display_pm

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S display-pm-mram ../alif/samples/drivers/pm/display_pm

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S display-pm-mram ../alif/samples/drivers/pm/display_pm

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S display-pm-mram ../alif/samples/drivers/pm/display_pm

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S display-pm-mram ../alif/samples/drivers/pm/display_pm

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S display-pm-mram ../alif/samples/drivers/pm/display_pm

.. _build-commands-pdm:

PDM
---

Sample path: ``../alif/samples/drivers/audio/dmic_alif`` Snippet: ``-S alif-pdm``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-pdm ../alif/samples/drivers/audio/dmic_alif

Sample path: ``../alif/samples/drivers/pm/alif_pdm`` Snippet: ``-S pdm-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S pdm-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_pdm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S pdm-pm-s2ram-tcm ../alif/samples/drivers/pm/alif_pdm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/alif_pdm`` Snippet: ``-S pdm-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S pdm-pm-mram ../alif/samples/drivers/pm/alif_pdm

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S pdm-pm-mram ../alif/samples/drivers/pm/alif_pdm

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S pdm-pm-mram ../alif/samples/drivers/pm/alif_pdm

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S pdm-pm-mram ../alif/samples/drivers/pm/alif_pdm

.. _build-commands-power-management:

Power Management
----------------

Sample path: ``../alif/samples/drivers/pm/system_off`` Snippet: ``-S pm-system-off-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S pm-system-off-s2ram-tcm ../alif/samples/drivers/pm/system_off -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S pm-system-off-s2ram-tcm ../alif/samples/drivers/pm/system_off -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/system_off`` Snippet: ``-S pm-system-off-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S pm-system-off-mram ../alif/samples/drivers/pm/system_off

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S pm-system-off-mram ../alif/samples/drivers/pm/system_off

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S pm-system-off-mram ../alif/samples/drivers/pm/system_off

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S pm-system-off-mram ../alif/samples/drivers/pm/system_off

Sample path: ``../alif/samples/drivers/pm/system_off`` Snippet: ``-S pm-system-off-s2ram-sram0``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S pm-system-off-s2ram-sram0 ../alif/samples/drivers/pm/system_off

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S pm-system-off-s2ram-sram0 ../alif/samples/drivers/pm/system_off

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S pm-system-off-s2ram-sram0 ../alif/samples/drivers/pm/system_off

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S pm-system-off-s2ram-sram0 ../alif/samples/drivers/pm/system_off

.. _build-commands-psram:

PSRAM
-----

Sample path: ``../alif/samples/drivers/spi_psram``

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/spi_psram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/spi_psram

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he ../alif/samples/drivers/spi_psram

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he ../alif/samples/drivers/spi_psram

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he ../alif/samples/drivers/spi_psram

Sample path: ``../alif/samples/drivers/spi_psram`` Extra options: ``-DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/spi_psram -- -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/spi_psram -- -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/spi_psram -- -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/spi_psram -- -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

.. _build-commands-pwm:

PWM
---

Sample path: ``samples/basic/fade_led`` Snippet: ``-S alif-fade-led``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-fade-led samples/basic/fade_led

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-fade-led samples/basic/fade_led

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-fade-led samples/basic/fade_led

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-fade-led samples/basic/fade_led

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-fade-led samples/basic/fade_led

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-fade-led samples/basic/fade_led

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-fade-led samples/basic/fade_led

Sample path: ``samples/basic/blinky_pwm`` Snippet: ``-S alif-blinky-pwm``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-blinky-pwm samples/basic/blinky_pwm

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-blinky-pwm samples/basic/blinky_pwm

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-blinky-pwm samples/basic/blinky_pwm

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-blinky-pwm samples/basic/blinky_pwm

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-blinky-pwm samples/basic/blinky_pwm

Sample path: ``../alif/samples/drivers/pm/pwm-alif`` Snippet: ``-S pwm-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S pwm-pm-s2ram-tcm ../alif/samples/drivers/pm/pwm-alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/pwm-alif`` Snippet: ``-S pwm-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S pwm-pm-mram ../alif/samples/drivers/pm/pwm-alif

.. _build-commands-qdec:

QDEC
----

Sample path: ``samples/sensor/qdec`` Snippet: ``-S alif-qdec``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-qdec samples/sensor/qdec

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-qdec samples/sensor/qdec

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-qdec samples/sensor/qdec

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-qdec samples/sensor/qdec

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-qdec samples/sensor/qdec

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-qdec samples/sensor/qdec

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-qdec samples/sensor/qdec

Sample path: ``../alif/samples/drivers/pm/qdec_alif`` Snippet: ``-S qdec-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S qdec-pm-s2ram-tcm ../alif/samples/drivers/pm/qdec_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S qdec-pm-s2ram-tcm ../alif/samples/drivers/pm/qdec_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S qdec-pm-s2ram-tcm ../alif/samples/drivers/pm/qdec_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S qdec-pm-s2ram-tcm ../alif/samples/drivers/pm/qdec_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/qdec_alif`` Snippet: ``-S qdec-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S qdec-pm-mram ../alif/samples/drivers/pm/qdec_alif

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S qdec-pm-mram ../alif/samples/drivers/pm/qdec_alif

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S qdec-pm-mram ../alif/samples/drivers/pm/qdec_alif

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S qdec-pm-mram ../alif/samples/drivers/pm/qdec_alif

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S qdec-pm-mram ../alif/samples/drivers/pm/qdec_alif

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S qdec-pm-mram ../alif/samples/drivers/pm/qdec_alif

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S qdec-pm-mram ../alif/samples/drivers/pm/qdec_alif

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S qdec-pm-mram ../alif/samples/drivers/pm/qdec_alif

.. _build-commands-sdmmc:

SDMMC
-----

Sample path: ``samples/subsys/fs/fs_sample`` Snippet: ``-S alif-sdmmc``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-sdmmc samples/subsys/fs/fs_sample

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-sdmmc samples/subsys/fs/fs_sample

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-sdmmc samples/subsys/fs/fs_sample

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-sdmmc samples/subsys/fs/fs_sample

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-sdmmc samples/subsys/fs/fs_sample

Sample path: ``samples/subsys/fs/fs_sample`` Snippet: ``-S alif-emmc``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-emmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-emmc samples/subsys/fs/fs_sample

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-emmc samples/subsys/fs/fs_sample

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-emmc samples/subsys/fs/fs_sample

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-emmc samples/subsys/fs/fs_sample

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-emmc samples/subsys/fs/fs_sample

Sample path: ``samples/net/wifi/shell``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/net/wifi/shell

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/net/wifi/shell

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp samples/net/wifi/shell

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/net/wifi/shell

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/net/wifi/shell

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp samples/net/wifi/shell

Sample path: ``samples/net/wifi/shell`` Extra options: ``-DEXTRA_CONF_FILE=overlay-p2p.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/net/wifi/shell -- -DEXTRA_CONF_FILE=overlay-p2p.conf

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/net/wifi/shell -- -DEXTRA_CONF_FILE=overlay-p2p.conf

Sample path: ``samples/net/zperf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/net/zperf

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/net/zperf

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp samples/net/zperf

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/net/zperf

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/net/zperf

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp samples/net/zperf

.. _build-commands-se-tool-flash:

SE Tool Flashing for Alif DevKits
---------------------------------

Sample path: ``samples/hello_world``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/hello_world

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/hello_world

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/hello_world

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/hello_world

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he samples/hello_world

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp samples/hello_world

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/hello_world

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/hello_world

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he samples/hello_world

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/hello_world

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he samples/hello_world

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp samples/hello_world

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he samples/hello_world

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he samples/hello_world

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he samples/hello_world

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he samples/hello_world

.. _build-commands-serial-display:

Serial Display
--------------

Sample path: ``../alif/samples/drivers/display`` Snippet: ``-S serial-display-2lane``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S serial-display-2lane ../alif/samples/drivers/display

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S serial-display-2lane ../alif/samples/drivers/display

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S serial-display-2lane ../alif/samples/drivers/display

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S serial-display-2lane ../alif/samples/drivers/display

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S serial-display-2lane ../alif/samples/drivers/display

Sample path: ``../alif/samples/drivers/display`` Snippet: ``-S serial-display-1lane``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S serial-display-1lane ../alif/samples/drivers/display

Sample path: ``../alif/samples/drivers/pm/display_pm`` Snippet: ``-S display-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256`` ``-DEXTRA_DTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/pm/display_pm/serial_display_1lane.overlay`` ``-DEXTRA_CONF_FILE=$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf``

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S display-pm-s2ram-tcm ../alif/samples/drivers/pm/display_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256 -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_1lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S display-pm-s2ram-tcm ../alif/samples/drivers/pm/display_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256 -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_1lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p auto -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S display-pm-s2ram-tcm ../alif/samples/drivers/pm/display_pm -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256 -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_1lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Sample path: ``../alif/samples/drivers/pm/display_pm`` Snippet: ``-S display-pm-mram`` Extra options: ``-DEXTRA_DTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay`` ``-DEXTRA_CONF_FILE=$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S display-pm-mram ../alif/samples/drivers/pm/display_pm -- -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S display-pm-mram ../alif/samples/drivers/pm/display_pm -- -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S display-pm-mram ../alif/samples/drivers/pm/display_pm -- -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S display-pm-mram ../alif/samples/drivers/pm/display_pm -- -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S display-pm-mram ../alif/samples/drivers/pm/display_pm -- -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S display-pm-mram ../alif/samples/drivers/pm/display_pm -- -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S display-pm-mram ../alif/samples/drivers/pm/display_pm -- -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S display-pm-mram ../alif/samples/drivers/pm/display_pm -- -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

.. _build-commands-spi:

SPI
---

Sample path: ``../alif/samples/drivers/spi_dw`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-dk ../alif/samples/drivers/spi_dw

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-dk ../alif/samples/drivers/spi_dw

Sample path: ``../alif/samples/drivers/pm/spi_dw`` Snippet: ``-S spi-dw-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S spi-dw-pm-s2ram-tcm ../alif/samples/drivers/pm/spi_dw -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S spi-dw-pm-s2ram-tcm ../alif/samples/drivers/pm/spi_dw -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/spi_dw`` Snippet: ``-S spi-dw-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S spi-dw-pm-mram ../alif/samples/drivers/pm/spi_dw

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S spi-dw-pm-mram ../alif/samples/drivers/pm/spi_dw

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S spi-dw-pm-mram ../alif/samples/drivers/pm/spi_dw

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S spi-dw-pm-mram ../alif/samples/drivers/pm/spi_dw

Sample path: ``../alif/samples/drivers/pm/spi_dw`` Snippet: ``-S spi-dw-pm-s2ram-sram0``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S spi-dw-pm-s2ram-sram0 ../alif/samples/drivers/pm/spi_dw

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S spi-dw-pm-s2ram-sram0 ../alif/samples/drivers/pm/spi_dw

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S spi-dw-pm-s2ram-sram0 ../alif/samples/drivers/pm/spi_dw

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S spi-dw-pm-s2ram-sram0 ../alif/samples/drivers/pm/spi_dw

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S spi-dw-pm-s2ram-sram0 ../alif/samples/drivers/pm/spi_dw

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S spi-dw-pm-s2ram-sram0 ../alif/samples/drivers/pm/spi_dw

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S spi-dw-pm-s2ram-sram0 ../alif/samples/drivers/pm/spi_dw

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S spi-dw-pm-s2ram-sram0 ../alif/samples/drivers/pm/spi_dw

.. _build-commands-touchscreen:

GT911 Touchscreen
-----------------

Sample path: ``samples/subsys/input/input_dump`` Snippet: ``-S alif-ak-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-ak-dk samples/subsys/input/input_dump

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-ak-dk samples/subsys/input/input_dump

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-ak-dk samples/subsys/input/input_dump

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-ak-dk samples/subsys/input/input_dump

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-ak-dk samples/subsys/input/input_dump

.. _build-commands-uart:

UART
----

Sample path: ``samples/drivers/uart/echo_bot``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/drivers/uart/echo_bot

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/drivers/uart/echo_bot

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/drivers/uart/echo_bot

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/drivers/uart/echo_bot

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he samples/drivers/uart/echo_bot

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp samples/drivers/uart/echo_bot

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/drivers/uart/echo_bot

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/drivers/uart/echo_bot

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he samples/drivers/uart/echo_bot

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/drivers/uart/echo_bot

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he samples/drivers/uart/echo_bot

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp samples/drivers/uart/echo_bot

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he samples/drivers/uart/echo_bot

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he samples/drivers/uart/echo_bot

Sample path: ``samples/drivers/uart/echo_bot`` Snippet: ``-S lpuart``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S lpuart samples/drivers/uart/echo_bot

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S lpuart samples/drivers/uart/echo_bot

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S lpuart samples/drivers/uart/echo_bot

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S lpuart samples/drivers/uart/echo_bot

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S lpuart samples/drivers/uart/echo_bot

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S lpuart samples/drivers/uart/echo_bot

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S lpuart samples/drivers/uart/echo_bot

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S lpuart samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S lpuart samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S lpuart samples/drivers/uart/echo_bot

Sample path: ``samples/drivers/uart/echo_bot`` Snippet: ``-S uart0-rts-cts``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S uart0-rts-cts samples/drivers/uart/echo_bot

Sample path: ``samples/drivers/uart/echo_bot`` Snippet: ``-S uart2-hfosc-clk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S uart2-hfosc-clk samples/drivers/uart/echo_bot

Sample path: ``../alif/samples/drivers/uart/echo_dma`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/uart/echo_dma

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-dk ../alif/samples/drivers/uart/echo_dma

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/uart/echo_dma

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-dk ../alif/samples/drivers/uart/echo_dma

.. _build-commands-usb-device:

USB Device
----------

Sample path: ``samples/subsys/usb/cdc_acm`` Extra options: ``-DCONF_FILE=usbd_next_prj.conf`` ``-DDTC_OVERLAY_FILE=boards/alif_usb.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he samples/subsys/usb/cdc_acm -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Sample path: ``samples/subsys/usb/mass`` Snippet: ``-S alif-msc-sd`` Extra options: ``-DCONF_FILE=usbd_next_prj.conf;boards/alif_msc_sd.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-msc-sd samples/subsys/usb/mass -- -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Sample path: ``samples/subsys/usb/mass`` Extra options: ``-DCONF_FILE=usbd_next_prj.conf`` ``-DDTC_OVERLAY_FILE=boards/alif_usb.overlay;ramdisk.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he samples/subsys/usb/mass -- -DCONF_FILE=usbd_next_prj.conf -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Sample path: ``../alif/samples/subsys/usb/mass`` Snippet: ``-S alif-msc-ramdisk-ospi-sd``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-msc-ramdisk-ospi-sd ../alif/samples/subsys/usb/mass

Sample path: ``samples/subsys/usb/hid-joystick`` Snippet: ``-S alif-hid-joystick`` Extra options: ``-DCONF_FILE=usbd_next_prj.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-hid-joystick samples/subsys/usb/hid-joystick -- -DCONF_FILE=usbd_next_prj.conf

Sample path: ``samples/net/sockets/echo_server`` Extra options: ``-DEXTRA_CONF_FILE=overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf`` ``-DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/net/sockets/echo_server -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/net/sockets/echo_server -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/net/sockets/echo_server -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/net/sockets/echo_server -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he samples/net/sockets/echo_server -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp samples/net/sockets/echo_server -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he samples/net/sockets/echo_server -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp samples/net/sockets/echo_server -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he samples/net/sockets/echo_server -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Sample path: ``../alif/samples/drivers/video_uvc``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/video_uvc

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/video_uvc

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he ../alif/samples/drivers/video_uvc

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he ../alif/samples/drivers/video_uvc

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he ../alif/samples/drivers/video_uvc

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he ../alif/samples/drivers/video_uvc

Sample path: ``../alif/samples/drivers/video_uvc`` Extra options: ``-DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_arx3a0.overlay`` ``-DOVERLAY_CONFIG=boards/arx3a0.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/video_uvc -- -DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_arx3a0.overlay -DOVERLAY_CONFIG=boards/arx3a0.conf

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/video_uvc -- -DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_arx3a0.overlay -DOVERLAY_CONFIG=boards/arx3a0.conf

.. _build-commands-utimer-counter:

UTimer Counter
--------------

Sample path: ``samples/drivers/counter/alarm`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/drivers/counter/alarm -- -DDTC_OVERLAY_FILE="$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/drivers/counter/alarm -- -DDTC_OVERLAY_FILE="$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he samples/drivers/counter/alarm -- -DDTC_OVERLAY_FILE="$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp samples/drivers/counter/alarm -- -DDTC_OVERLAY_FILE="$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay"

Sample path: ``../alif/samples/drivers/pm/counter_alif`` Snippet: ``-S counter-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S counter-pm-s2ram-tcm ../alif/samples/drivers/pm/counter_alif -- -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/counter_alif`` Snippet: ``-S counter-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S counter-pm-mram ../alif/samples/drivers/pm/counter_alif

.. _build-commands-viewfinder:

Viewfinder
----------

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp ../alif/samples/drivers/viewfinder -- -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

.. _build-commands-wdt:

WDT
---

Sample path: ``samples/drivers/watchdog`` Snippet: ``-S alif-wdt``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_he -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_dk/ae302f80f55d5xx/rtss_hp -S alif-wdt samples/drivers/watchdog

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_he -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always -b alif_e7_ak/ae722f80f55d5xx/rtss_hp -S alif-wdt samples/drivers/watchdog

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_he -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp -S alif-wdt samples/drivers/watchdog

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp -S alif-wdt samples/drivers/watchdog

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always -b alif_e1c_dk/ae1c1f4051920hh/rtss_he -S alif-wdt samples/drivers/watchdog

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he -S alif-wdt samples/drivers/watchdog

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he -S alif-wdt samples/drivers/watchdog

