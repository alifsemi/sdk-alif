/// Own BD address source of the device
#include "gapm.h"

enum alif_addr_type
{
   /// Generated static random address
   ALIF_STATIC_RAND_ADDR = 0u,
   /// Generated resolvable private random address
   ALIF_GEN_RSLV_RAND_ADDR,
   /// Generated non-resolvable private random address
   ALIF_GEN_NON_RSLV_RAND_ADDR,
    /// Generated Public Address
    ALIF_PUBLIC_ADDR,
};
uint8_t address_verif(uint8_t SAMPLE_ADDR_TYPE, gapm_config_t *gapm_cfg);
