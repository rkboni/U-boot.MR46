#include <common.h>
#include <i2c.h>
#include <board_data.h>
#include <meraki_config.h>
#include "ipq807x.h"
#include "sec_dat.h"  /* fuse file generated from qca-sectools */


#define DISP_LINE_LEN                0x10
#define EEPROM_I2C_ADDR              CONFIG_EEPROM_I2C_ADDR
#define EEPROM_PAGE_SIZE             32
#define EEPROM_I2C_SCL_WRITE_STATUS  0x200
#define BOARD_DATA_READ_SIZE         (sizeof(struct ar531x_boarddata) + (EEPROM_PAGE_SIZE - (sizeof(struct ar531x_boarddata) % EEPROM_PAGE_SIZE)))
#define MAX_I2C_RECOVER              2


// Need to make sure we read a multiple of the page size.
static uint8_t board_data_buf[BOARD_DATA_READ_SIZE] = {0};
static struct ar531x_boarddata* board_data = NULL;

struct eeprom_i2c_config
{
	uint16_t gpio_scl;
	uint16_t gpio_scl_func;
	uint16_t gpio_sda;
	uint16_t gpio_sda_func;
	uint16_t eeprom_addr;
};

/* valid eeprom configuration for axes */
static const struct eeprom_i2c_config valid_eeprom_i2c_config[] = {
    { 0, 3, 2, 3, 0x56 }, // Phoenix, Hendrix
};
#define NUM_VALID_EEPROM_CONFIG   (sizeof(valid_eeprom_i2c_config)/sizeof(valid_eeprom_i2c_config[0]))

static int read_board_data(void)
{
    struct ar531x_boarddata* bd = (struct ar531x_boarddata*)board_data_buf;

    uint8_t   *linebuf = (uint8_t *)bd;
    int nbytes = BOARD_DATA_READ_SIZE;
    int linebytes;
    unsigned int addr = 0;
    int i;
    uint8_t recover_i2c = 0;

    run_command("i2c dev 0", 0);

retry:
    for (i = 0; i < NUM_VALID_EEPROM_CONFIG; i++) {
       if (recover_i2c)
          toggle_scl_universal(valid_eeprom_i2c_config[i].gpio_sda, valid_eeprom_i2c_config[i].gpio_scl, EEPROM_I2C_SCL_WRITE_STATUS);

       char cmd[64];
       snprintf(cmd, 64, "i2c probe %x", valid_eeprom_i2c_config[i].eeprom_addr);
       if (run_command(cmd, 0) != CMD_RET_SUCCESS) {
          puts("Bad Board Data Magic!\n");

          if (recover_i2c < MAX_I2C_RECOVER) {
             puts("Retrying...\n");
             recover_i2c++;
             goto retry;
          }
          else
              continue;
       }
       do {
         linebytes = (nbytes > DISP_LINE_LEN) ? DISP_LINE_LEN : nbytes;

         if (i2c_read_data(NULL, valid_eeprom_i2c_config[i].eeprom_addr, addr, 2, linebuf, linebytes) != 0)
            break;

         linebuf += linebytes;
         addr += linebytes;
         nbytes -= linebytes;
      } while (nbytes > 0);
    }

    if (ntohl(bd->magic) == AR531X_BD_MAGIC) {
       printf("Meraki Product (major num): %d (minor num): %d\n",ntohs(bd->major), ntohs(bd->minor));
      board_data = bd;
       return 0;
    }

    return EINVAL;
}

int meraki_config_get_product(void)
{
    if (!board_data) {
        int err = read_board_data();
        if (err)
            return -1;
    }

    return ntohs(board_data->major);
}

char *get_meraki_prompt()
{
   return CONFIG_SYS_PROMPT;
}

enum meraki_product get_meraki_product_id()
{
   if (board_data) {
      return ntohs(board_data->major);
   }
   return MERAKI_BOARD_UNKNOWN;
}

void set_meraki_kernel_config()
{
   if (board_data) {
      char itb_config[16];
      if(ntohs(board_data->minor) <= 1) {
          snprintf(itb_config, 12, "config_%d", ntohs(board_data->major));
      }
      else {
          snprintf(itb_config, 16, "config_%d_%d", ntohs(board_data->major), ntohs(board_data->minor));
      }
      env_set("itb_config", itb_config);
      printf("set_meraki_kernel_config -> %s\n", itb_config);
   }
}

void meraki_axe_ethaddr(uchar *enetaddr)
{
   if (!board_data) {
      return;
   }

   memcpy(enetaddr, board_data->enet0Mac, 6);
}

bool meraki_check_unique_id(const void *dev_crt)
{
    const u32 *id = dev_crt;
    const u32 *qfprom_raw_serial_num = (const u32 *)0x000a0128;
    return *qfprom_raw_serial_num == *id;
}
