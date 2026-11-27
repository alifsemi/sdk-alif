/* Linker snippet for BLE host patch info to be placed in read-only memory. */

SECTION_PROLOGUE(.ble_patch_info,,)
{
    __ble_patch_info_start = .;
    KEEP(*(._ble_patch_info));
    . = ALIGN(16);
    __ble_patch_info_end = .;
} GROUP_LINK_IN(ROMABLE_REGION)
