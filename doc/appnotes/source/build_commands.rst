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

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/adc \
     -S alif-adc

.. _build-commands-apss-a32:

Running Zephyr on the Cortex-A32 based APSS
-------------------------------------------

Sample path: ``samples/hello_world``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for APSS:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/apss \
     samples/hello_world

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for APSS:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/apss \
     samples/hello_world

.. _build-commands-can:

CAN (Controller Area Network)
-----------------------------

Sample path: ``samples/drivers/can/counter`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/can/counter \
     -S alif-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/drivers/can/counter \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/can/counter \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/drivers/can/counter \
     -S alif-dk

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/can/counter \
     -S alif-dk

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/can/counter \
     -S alif-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/can/counter \
     -S alif-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/drivers/can/counter \
     -S alif-dk

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/can/counter \
     -S alif-dk

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/drivers/can/counter \
     -S alif-dk

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/drivers/can/counter \
     -S alif-dk

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/drivers/can/counter \
     -S alif-dk

.. _build-commands-cmp:

CMP
---

Sample path: ``../alif/samples/drivers/cmp`` Snippet: ``-S alif-cmp``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/cmp \
     -S alif-cmp

.. _build-commands-cpu-freq-pstate:

RTSS CPU Frequency P-states
---------------------------

Sample path: ``../alif/samples/drivers/pm/pstate_set`` Snippet: ``-S pstate-he`` on HE, ``-S pstate-hp`` on HP

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-he

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-hp

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-he

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-hp

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-he

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-hp

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-he

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-hp

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-he

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-he

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-he

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/pm/pstate_set \
     -S pstate-he

.. _build-commands-crc:

CRC
---

Sample path: ``../alif/samples/drivers/crc``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/crc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/crc

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/crc

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/crc

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/crc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/crc

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/crc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/crc

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/crc

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/crc

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/crc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/crc

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/crc

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/crc

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/crc

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/crc

Sample path: ``../alif/samples/drivers/pm/alif_crc`` Snippet: ``-S crc-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/alif_crc`` Snippet: ``-S crc-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/pm/alif_crc \
     -S crc-pm-mram

.. _build-commands-dac:

DAC
---

Sample path: ``../alif/samples/drivers/dac``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/dac

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/dac

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/dac

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/dac

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/dac

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/dac

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/dac

.. _build-commands-dma:

DMA
---

Sample path: ``../alif/samples/drivers/spi_dw`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

.. _build-commands-entropy:

Entropy
-------

Sample path: ``tests/drivers/entropy/api``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     tests/drivers/entropy/api

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     tests/drivers/entropy/api

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     tests/drivers/entropy/api

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     tests/drivers/entropy/api

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     tests/drivers/entropy/api

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     tests/drivers/entropy/api

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     tests/drivers/entropy/api

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     tests/drivers/entropy/api

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     tests/drivers/entropy/api

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     tests/drivers/entropy/api

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     tests/drivers/entropy/api

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     tests/drivers/entropy/api

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     tests/drivers/entropy/api

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     tests/drivers/entropy/api

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     tests/drivers/entropy/api

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     tests/drivers/entropy/api

.. _build-commands-ethernet:

Ethernet
--------

Sample path: ``samples/net/dhcpv4_client`` Snippet: ``-S alif-dhcpv4-client``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/net/dhcpv4_client \
     -S alif-dhcpv4-client

.. _build-commands-ethos:

Ethos NPU U-55/U85
------------------

