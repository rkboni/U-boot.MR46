#include <common.h>
#include <meraki_config.h>

bool meraki_product_has_aq_phy(void)
{
    enum meraki_product product_id = get_meraki_product_id();
    if (product_id == MERAKI_BOARD_SASQUATCH ||
        product_id == MERAKI_BOARD_WOOKIE ||
        product_id == MERAKI_BOARD_DOCTOROCTOPUS)
        return true;

    return false;
}

#define TOGGLE_DELAY_US      4000
#define TOGGLE_MAX_RETRIES     50
int toggle_scl_universal(uint16_t sda_pin, uint16_t scl_pin, uint32_t scl_output_status)
{
    int i;
    volatile unsigned int *sda = (volatile unsigned int *)GPIO_IN_OUT_ADDR(sda_pin);
    uint32_t scl_config_old = *((volatile unsigned int *)GPIO_CONFIG_ADDR(scl_pin));

    if (*sda)
        return 0;

    // set SCL as output gpio, but save the old config state to restore later
    (*(volatile unsigned int *)GPIO_CONFIG_ADDR(scl_pin)) = scl_output_status;
    udelay(TOGGLE_DELAY_US);

    // toggle SCL until SDA is released or we have tried MAX_RETRIES times
    for (i = 0; i < TOGGLE_MAX_RETRIES; i++) {
        (*(volatile unsigned int *)GPIO_IN_OUT_ADDR(scl_pin)) = 0x2;
        udelay(TOGGLE_DELAY_US);
        (*(volatile unsigned int *)GPIO_IN_OUT_ADDR(scl_pin)) = 0x0;
        udelay(TOGGLE_DELAY_US);

        if (*sda) {
            debug("SDA released after %d toggles of SCL (pin %d); SDA=%d !!\n", i + 1, scl_pin, *sda);
            (*(volatile unsigned int *)GPIO_CONFIG_ADDR(scl_pin)) = scl_config_old;
            return 0;
        }
    }

    debug("SDA still low after %d toggles of SCL (pin %d) !!\n", TOGGLE_MAX_RETRIES, scl_pin);
    return -1;
}
