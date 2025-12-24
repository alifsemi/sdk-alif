{{ ... }}
/**
  \fn          void chbsp_debug_toggle(uint8_t dbg_pin_num)
  \brief       Toggle a debug indicator pin
  \param[in]   dbg_pin_num     Index of debug pin to toggle
*/
void chbsp_debug_toggle(uint8_t dbg_pin_num) {
    (void)dbg_pin_num;
}

/**
  \fn          void chbsp_debug_on(uint8_t dbg_pin_num)
  \brief       Turn on a debug indicator pin
  \param[in]   dbg_pin_num     Index of debug pin to turn on
*/
void chbsp_debug_on(uint8_t dbg_pin_num) {
    (void)dbg_pin_num;
}

/**
  \fn          void chbsp_debug_off(uint8_t dbg_pin_num)
  \brief       Turn off a debug indicator pin
  \param[in]   dbg_pin_num     Index of debug pin to turn off
*/
void chbsp_debug_off(uint8_t dbg_pin_num) {
    (void)dbg_pin_num;
}

/**
  \fn          void chbsp_reset_assert(void)
  \brief       Assert reset for all sensors (drive RESET_N low)
*/
void chbsp_reset_assert(void) {
}

/**
  \fn          void chbsp_reset_release(void)
  \brief       Deassert reset for all sensors (drive RESET_N high)
*/
void chbsp_reset_release(void) {
}

/**
  \fn          void chbsp_program_enable(ch_dev_t *dev_ptr)
  \brief       Assert PROG pin for specified device
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_program_enable(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_program_disable(ch_dev_t *dev_ptr)
  \brief       Deassert PROG pin for specified device
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_program_disable(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_set_int1_dir_out(ch_group_t *grp_ptr)
  \brief       Configure INT1 as output for a group of sensors
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_set_int1_dir_out(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_set_int1_dir_out(ch_dev_t *dev_ptr)
  \brief       Configure INT1 as output for one sensor
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_set_int1_dir_out(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_set_int1_dir_in(ch_group_t *grp_ptr)
  \brief       Configure INT1 as input for a group of sensors
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_set_int1_dir_in(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_set_int1_dir_in(ch_dev_t *dev_ptr)
  \brief       Configure INT1 as input for one sensor
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_set_int1_dir_in(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_int1_clear(ch_group_t *grp_ptr)
  \brief       Drive INT1 low for a group of sensors
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_int1_clear(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_int1_clear(ch_dev_t *dev_ptr)
  \brief       Drive INT1 low for one sensor
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_int1_clear(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_int1_set(ch_group_t *grp_ptr)
  \brief       Drive INT1 high for a group of sensors
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_int1_set(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_int1_set(ch_dev_t *dev_ptr)
  \brief       Drive INT1 high for one sensor
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_int1_set(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_int1_interrupt_enable(ch_group_t *grp_ptr)
  \brief       Enable INT1 interrupt for a group of sensors
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_int1_interrupt_enable(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_int1_interrupt_enable(ch_dev_t *dev_ptr)
  \brief       Enable INT1 interrupt for one sensor
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_int1_interrupt_enable(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_int1_interrupt_disable(ch_group_t *grp_ptr)
  \brief       Disable INT1 interrupt for a group of sensors
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_int1_interrupt_disable(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_int1_interrupt_disable(ch_dev_t *dev_ptr)
  \brief       Disable INT1 interrupt for one sensor
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_int1_interrupt_disable(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_set_int2_dir_out(ch_group_t *grp_ptr)
  \brief       Configure INT2 as output for a group of sensors (ICU)
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_set_int2_dir_out(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_set_int2_dir_out(ch_dev_t *dev_ptr)
  \brief       Configure INT2 as output for one sensor (ICU)
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_set_int2_dir_out(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_set_int2_dir_in(ch_group_t *grp_ptr)
  \brief       Configure INT2 as input for a group of sensors (ICU)
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_set_int2_dir_in(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_set_int2_dir_in(ch_dev_t *dev_ptr)
  \brief       Configure INT2 as input for one sensor (ICU)
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_set_int2_dir_in(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_int2_clear(ch_group_t *grp_ptr)
  \brief       Drive INT2 low for a group of sensors (ICU)
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_int2_clear(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_int2_clear(ch_dev_t *dev_ptr)
  \brief       Drive INT2 low for one sensor (ICU)
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_int2_clear(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_int2_set(ch_group_t *grp_ptr)
  \brief       Drive INT2 high for a group of sensors (ICU)
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_int2_set(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_int2_set(ch_dev_t *dev_ptr)
  \brief       Drive INT2 high for one sensor (ICU)
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_int2_set(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_int2_interrupt_enable(ch_group_t *grp_ptr)
  \brief       Enable INT2 interrupt for a group of sensors (ICU)
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_int2_interrupt_enable(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_int2_interrupt_enable(ch_dev_t *dev_ptr)
  \brief       Enable INT2 interrupt for one sensor (ICU)
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_int2_interrupt_enable(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_group_int2_interrupt_disable(ch_group_t *grp_ptr)
  \brief       Disable INT2 interrupt for a group of sensors (ICU)
  \param[in]   grp_ptr         Pointer to group descriptor
*/
void chbsp_group_int2_interrupt_disable(ch_group_t *grp_ptr) {
    (void)grp_ptr;
}