Sample path: ``../alif/samples/modules/tflite-micro/tflm_ethosu`` Snippet: ``-S ethos-u55-enable`` Extra options: ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/modules/tflite-micro/tflm_ethosu \
     -S ethos-u55-enable \
     -- \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/modules/tflite-micro/tflm_ethosu \
     -S ethos-u55-enable \
     -- \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/modules/tflite-micro/tflm_ethosu \
     -S ethos-u55-enable \
     -- \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/modules/tflite-micro/tflm_ethosu \
     -S ethos-u55-enable \
     -- \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Sample path: ``../alif/samples/modules/tflite-micro/tflm_ethosu`` Snippet: ``-S ethos-u55-enable`` Extra options: ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/modules/tflite-micro/tflm_ethosu \
     -S ethos-u55-enable \
     -- \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/modules/tflite-micro/tflm_ethosu \
     -S ethos-u55-enable \
     -- \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Sample path: ``../alif/samples/modules/tflite-micro/tflm_ethosu`` Snippet: ``-S ethos-u85-enable`` Extra options: ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/modules/tflite-micro/tflm_ethosu \
     -S ethos-u85-enable \
     -- \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Sample path: ``../alif/samples/modules/tflite-micro/tflm_ethosu`` Snippet: ``-S ethos-u85-apss-enable`` Extra options: ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for APSS:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/apss \
     ../alif/samples/modules/tflite-micro/tflm_ethosu \
     -S ethos-u85-apss-enable \
     -- \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Sample path: ``alif/samples/modules/executorch/kws_ethosu`` Snippet: ``-S ethos-u55-enable`` Extra options: ``-DET_PTE_FILE_PATH=./kws_u55_256.pte`` ``-DET_PTE_SECTION=.rodata.model`` ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     alif/samples/modules/executorch/kws_ethosu \
     -S ethos-u55-enable \
     -- \
     -DET_PTE_FILE_PATH=./kws_u55_256.pte \
     -DET_PTE_SECTION=.rodata.model \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     alif/samples/modules/executorch/kws_ethosu \
     -S ethos-u55-enable \
     -- \
     -DET_PTE_FILE_PATH=./kws_u55_256.pte \
     -DET_PTE_SECTION=.rodata.model \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Sample path: ``alif/samples/modules/executorch/kws_ethosu`` Snippet: ``-S ethos-u85-enable`` Extra options: ``-DET_PTE_FILE_PATH=./kws_u85_256.pte`` ``-DET_PTE_SECTION=.rodata.model`` ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     alif/samples/modules/executorch/kws_ethosu \
     -S ethos-u85-enable \
     -- \
     -DET_PTE_FILE_PATH=./kws_u85_256.pte \
     -DET_PTE_SECTION=.rodata.model \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     alif/samples/modules/executorch/kws_ethosu \
     -S ethos-u85-enable \
     -- \
     -DET_PTE_FILE_PATH=./kws_u85_256.pte \
     -DET_PTE_SECTION=.rodata.model \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Sample path: ``alif/samples/modules/executorch/kws_ethosu`` Snippet: ``-S ethos-u55-enable`` Extra options: ``-DET_PTE_FILE_PATH=./kws_u55_128.pte`` ``-DET_PTE_SECTION=.rodata.model`` ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     alif/samples/modules/executorch/kws_ethosu \
     -S ethos-u55-enable \
     -- \
     -DET_PTE_FILE_PATH=./kws_u55_128.pte \
     -DET_PTE_SECTION=.rodata.model \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     alif/samples/modules/executorch/kws_ethosu \
     -S ethos-u55-enable \
     -- \
     -DET_PTE_FILE_PATH=./kws_u55_128.pte \
     -DET_PTE_SECTION=.rodata.model \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     alif/samples/modules/executorch/kws_ethosu \
     -S ethos-u55-enable \
     -- \
     -DET_PTE_FILE_PATH=./kws_u55_128.pte \
     -DET_PTE_SECTION=.rodata.model \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     alif/samples/modules/executorch/kws_ethosu \
     -S ethos-u55-enable \
     -- \
     -DET_PTE_FILE_PATH=./kws_u55_128.pte \
     -DET_PTE_SECTION=.rodata.model \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Sample path: ``alif/samples/modules/executorch/kws_ethosu`` Snippet: ``-S ethos-u85-apss-enable`` Extra options: ``-DET_PTE_FILE_PATH=./kws_u85_256.pte`` ``-DET_PTE_SECTION=.rodata.model`` ``-DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for APSS:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/apss \
     alif/samples/modules/executorch/kws_ethosu \
     -S ethos-u85-apss-enable \
     -- \
     -DET_PTE_FILE_PATH=./kws_u85_256.pte \
     -DET_PTE_SECTION=.rodata.model \
     -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

