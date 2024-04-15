#include <rtthread.h>
#include "hal_data.h"

#ifdef BSP_USING_FS

#include <dfs_fs.h>

#define DBG_TAG "app.filesystem"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifdef BSP_USING_SDCARD_FS
#include <drv_sdhi.h>

/* SD Card hot plug detection pin */
#define SD_CHECK_PIN  "p405"

static rt_base_t sd_check_pin = 0;

static void _sdcard_mount(void)
{
    rt_device_t device;

    device = rt_device_find("sd");
    rt_kprintf("rt_device_find %x \r\n", device);
    if (device == NULL)
    {
        mmcsd_wait_cd_changed(0);
        sdcard_change();
        mmcsd_wait_cd_changed(RT_WAITING_FOREVER);
        device = rt_device_find("sd");
    }

    if (device != RT_NULL)
    {
        if (dfs_mount("sd", "/", "elm", 0, 0) == RT_EOK)
        {
            LOG_I("sd card mount to '/'");
        }
        else
        {
            LOG_W("sd card mount to '/' failed!");
        }
    }
}

static void _sdcard_unmount(void)
{
    rt_thread_mdelay(200);
    dfs_unmount("/sdcard");
    LOG_I("Unmount \"/sdcard\"");

    mmcsd_wait_cd_changed(0);
    sdcard_change();
    mmcsd_wait_cd_changed(RT_WAITING_FOREVER);
}

static void sd_auto_mount(void *parameter)
{
    rt_uint8_t re_sd_check_pin = 1;
    rt_thread_mdelay(20);

    if (!rt_pin_read(sd_check_pin))
    {
        _sdcard_mount();
    }

    while (1)
    {
        rt_thread_mdelay(200);

        if (re_sd_check_pin && (re_sd_check_pin = rt_pin_read(sd_check_pin)) == 0)
        {
            _sdcard_mount();
        }

        if (!re_sd_check_pin && (re_sd_check_pin = rt_pin_read(sd_check_pin)) != 0)
        {
            _sdcard_unmount();
        }
    }
}

static void sd_mount(void)
{
    rt_thread_t tid;

    sd_check_pin = rt_pin_get(SD_CHECK_PIN);
    rt_pin_mode(sd_check_pin, PIN_MODE_INPUT_PULLUP);

    tid = rt_thread_create("sd_mount", sd_auto_mount, RT_NULL,
                           2048, RT_THREAD_PRIORITY_MAX - 2, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("create sd_mount thread err!");
        return;
    }
}

#else
#include <spi_msd.h>
#include "drv_sci.h"
int sd_mount(void)
{
    uint32_t cs_pin = BSP_IO_PORT_06_PIN_11;
    rt_hw_sci_spi_device_attach("sci7s", "scpi70", cs_pin);
    msd_init("sd0", "scpi70");
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        LOG_I("Mount \"/dev/sd0\" on \"/\"\n");
    }
    else
    {
        LOG_W("sd card mount to '/' failed!");
    }
    return 0;
}
#endif /* BSP_USING_SDCARD_FS */

int mount_init(void)
{
    sd_mount();
    return RT_EOK;
}
INIT_ENV_EXPORT(mount_init);
#endif