/**
  \fn          void chbsp_int2_interrupt_disable(ch_dev_t *dev_ptr)
  \brief       Disable INT2 interrupt for one sensor (ICU)
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_int2_interrupt_disable(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_delay_us(uint32_t us)
  \brief       Delay for specified microseconds
  \param[in]   us              Microseconds to delay
*/
void chbsp_delay_us(uint32_t us) {
    (void)us;
}

/**
  \fn          void chbsp_delay_ms(uint32_t ms)
  \brief       Delay for specified milliseconds
  \param[in]   ms              Milliseconds to delay
*/
void chbsp_delay_ms(uint32_t ms) {
    (void)ms;
}

/**
  \fn          uint32_t chbsp_timestamp_ms(void)
  \brief       Return a free-running counter value in milliseconds
  \return      Milliseconds tick count
*/
uint32_t chbsp_timestamp_ms(void) {
    return 0U;
}

/**
  \fn          uint8_t chbsp_event_wait(uint16_t time_out_ms, uint32_t event_mask)
  \brief       Wait for any interrupt event or timeout
  \param[in]   time_out_ms     Timeout in milliseconds
  \param[in]   event_mask      Event bitmask to wait for
  \return      0 if event received, 1 if timeout
*/
uint8_t chbsp_event_wait(uint16_t time_out_ms, uint32_t event_mask) {
    (void)time_out_ms;
    (void)event_mask;
    return 0U;
}

/**
  \fn          void chbsp_event_notify(uint32_t event_mask)
  \brief       Notify an interrupt event (ISR context)
  \param[in]   event_mask      Event bitmask corresponding to device(s)
*/
void chbsp_event_notify(uint32_t event_mask) {
    (void)event_mask;
}

/**
  \fn          void chbsp_event_wait_setup(uint32_t event_mask)
  \brief       Prepare mechanism to wait for an event
  \param[in]   event_mask      Event bitmask to configure
*/
void chbsp_event_wait_setup(uint32_t event_mask) {
    (void)event_mask;
}

/**
  \fn          int chbsp_i2c_init(void)
  \brief       Initialize host I2C hardware and software structures
  \return      0 on success, non-zero on error
*/
int chbsp_i2c_init(void) {
    return 0;
}

/**
  \fn          uint8_t chbsp_i2c_get_info(ch_group_t *grp_ptr, uint8_t dev_num, ch_i2c_info_t *info_ptr)
  \brief       Get I2C address/bus/flags for a device
  \param[in]   grp_ptr         Pointer to group descriptor
  \param[in]   dev_num         Device number within group
  \param[out]  info_ptr        Pointer to I2C info structure to fill
  \return      0 on success, 1 on error
*/
uint8_t chbsp_i2c_get_info(ch_group_t *grp_ptr, uint8_t dev_num, ch_i2c_info_t *info_ptr) {
    (void)grp_ptr;
    (void)dev_num;
    (void)info_ptr;
    return 0U;
}