.. _build-commands-gpio:

General-Purpose Input/Output (GPIO)
-----------------------------------

Sample path: ``samples/basic/blinky``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/basic/blinky

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/basic/blinky

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/basic/blinky

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/basic/blinky

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/basic/blinky

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/basic/blinky

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/basic/blinky

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/basic/blinky

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/basic/blinky

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/basic/blinky

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/basic/blinky

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/basic/blinky

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/basic/blinky

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/basic/blinky

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/basic/blinky

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/basic/blinky

Sample path: ``samples/basic/button`` Snippet: ``-S alif-button``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/basic/button \
     -S alif-button

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/basic/button \
     -S alif-button

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/basic/button \
     -S alif-button

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/basic/button \
     -S alif-button

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/basic/button \
     -S alif-button

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/basic/button \
     -S alif-button

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/basic/button \
     -S alif-button

.. _build-commands-helloworld-ospi:

Building Hello World for OSPI NOR Flash
---------------------------------------

Sample path: ``samples/hello_world`` Extra options: ``-DCONFIG_ARM_MPU=n`` ``-DCONFIG_FLASH_BASE_ADDRESS=0xC0200000``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/hello_world \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000

Sample path: ``samples/drivers/uart/echo_bot`` Extra options: ``-DCONFIG_ARM_MPU=n`` ``-DCONFIG_FLASH_BASE_ADDRESS=0xC0000000`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/drivers/uart/echo_bot \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/drivers/uart/echo_bot \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/drivers/uart/echo_bot \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/uart/echo_bot \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/drivers/uart/echo_bot \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/uart/echo_bot \
     -- \
     -DCONFIG_ARM_MPU=n \
     -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0

.. _build-commands-i2c:

I2C
---

Sample path: ``../alif/samples/drivers/i2c_dw`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/i2c_dw \
     -S alif-dk

