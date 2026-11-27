/* Linker snippet for BLE host patch sections to be placed in RAM. Note: patch code could be
   placed into read-only memory instead but will execute faster from RAM. */

SECTION_PROLOGUE(.ble_patch_code,,)
{
    __ble_patch_code_load_start = LOADADDR(.ble_patch_code);
    KEEP(*(._ble_patch_code));
    . = ALIGN(16);
} GROUP_LINK_IN(ROMABLE_REGION)

SECTION_PROLOGUE(.ble_patch_data,,)
{
    __ble_patch_data_load_start = LOADADDR(.ble_patch_data);
    KEEP(*(._ble_patch_data));
    . = ALIGN(16);
} GROUP_LINK_IN(RAMABLE_REGION AT > ROMABLE_REGION)

SECTION_PROLOGUE(.ble_patch_bss,(NOLOAD),)
{
    __ble_patch_bss_start = .;
    KEEP(*(._ble_patch_bss));
    . = ALIGN(16);
} GROUP_LINK_IN(RAMABLE_REGION)
