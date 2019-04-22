
#include "lcm_drv.h"

/* Local Constants */
#define LCM_NAME "s6d7aa0_dsi_vdo_common"
#define LCM_ID (0x0)
#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

/* Local Variables */
#define SET_RESET_PIN(v) (lcm_util.set_reset_pin((v)))
#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

/**
 * REGFLAG_DELAY, used to trigger MDELAY,
 * REGFLAG_END_OF_TABLE, used to mark the end of LCM_setting_table.
 * their values dosen't matter until they,
 * match with any LCM_setting_table->cmd.
 */
#define REGFLAG_DELAY (0xFA)
#define REGFLAG_END_OF_TABLE (0XFB)

/* Local Debug Variables */
#define LCM_DBG_TAG "[LCM]"
#define LCM_LOGD(str, args...) pr_info(LCM_DBG_TAG "[%s][%s] " str, LCM_NAME, __func__, ##args)

#ifdef BUILD_LK
#undef LCM_LOGD
#define LCM_LOGD(str, args...) print(LCM_DBG_TAG "[%s][%s] " str, LCM_NAME, __func__, ##args)
#endif

/* Local Functions */
#define dsi_set_cmdq_V3(para_tbl,size,force_update)         lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define read_reg_v2(cmd, buffer, buffer_size)	            lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)
#define write_regs(addr, pdata, byte_nums)	                lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)   lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define wrtie_cmd(cmd)	lcm_util.dsi_write_cmd(cmd)

/* LCM Driver Implementations */

static LCM_UTIL_FUNCS lcm_util = { 0 };

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[5];
};

static struct LCM_setting_table lcm_initialization_setting[] =
{
     {0xf0, 2, {0x02,0x5a,0x5a,0x00}},
     {0xf1, 2, {0x5a,0x5a}},
     {0xfc, 2, {0x5a,0x5a}},
     {0xca, 4, {0x00,0x54,0x05,0x28}},
     {0xb1, 6, {0x25,0x40,0xe8,0x10,0x00,0x22}},
     {0xb2, 1, {0x21}},
     {0xe3, 1, {0x26}},
     {0xf2, 4, {0x0c,0x10,0x0e,0x0d}},
     {0xf5, 18, {0x6e,0x78,0x10,0x6a,0x27,0x1d,0x4c,0x4c,0x03,0x03,0x04,0x22,0x11,0x31,0x50,0x2a,0x16,0x75}},
     {0xf6, 7, {0x04,0x8c,0x0f,0x80,0x46,0x00,0x00}},
     {0xf7, 22, {0x04,0x14,0x16,0x18,0x1a,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x01,0x24,0x25,0x0c}},
     {0xf8, 22, {0x05,0x15,0x17,0x19,0x1b,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x01,0x24,0x25,0x0d}},
     {0xed, 11, {0xc0,0x58,0x38,0x58,0x38,0x58,0x38,0x04,0x04,0x11,0x22}},
     {0xee, 24, {0x67,0x89,0x45,0x23,0x55,0x55,0x55,0x55,0x43,0x32,0x87,0xa9,0x33,0x33,0x33,0x33,0x89,0x01,0x00,0x00,0x01,0x89,0x00,0x00}},
     {0xef, 32, {0x3d,0x09,0x00,0x40,0x06,0x67,0x45,0x23,0x01,0x00,0x00,0x00,0x00,0x04,0x80,0x80,0x84,0x07,0x09,0x89,0x08,0x12,0x21,0x21,0x03,0x03,0x44,0x33,0x00,0x00,0x00,0x00}},
     {0xfa, 17, {0x10,0x30,0x15,0x1a,0x11,0x15,0x1c,0x1a,0x1e,0x28,0x2c,0x2b,0x2b,0x2a,0x2a,0x20,0x2e}},
     {0xfb, 17, {0x10,0x30,0x15,0x1a,0x11,0x15,0x1c,0x1a,0x1e,0x28,0x2c,0x2b,0x2b,0x2a,0x2a,0x20,0x2e}},
     {0xc5, 1, {0x21}},
     {0xfe, 1, {0x48}},
     {0xc8, 2, {0x24,0x53}},
     {0x11, 1, {0x00}},
     {REGFLAG_DELAY,120,{}},
     {0x29, 1,{0x00}},
     {REGFLAG_DELAY,20,{}},
     {REGFLAG_END_OF_TABLE,0,{}},
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = 
{
    // Sleep Mode On
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    {REGFLAG_END_OF_TABLE, 0, {}},
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    int i;
    for(i = 0; i < count; i++)
    {
        switch (table[i].cmd) {
        case REGFLAG_DELAY :
            MDELAY(table[i].count);
            break;
        case REGFLAG_END_OF_TABLE :
            break;
        default:
            dsi_set_cmdq_V2(table[i].cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type = 2;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->dsi.mode = 720;

	params->dsi.LANE_NUM = 3;
	
        params->dsi.data_format.color_order = 0;
        params->dsi.data_format.trans_seq = 0;
        params->dsi.data_format.padding = 0;
	params->dsi.data_format.format = 2;

	params->dsi.packet_size = 256;
	params->dsi.PS = 2;
	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch = 4;
	params->dsi.vertical_frontporch = 8;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 14;
	params->dsi.horizontal_backporch = 140;
	params->dsi.horizontal_frontporch = 16;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 245;
}

static void lcm_init(void)
{
    LCM_LOGD("Starting LCM Initialization!");

    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(120);

    push_table(lcm_initialization_setting,
        sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}

static unsigned int lcm_compare_id(void)
{
    return 1;
}

static void lcm_suspend(void)
{
    push_table(lcm_deep_sleep_mode_in_setting,
        sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(120);

    /**
    // For debuging lcm_compare_id
    if(lcm_compare_id())
        LCM_LOGD("yay! lcm id is correct.");
    */
}

/* Get LCM Driver Hooks */
LCM_DRIVER s6d7aa0_dsi_vdo_common_lcm_drv =
{
    .name           = LCM_NAME,
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_init,
    .compare_id     = lcm_compare_id,
};