Sample path: ``../alif/samples/sensor/bme680`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/sensor/bme680 \
     -S alif-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/sensor/bme680 \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/sensor/bme680 \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/sensor/bme680 \
     -S alif-dk

.. _build-commands-i2s:

I2S
---

Sample path: ``samples/drivers/i2s/echo`` Snippet: ``-S alif-i2s-echo``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/i2s/echo \
     -S alif-i2s-echo

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/drivers/i2s/echo \
     -S alif-i2s-echo

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/i2s/echo \
     -S alif-i2s-echo

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/drivers/i2s/echo \
     -S alif-i2s-echo

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/i2s/echo \
     -S alif-i2s-echo

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/i2s/echo \
     -S alif-i2s-echo

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/i2s/echo \
     -S alif-i2s-echo

Sample path: ``samples/drivers/i2s/output`` Snippet: ``-S alif-i2s-output``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/i2s/output \
     -S alif-i2s-output

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/i2s/output \
     -S alif-i2s-output

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/i2s/output \
     -S alif-i2s-output

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/i2s/output \
     -S alif-i2s-output

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/i2s/output \
     -S alif-i2s-output

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/drivers/i2s/output \
     -S alif-i2s-output

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/drivers/i2s/output \
     -S alif-i2s-output

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/drivers/i2s/output \
     -S alif-i2s-output

Sample path: ``../alif/samples/drivers/i2s_codec`` Snippet: ``-S i2s-codec``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/i2s_codec \
     -S i2s-codec

Sample path: ``../alif/samples/drivers/i2s_duplex`` Snippet: ``-S i2s-duplex``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/i2s_duplex \
     -S i2s-duplex

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/i2s_duplex \
     -S i2s-duplex

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/i2s_duplex \
     -S i2s-duplex

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/i2s_duplex \
     -S i2s-duplex

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/i2s_duplex \
     -S i2s-duplex

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/i2s_duplex \
     -S i2s-duplex

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/i2s_duplex \
     -S i2s-duplex

Sample path: ``../alif/samples/drivers/pm/i2s_dw`` Snippet: ``-S i2s-dw-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/i2s_dw \
     -S i2s-dw-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/i2s_dw \
     -S i2s-dw-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/i2s_dw`` Snippet: ``-S i2s-dw-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/i2s_dw \
     -S i2s-dw-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/i2s_dw \
     -S i2s-dw-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/i2s_dw \
     -S i2s-dw-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/i2s_dw \
     -S i2s-dw-pm-mram

.. _build-commands-i3c:

I3C
---

Sample path: ``samples/sensor/bmi323`` Snippet: ``-S alif-dk-ak``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/sensor/bmi323 \
     -S alif-dk-ak

Sample path: ``../alif/samples/sensor/bmi323_pm`` Snippet: ``-S alif-dk-ak`` ``-S pm-system-off-s2ram-tcm``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/sensor/bmi323_pm \
     -S alif-dk-ak \
     -S pm-system-off-s2ram-tcm

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/sensor/bmi323_pm \
     -S alif-dk-ak \
     -S pm-system-off-s2ram-tcm

Sample path: ``../alif/samples/sensor/bmi323_pm`` Snippet: ``-S alif-dk-ak`` ``-S pm-system-off-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/sensor/bmi323_pm \
     -S alif-dk-ak \
     -S pm-system-off-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/sensor/bmi323_pm \
     -S alif-dk-ak \
     -S pm-system-off-mram

.. _build-commands-lp-i2c:

LPI2C
-----

Sample path: ``../alif/samples/drivers/lpi2c`` Snippet: ``-S alif-lpi2c``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/lpi2c \
     -S alif-lpi2c

.. _build-commands-mhu:

MHU
---

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhuv2`` Extra options: ``-DCONFIG_HE_HP_S=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_arm_mhuv2 \
     -- \
     -DCONFIG_HE_HP_S=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_arm_mhuv2 \
     -- \
     -DCONFIG_HE_HP_S=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhuv2`` Extra options: ``-DCONFIG_HP_HE_R=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_arm_mhuv2 \
     -- \
     -DCONFIG_HP_HE_R=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_arm_mhuv2 \
     -- \
     -DCONFIG_HP_HE_R=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhuv2`` Extra options: ``-DCONFIG_HE_HP_S=y`` ``-DCONFIG_USE_MHU1=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_arm_mhuv2 \
     -- \
     -DCONFIG_HE_HP_S=y \
     -DCONFIG_USE_MHU1=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_arm_mhuv2 \
     -- \
     -DCONFIG_HE_HP_S=y \
     -DCONFIG_USE_MHU1=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhuv2`` Extra options: ``-DCONFIG_HP_HE_R=y`` ``-DCONFIG_USE_MHU1=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_arm_mhuv2 \
     -- \
     -DCONFIG_HP_HE_R=y \
     -DCONFIG_USE_MHU1=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_arm_mhuv2 \
     -- \
     -DCONFIG_HP_HE_R=y \
     -DCONFIG_USE_MHU1=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell`` Extra options: ``-DCONFIG_HE_HP_S=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell \
     -- \
     -DCONFIG_HE_HP_S=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell \
     -- \
     -DCONFIG_HE_HP_S=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell`` Extra options: ``-DCONFIG_HP_HE_R=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell \
     -- \
     -DCONFIG_HP_HE_R=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell \
     -- \
     -DCONFIG_HP_HE_R=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell`` Extra options: ``-DCONFIG_HE_A32_S=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell \
     -- \
     -DCONFIG_HE_A32_S=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell \
     -- \
     -DCONFIG_HE_A32_S=y

Sample path: ``../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell`` Extra options: ``-DCONFIG_A32_HE_R=y``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for APSS:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/apss \
     ../alif/samples/drivers/ipm/ipm_arm_mhu_doorbell \
     -- \
     -DCONFIG_A32_HE_R=y

.. _build-commands-mipi-camera:

MIPI Camera
-----------

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0.overlay"

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114_standard.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/serial_camera_mt9m114.conf"

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=boards/serial_camera_ov5675_selfie.overlay`` ``-DOVERLAY_CONFIG=boards/isp.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE=boards/serial_camera_ov5675_selfie.overlay \
     -DOVERLAY_CONFIG=boards/isp.conf

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE=boards/serial_camera_ov5675_selfie.overlay \
     -DOVERLAY_CONFIG=boards/isp.conf

Sample path: ``../alif/samples/drivers/pm/cam_pm`` Snippet: ``-S devkit-he-arx3a0-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/cam_pm \
     -S devkit-he-arx3a0-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/cam_pm \
     -S devkit-he-arx3a0-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/cam_pm`` Snippet: ``-S devkit-hp-arx3a0-mram``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/cam_pm \
     -S devkit-hp-arx3a0-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/cam_pm \
     -S devkit-hp-arx3a0-mram

Sample path: ``../alif/samples/drivers/pm/cam_pm`` Snippet: ``-S devkit-he-ov5675-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/cam_pm \
     -S devkit-he-ov5675-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/cam_pm \
     -S devkit-he-ov5675-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/cam_pm`` Snippet: ``-S devkit-hp-ov5675-mram``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/cam_pm \
     -S devkit-hp-ov5675-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/cam_pm \
     -S devkit-hp-ov5675-mram

.. _build-commands-mram:

MRAM
----

Sample path: ``samples/subsys/fs/littlefs`` Snippet: ``-S alif-lfs-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/subsys/fs/littlefs \
     -S alif-lfs-mram

.. _build-commands-ospi-flash:

OSPI Flash
----------

Sample path: ``../alif/samples/drivers/spi_flash`` Snippet: ``-S ospi-flash``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash

Sample path: ``../alif/samples/drivers/spi_flash`` Snippet: ``-S ospi-flash`` Extra options: ``-DCONFIG_ALIF_OSPI_FLASH_XIP=y`` ``-DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/spi_flash \
     -S ospi-flash \
     -- \
     -DCONFIG_ALIF_OSPI_FLASH_XIP=y \
     -DCONFIG_ALIF_OSPI_FLASH_BOOT_XIP_IMAGE=y

.. _build-commands-parallel-camera:

Parallel Camera
---------------

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_lpcam.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_lpcam.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_lpcam.overlay"

Sample path: ``../alif/samples/drivers/video`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/video \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/parallel_camera_mt9m114_cam.overlay"

