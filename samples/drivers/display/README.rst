.. _display-sample:

Display Sample
##############

Overview
********

This sample will draw some basic rectangles onto the display.
The rectangle colors and positions are chosen so that you can check the
orientation of the LCD and correct RGB bit order. The rectangles are drawn
in clockwise order, from top left corner: red, green, blue, grey. The shade of
grey changes from black through to white. If the grey looks too green or red
at any point or the order of the corners is not as described above then the LCD
may be endian swapped.

Building and Running
********************

As this is a generic sample it should work with any display supported by Zephyr.

Below is an example on how to build for :ref:`alif_e7_dk_rtss_he` and :ref:`alif_e7_dk_rtss_hp` board with a
:ref:`focus_lcd`.

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/display
   :board: alif_e7_dk_rtss_he
   :goals: build
   :shield: focus_lcd
   :compact:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/display
   :board: alif_e7_dk_rtss_hp
   :goals: build
   :shield: focus_lcd
   :compact:

For testing purpose without the need of any hardware, the :ref:`native_posix`
board is also supported and can be built as follows;

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/display
   :board: native_posix
   :goals: build
   :compact:

List of Alif Ensemble DevKit display shields
********************************************

- :ref:`focus_lcd`