/**
  \fn          int chbsp_i2c_write(ch_dev_t *dev_ptr, const uint8_t *data, uint16_t num_bytes)
  \brief       Write bytes to an I2C slave
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[in]   data            Pointer to data buffer
  \param[in]   num_bytes       Number of bytes to write
  \return      0 on success, 1 on error or NACK
*/
int chbsp_i2c_write(ch_dev_t *dev_ptr, const uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          int chbsp_i2c_mem_write(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes)
  \brief       Write bytes to an I2C slave using memory/register addressing
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[in]   mem_addr        Internal memory/register address
  \param[in]   data            Pointer to data buffer
  \param[in]   num_bytes       Number of bytes to write
  \return      0 on success, 1 on error or NACK
*/
int chbsp_i2c_mem_write(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)mem_addr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          int chbsp_i2c_mem_write_nb(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes)
  \brief       Start non-blocking I2C write using memory/register addressing
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[in]   mem_addr        Internal memory/register address
  \param[in]   data            Pointer to data buffer
  \param[in]   num_bytes       Number of bytes to write
  \return      0 on success, 1 on error or NACK
*/
int chbsp_i2c_mem_write_nb(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)mem_addr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          int chbsp_i2c_read(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes)
  \brief       Read bytes from an I2C slave
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[out]  data            Pointer to receive buffer
  \param[in]   num_bytes       Number of bytes to read
  \return      0 on success, 1 on error or NACK
*/
int chbsp_i2c_read(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          int chbsp_i2c_mem_read(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes)
  \brief       Read bytes from an I2C slave using memory/register addressing
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[in]   mem_addr        Internal memory/register address
  \param[out]  data            Pointer to receive buffer
  \param[in]   num_bytes       Number of bytes to read
  \return      0 on success, 1 on error or NACK
*/
int chbsp_i2c_mem_read(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)mem_addr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          int chbsp_i2c_read_nb(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes)
  \brief       Start non-blocking I2C read
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[out]  data            Pointer to receive buffer
  \param[in]   num_bytes       Number of bytes to read
  \return      0 on success, 1 on error or NACK
*/
int chbsp_i2c_read_nb(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          int chbsp_i2c_mem_read_nb(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes)
  \brief       Start non-blocking I2C read using memory/register addressing
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[in]   mem_addr        Internal memory/register address
  \param[out]  data            Pointer to receive buffer
  \param[in]   num_bytes       Number of bytes to read
  \return      0 on success, 1 on error
*/
int chbsp_i2c_mem_read_nb(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)mem_addr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          void chbsp_external_irq_handler(chdrv_transaction_t *trans)
  \brief       IRQ callout for external devices sharing SPI/I2C bus
  \param[in]   trans           Pointer to transaction control structure
*/
void chbsp_external_irq_handler(chdrv_transaction_t *trans) {
    (void)trans;
}

/**
  \fn          void chbsp_i2c_reset(ch_dev_t *dev_ptr)
  \brief       Reset I2C bus associated with device
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_i2c_reset(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_spi_cs_on(ch_dev_t *dev_ptr)
  \brief       Assert SPI chip select
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_spi_cs_on(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          void chbsp_spi_cs_off(ch_dev_t *dev_ptr)
  \brief       Deassert SPI chip select
  \param[in]   dev_ptr         Pointer to device descriptor
*/
void chbsp_spi_cs_off(ch_dev_t *dev_ptr) {
    (void)dev_ptr;
}

/**
  \fn          int chbsp_spi_write(ch_dev_t *dev_ptr, const uint8_t *data, uint16_t num_bytes)
  \brief       Write bytes to an SPI slave
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[in]   data            Pointer to data buffer
  \param[in]   num_bytes       Number of bytes to write
  \return      0 on success, 1 on error
*/
int chbsp_spi_write(ch_dev_t *dev_ptr, const uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          int chbsp_spi_read(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes)
  \brief       Read bytes from an SPI slave
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[out]  data            Pointer to receive buffer
  \param[in]   num_bytes       Number of bytes to read
  \return      0 on success, 1 on error
*/
int chbsp_spi_read(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          int chbsp_spi_mem_read_nb(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes)
  \brief       Start non-blocking SPI read using memory/register addressing
  \param[in]   dev_ptr         Pointer to device descriptor
  \param[in]   mem_addr        Internal memory/register address
  \param[out]  data            Pointer to receive buffer
  \param[in]   num_bytes       Number of bytes to read
  \return      0 on success, 1 on error
*/
int chbsp_spi_mem_read_nb(ch_dev_t *dev_ptr, uint16_t mem_addr, uint8_t *data, uint16_t num_bytes) {
    (void)dev_ptr;
    (void)mem_addr;
    (void)data;
    (void)num_bytes;
    return 0;
}

/**
  \fn          void chbsp_print_str(const char *str)
  \brief       Output a text string via serial interface for debugging
  \param[in]   str             NUL-terminated string to print
*/
void chbsp_print_str(const char *str) {
    (void)str;
}