Sample path: ``../alif/samples/drivers/video``

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/video

.. _build-commands-parallel-display:

Parallel Display
----------------

Sample path: ``../alif/samples/drivers/display`` Snippet: ``-S parallel-display``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/display \
     -S parallel-display

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/display \
     -S parallel-display

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/display \
     -S parallel-display

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/display \
     -S parallel-display

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/display \
     -S parallel-display

Sample path: ``../alif/samples/drivers/pm/display_pm`` Snippet: ``-S display-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/display_pm`` Snippet: ``-S display-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram

.. _build-commands-pdm:

PDM
---

Sample path: ``../alif/samples/drivers/audio/dmic_alif`` Snippet: ``-S alif-pdm``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/audio/dmic_alif \
     -S alif-pdm

Sample path: ``../alif/samples/drivers/pm/alif_pdm`` Snippet: ``-S pdm-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_pdm \
     -S pdm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_pdm \
     -S pdm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/alif_pdm`` Snippet: ``-S pdm-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_pdm \
     -S pdm-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/alif_pdm \
     -S pdm-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/alif_pdm \
     -S pdm-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/alif_pdm \
     -S pdm-pm-mram

.. _build-commands-power-management:

Power Management
----------------

Sample path: ``../alif/samples/drivers/pm/system_off`` Snippet: ``-S pm-system-off-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/system_off \
     -S pm-system-off-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/system_off \
     -S pm-system-off-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/system_off`` Snippet: ``-S pm-system-off-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/system_off \
     -S pm-system-off-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/system_off \
     -S pm-system-off-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/system_off \
     -S pm-system-off-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/system_off \
     -S pm-system-off-mram

