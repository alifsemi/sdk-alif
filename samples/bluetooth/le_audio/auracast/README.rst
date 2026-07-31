.. _bluetooth-auracast-sample:

BLE Auracast Sample
###################

Overview
********

This sample demonstrates the LE audio Auracast (broadcast) use-cases.

Sample supports both Auracast source and Auracast sink roles.
Scan delegator role is also supported to allow discovering and connecting to Auracast
sources using e.g. mobile phone as a Auracast Assistant (phone must be paired and bonded to the device).

Different roles can be selected using shell commands or via the custom Auracast
control GATT profile (connectable advertising, enabled by default):

.. code-block:: console

    auracast help
    auracast name <device_name>
    auracast source <stream_name> [--passwd <password>] [--codec <codec_name>] [--sdu <octets_per_frame_in_bytes>] [--rate <frame_rate_hz>] [--ms <frame_duration_in_ms>]
    auracast sink [<stream_name> [<password>]]
    auracast delegator
    auracast stop

BLE control profile
*******************

When :kconfig:option:`CONFIG_AURACAST_CONTROL_PROFILE` is enabled, the device
advertises a custom GATT service that configures and selects the Auracast mode.

Service UUID: ``0000AC00-0000-1000-8000-00805F9B34FB``

.. list-table:: Control profile characteristics
   :header-rows: 1
   :widths: 2 3 2

   * - Characteristic
     - UUID
     - Format
   * - Mode (R/W)
     - ``0000AC01-...``
     - ``uint8`` mode select
   * - Mode status (R/N)
     - ``0000AC02-...``
     - ``uint8`` current mode
   * - Stream name (R/W)
     - ``0000AC03-...``
     - UTF-8 string, max 31 bytes (empty clears)
   * - Encryption key (R/W)
     - ``0000AC04-...``
     - UTF-8 string, 4..16 bytes (empty disables)
   * - Codec (R/W)
     - ``0000AC05-...``
     - UTF-8 preset name, e.g. ``48_2`` (empty clears)
   * - SDU octets (R/W)
     - ``0000AC06-...``
     - ``uint16`` little-endian octets per frame
   * - Frame duration (R/W)
     - ``0000AC07-...``
     - UTF-8 string: ``7.5ms`` or ``10ms`` (``7.5`` / ``10`` also accepted on write)
   * - Frame rate (R/W)
     - ``0000AC08-...``
     - UTF-8 string in kHz: ``8``, ``16``, ``24``, ``32`` or ``48`` (``kHz`` suffix optional)

Write a single byte to the Mode characteristic:

* ``0`` - BLE config role (``ROLE_BLE_CONFIG``): stop Auracast activity and
  stay connectable for further mode selection
* ``1`` - Auracast source (uses stream name, encryption and codec/SDU/rate/duration)
* ``2`` - Auracast sink (uses stream name and encryption filter if set)
* ``3`` - Auracast scan delegator

Writing a codec preset updates SDU, frame duration and rate to the matching BAP
values. Writing SDU, duration or rate clears the codec preset string and uses the
manual values instead.

The Mode status characteristic reports the active mode and can notify on change.
Peripheral advertising for this service remains available in all modes so a
client can reconnect and switch roles again.

For example start the Auracast source with 48_2 codec configuration:

.. code-block:: console

   auracast source MyStream --codec 48_2
   # or with encryption enabled
   auracast source MyStream --passwd password --codec 48_2

and connect sink device to the stream:

.. code-block:: console

   auracast sink MyStream
   # or with encryption enabled
   auracast sink MyStream password

Auracast sink's stream_name can be a prefix to filter the available broadcasts.
Sink will connect immediately to the first fully matching broadcast stream name.
Sink will list all available broadcasts if the stream_name is not provided.

Power Management
********************

Application uses the power management API to allow the system to enter low power states when
the device is idle. The power management is integrated with the application logic to ensure
that the device can enter low power states when it is not actively transmitting or receiving
audio data.

Note: Sleeps are allowed with scan delegator and sink only at the moment. This also makes default
shell (UART2) unresponsive when mode is activated. This is a known issue and will be fixed in the
future. Board can be reset to recover shell responsiveness and change the role.
`lpuart.overlay` can be used to make shell available but will require additional configuration
(see Sw4 switch on the board). This just increase idle power comsumption a bit since the lpuart
will be active all the time.

Building and Running
********************

This sample can be found under :zephyr_file:`samples/bluetooth/le_audio/auracast` in the
sdk-alif tree.

See :ref:`Alif bluetooth samples section <alif-bluetooth-samples>` for details.

Configuration options:

.. list-table::

    * - :file:`prj.conf`
      - This is the standard default config. Sampling frequency 48kHz, 10ms frame duration, 100 octets per codec frame. The default retransmissions are set to 4.

BAP defined Codec Configuration Settings
******************************************

.. table:: BAP defined Codec Configuration Settings
   :widths: 1 1 1 1 1

   +---------------+-----------+-----------+-------------+---------+
   | Codec         | Supported | Supported | Supported   | Bitrate |
   | Configuration | Sampling  | Frame     | Octets per  | (kbps)  |
   | Setting       | Frequency | Duration  | Codec Frame |         |
   +===============+===========+===========+=============+=========+
   | 8_1           | 8         | 7.5 ms    | 26          | 27.734  |
   | 8_2           | 8         | 10 ms     | 30          | 24      |
   | 16_1          | 16        | 7.5 ms    | 30          | 32      |
   | 16_2\*        | 16        | 10 ms     | 40          | 32      |
   | 24_1          | 24        | 7.5 ms    | 45          | 48      |
   | 24_2\*\*      | 24        | 10 ms     | 60          | 48      |
   | 32_1          | 32        | 7.5 ms    | 60          | 64      |
   | 32_2          | 32        | 10 ms     | 80          | 64      |
   | 441_1         | 44.1      | 7.5 ms    | 97          | 95.06   |
   | 441_2         | 44.1      | 10 ms     | 130         | 95.55   |
   | 48_1          | 48        | 7.5 ms    | 75          | 80      |
   | 48_2          | 48        | 10 ms     | 100         | 80      |
   | 48_3          | 48        | 7.5 ms    | 90          | 96      |
   | 48_4          | 48        | 10 ms     | 120         | 96      |
   | 48_5          | 48        | 7.5 ms    | 117         | 124.8   |
   | 48_6          | 48        | 10 ms     | 155         | 124     |
   +---------------+-----------+-----------+-------------+---------+

\*\* Auracast mandated Standard Quality Public Broadcast Codec Configuration option 1
\*\* Auracast mandated Standard Quality Public Broadcast Codec Configuration option 2

All Auracast transmitters must support at least one broadcast stream that uses one of the
Standard Quality(SQ) codec configurations which are defined in Public Boradcast Profile(PBP).
Support for other codec configurations is optional.
Low latency configurations set retransmissions to 2, the high reliability ones to 4.
Low latency configurations are beneficial when there is ambient sound which would be causing echo effects.
