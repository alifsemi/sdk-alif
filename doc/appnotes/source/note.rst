Generic Note
============

**Building the Application for TCM and MRAM memory**

* For **TCM memory**, use:

  .. code-block:: bash

     -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0

* For **MRAM memory**, the build system uses MRAM addresses by default; no additional configuration is required.


**Debug Binary on the DevKit**

* Use ``west debug`` to debug the application through J-Link.