Sample path: ``../alif/samples/drivers/pm/system_off`` Snippet: ``-S pm-system-off-s2ram-sram0``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/system_off \
     -S pm-system-off-s2ram-sram0

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/system_off \
     -S pm-system-off-s2ram-sram0

.. _build-commands-psram:

PSRAM
-----

Sample path: ``../alif/samples/drivers/spi_psram``

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_psram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_psram

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/spi_psram

Sample path: ``../alif/samples/drivers/spi_psram`` Extra options: ``-DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_psram \
     -- \
     -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_psram \
     -- \
     -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_psram \
     -- \
     -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_psram \
     -- \
     -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

.. _build-commands-pwm:

PWM
---

Sample path: ``samples/basic/fade_led`` Snippet: ``-S alif-fade-led``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/basic/fade_led \
     -S alif-fade-led

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/basic/fade_led \
     -S alif-fade-led

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/basic/fade_led \
     -S alif-fade-led

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/basic/fade_led \
     -S alif-fade-led

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/basic/fade_led \
     -S alif-fade-led

Sample path: ``samples/basic/blinky_pwm`` Snippet: ``-S alif-blinky-pwm``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/basic/blinky_pwm \
     -S alif-blinky-pwm

Sample path: ``../alif/samples/drivers/pm/pwm-alif`` Snippet: ``-S pwm-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/pwm-alif`` Snippet: ``-S pwm-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/pm/pwm-alif \
     -S pwm-pm-mram

.. _build-commands-qdec:

QDEC
----

Sample path: ``samples/sensor/qdec`` Snippet: ``-S alif-qdec``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/sensor/qdec \
     -S alif-qdec

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/sensor/qdec \
     -S alif-qdec

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/sensor/qdec \
     -S alif-qdec

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/sensor/qdec \
     -S alif-qdec

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/sensor/qdec \
     -S alif-qdec

Sample path: ``../alif/samples/drivers/pm/qdec_alif`` Snippet: ``-S qdec-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/qdec_alif`` Snippet: ``-S qdec-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/qdec_alif \
     -S qdec-pm-mram

.. _build-commands-sdmmc:

SDMMC
-----

Sample path: ``samples/subsys/fs/fs_sample`` Snippet: ``-S alif-sdmmc``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/subsys/fs/fs_sample \
     -S alif-sdmmc

Sample path: ``samples/subsys/fs/fs_sample`` Snippet: ``-S alif-emmc``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-emmc

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-emmc

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-emmc

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-emmc

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-emmc

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/fs/fs_sample \
     -S alif-emmc

Sample path: ``samples/net/wifi/shell``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/net/wifi/shell

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/net/wifi/shell

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/net/wifi/shell

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/net/wifi/shell

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/net/wifi/shell

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/net/wifi/shell

Sample path: ``samples/net/wifi/shell`` Extra options: ``-DEXTRA_CONF_FILE=overlay-p2p.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/net/wifi/shell \
     -- \
     -DEXTRA_CONF_FILE=overlay-p2p.conf

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/net/wifi/shell \
     -- \
     -DEXTRA_CONF_FILE=overlay-p2p.conf

Sample path: ``samples/net/zperf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/net/zperf

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/net/zperf

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/net/zperf

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/net/zperf

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/net/zperf

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/net/zperf

.. _build-commands-se-tool-flash:

SE Tool Flashing for Alif DevKits
---------------------------------

Sample path: ``samples/hello_world``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/hello_world

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/hello_world

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/hello_world

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/hello_world

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/hello_world

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/hello_world

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/hello_world

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/hello_world

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/hello_world

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/hello_world

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/hello_world

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/hello_world

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/hello_world

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/hello_world

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/hello_world

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/hello_world

.. _build-commands-serial-display:

Serial Display
--------------

Sample path: ``../alif/samples/drivers/display`` Snippet: ``-S serial-display-2lane``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/display \
     -S serial-display-2lane

Sample path: ``../alif/samples/drivers/display`` Snippet: ``-S serial-display-1lane``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/display \
     -S serial-display-1lane

