.. _snippet-openthread-shell:

OpenThread Shell
################

Overview
********

This snippet enables Zephyr networking with OpenThread and the OpenThread
shell (``ot``) on applications that do not already configure Thread.

It appends ``openthread-shell.conf``, which turns on:

* IPv6, UDP, and BSD sockets (IPv4 and TCP stay off)
* ``CONFIG_NET_L2_OPENTHREAD`` with Thread 1.3
* OpenThread shell and ping sender
* NVS settings for the Thread dataset
* Automatic OpenThread start (``CONFIG_OPENTHREAD_MANUAL_START=n``)

Default network parameters from the snippet:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Setting
     - Value
   * - Channel
     - 11
   * - PAN ID
     - 4660 (``0x1234``)
   * - Network key
     - ``00:11:22:33:44:55:66:77:88:99:aa:bb:cc:dd:ee:ff``

IEEE 802.15.4 radio support is available on Balletto B1. Ensemble boards can
build networking samples without Thread radio.

The Balletto ``testapp`` already sets these OpenThread options in its board
``*.conf`` files (with ``CONFIG_OPENTHREAD_MANUAL_START=y``). Use this snippet
when adding Thread to another application.

Building and Running
********************

Example: enable OpenThread on the Balletto DevKit HE core:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/applications/testapp \
     -S openthread-shell

Or pass the snippet as a CMake argument:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/applications/testapp \
     -- -DSNIPPET=openthread-shell

If ``CONFIG_OPENTHREAD_MANUAL_START`` is ``y`` in the application, bring the
interface up from the shell:

.. code-block:: console

   ot ifconfig up
   ot thread start

With the snippet default (automatic start) those commands are not required
after a successful attach.

Common OpenThread shell checks:

.. code-block:: console

   ot state
   ot dataset active
   ot ping <peer-ipv6>

Constraints
***********

* Increase ``CONFIG_MBEDTLS_HEAP_SIZE`` (snippet sets ``30000``) and the
  system workqueue stack when adding settings or extra networking features.
* Do not enable IPv4 (``CONFIG_NET_CONFIG_NEED_IPV4``) with this snippet; it
  is explicitly disabled.
* Optional OpenThread debug Kconfig symbols in the snippet are commented out
  to keep log volume low. Enable them only when debugging attach or MAC
  issues.
* Matter samples already enable OpenThread in ``prj.conf``. Prefer those
  configs over stacking this snippet on a Matter image.