Sample path: ``../alif/samples/drivers/pm/display_pm`` Snippet: ``-S display-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256`` ``-DEXTRA_DTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/pm/display_pm/serial_display_1lane.overlay`` ``-DEXTRA_CONF_FILE=$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf``

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256 \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_1lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256 \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_1lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256 \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_1lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Sample path: ``../alif/samples/drivers/pm/display_pm`` Snippet: ``-S display-pm-mram`` Extra options: ``-DEXTRA_DTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay`` ``-DEXTRA_CONF_FILE=$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/display_pm \
     -S display-pm-mram \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display_2lane.overlay" \
     -DEXTRA_CONF_FILE="$PWD/../alif/samples/drivers/pm/display_pm/serial_display.conf"

.. _build-commands-spi:

SPI
---

Sample path: ``../alif/samples/drivers/spi_dw`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/spi_dw \
     -S alif-dk

Sample path: ``../alif/samples/drivers/pm/spi_dw`` Snippet: ``-S spi-dw-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/spi_dw`` Snippet: ``-S spi-dw-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-mram

Sample path: ``../alif/samples/drivers/pm/spi_dw`` Snippet: ``-S spi-dw-pm-s2ram-sram0``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-s2ram-sram0

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-s2ram-sram0

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-s2ram-sram0

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/spi_dw \
     -S spi-dw-pm-s2ram-sram0

.. _build-commands-touchscreen:

GT911 Touchscreen
-----------------

Sample path: ``samples/subsys/input/input_dump`` Snippet: ``-S alif-ak-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/subsys/input/input_dump \
     -S alif-ak-dk

.. _build-commands-uart:

UART
----

Sample path: ``samples/drivers/uart/echo_bot``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/drivers/uart/echo_bot

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/drivers/uart/echo_bot

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/drivers/uart/echo_bot

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/uart/echo_bot

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/uart/echo_bot

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/drivers/uart/echo_bot

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/drivers/uart/echo_bot

Sample path: ``samples/drivers/uart/echo_bot`` Snippet: ``-S lpuart``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S lpuart

Sample path: ``samples/drivers/uart/echo_bot`` Snippet: ``-S uart0-rts-cts``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart0-rts-cts

Sample path: ``samples/drivers/uart/echo_bot`` Snippet: ``-S uart2-hfosc-clk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/drivers/uart/echo_bot \
     -S uart2-hfosc-clk

Sample path: ``../alif/samples/drivers/uart/echo_dma`` Snippet: ``-S alif-dk``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/uart/echo_dma \
     -S alif-dk

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/uart/echo_dma \
     -S alif-dk

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/uart/echo_dma \
     -S alif-dk

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/uart/echo_dma \
     -S alif-dk

.. _build-commands-usb-device:

USB Device
----------

Sample path: ``samples/subsys/usb/cdc_acm`` Extra options: ``-DCONF_FILE=usbd_next_prj.conf`` ``-DDTC_OVERLAY_FILE=boards/alif_usb.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/subsys/usb/cdc_acm \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE=boards/alif_usb.overlay

Sample path: ``samples/subsys/usb/mass`` Snippet: ``-S alif-msc-sd`` Extra options: ``-DCONF_FILE=usbd_next_prj.conf;boards/alif_msc_sd.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/subsys/usb/mass \
     -S alif-msc-sd \
     -- \
     -DCONF_FILE="usbd_next_prj.conf;boards/alif_msc_sd.conf"

Sample path: ``samples/subsys/usb/mass`` Extra options: ``-DCONF_FILE=usbd_next_prj.conf`` ``-DDTC_OVERLAY_FILE=boards/alif_usb.overlay;ramdisk.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/subsys/usb/mass \
     -- \
     -DCONF_FILE=usbd_next_prj.conf \
     -DDTC_OVERLAY_FILE="boards/alif_usb.overlay;ramdisk.overlay"

Sample path: ``../alif/samples/subsys/usb/mass`` Snippet: ``-S alif-msc-ramdisk-ospi-sd``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/subsys/usb/mass \
     -S alif-msc-ramdisk-ospi-sd

Sample path: ``samples/subsys/usb/hid-joystick`` Snippet: ``-S alif-hid-joystick`` Extra options: ``-DCONF_FILE=usbd_next_prj.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/subsys/usb/hid-joystick \
     -S alif-hid-joystick \
     -- \
     -DCONF_FILE=usbd_next_prj.conf

Sample path: ``samples/net/sockets/echo_server`` Extra options: ``-DEXTRA_CONF_FILE=overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf`` ``-DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/net/sockets/echo_server \
     -- \
     -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
     -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/net/sockets/echo_server \
     -- \
     -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
     -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/net/sockets/echo_server \
     -- \
     -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
     -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/net/sockets/echo_server \
     -- \
     -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
     -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/net/sockets/echo_server \
     -- \
     -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
     -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/net/sockets/echo_server \
     -- \
     -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
     -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/net/sockets/echo_server \
     -- \
     -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
     -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/net/sockets/echo_server \
     -- \
     -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
     -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/net/sockets/echo_server \
     -- \
     -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
     -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

Sample path: ``../alif/samples/drivers/video_uvc`` Extra options: ``-DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_arx3a0.overlay`` ``-DOVERLAY_CONFIG=boards/arx3a0.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/video_uvc \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_arx3a0.overlay \
     -DOVERLAY_CONFIG=boards/arx3a0.conf

.. _build-commands-utimer-counter:

UTimer Counter
--------------

Sample path: ``samples/drivers/counter/alarm`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/counter/alarm \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/drivers/counter/alarm \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/counter/alarm \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/drivers/counter/alarm \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay"

Sample path: ``../alif/samples/drivers/pm/counter_alif`` Snippet: ``-S counter-pm-s2ram-tcm`` Extra options: ``-DCONFIG_FLASH_BASE_ADDRESS=0x0`` ``-DCONFIG_FLASH_LOAD_OFFSET=0x0`` ``-DCONFIG_FLASH_SIZE=256``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -- \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

Sample path: ``../alif/samples/drivers/pm/counter_alif`` Snippet: ``-S counter-pm-mram``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

.. _build-commands-viewfinder:

Viewfinder
----------

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae302f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

Sample path: ``../alif/samples/drivers/viewfinder`` Extra options: ``-DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay`` ``-DOVERLAY_CONFIG=$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf``

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/viewfinder \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

.. _build-commands-wdt:

WDT
---

Sample path: ``samples/drivers/watchdog`` Snippet: ``-S alif-wdt``

Alif E7 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/watchdog \
     -S alif-wdt

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/drivers/watchdog \
     -S alif-wdt

Alif E7 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae722f80f55d5xx`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/drivers/watchdog \
     -S alif-wdt

Build for SoC variant ``ae722f80f55d5xx`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/drivers/watchdog \
     -S alif-wdt

Alif E8 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/watchdog \
     -S alif-wdt

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/watchdog \
     -S alif-wdt

Build for SoC variant ``ae402fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/watchdog \
     -S alif-wdt

Build for SoC variant ``ae402fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/drivers/watchdog \
     -S alif-wdt

Alif E8 AppKit
~~~~~~~~~~~~~~

Build for SoC variant ``ae822fa0e5597xx0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/drivers/watchdog \
     -S alif-wdt

Build for SoC variant ``ae822fa0e5597xx0`` for HP:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/watchdog \
     -S alif-wdt

Alif E1C DevKit
~~~~~~~~~~~~~~~

Build for SoC variant ``ae1c1f4051920hh`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/watchdog \
     -S alif-wdt

Alif B1 DevKit
~~~~~~~~~~~~~~

Build for SoC variant ``ab1c1f4m51820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/drivers/watchdog \
     -S alif-wdt

Build for SoC variant ``ab1c1f1m41820hh0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/drivers/watchdog \
     -S alif-wdt

Build for SoC variant ``ab1c1f1m41820ph0`` for HE:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/drivers/watchdog \
     -S alif-wdt

