#ifndef IO_INTERRUPT_H__
#define IO_INTERRUPT_H__

#define LS2K0300_IO_INT_UART00      0
#define LS2K0300_IO_INT_UART01      1
#define LS2K0300_IO_INT_UART_02_05  2
#define LS2K0300_IO_INT_UART_06_09  3
#define LS2K0300_IO_INT_I2C_00_01   4
#define LS2K0300_IO_INT_I2C_02_03   5
#define LS2K0300_IO_INT_SPI2        6
#define LS2K0300_IO_INT_SPI3        7
#define LS2K0300_IO_INT_CAN0        8
#define LS2K0300_IO_INT_CAN1        9
#define LS2K0300_IO_INT_CAN2        10
#define LS2K0300_IO_INT_CAN3        11
#define LS2K0300_IO_INT_I2S         12
#define LS2K0300_IO_INT_ATIMER      13
#define LS2K0300_IO_INT_GTIMER      14
#define LS2K0300_IO_INT_BTIMER      15
#define LS2K0300_IO_INT_PWM_0_1     16
#define LS2K0300_IO_INT_PWM_2_3     17
#define LS2K0300_IO_INT_ADC         18
#define LS2K0300_IO_INT_HPET0       19
#define LS2K0300_IO_INT_HPET1       20
#define LS2K0300_IO_INT_HPET2       21
#define LS2K0300_IO_INT_HPET3       22
#define LS2K0300_IO_INT_APB_DMA0    23
#define LS2K0300_IO_INT_APB_DMA1    24
#define LS2K0300_IO_INT_APB_DMA2    25
#define LS2K0300_IO_INT_APB_DMA3    26
#define LS2K0300_IO_INT_APB_DMA4    27
#define LS2K0300_IO_INT_APB_DMA5    28
#define LS2K0300_IO_INT_APB_DMA6    29
#define LS2K0300_IO_INT_APB_DMA7    30
#define LS2K0300_IO_INT_SDIO0_CTRL  31
#define LS2K0300_IO_INT_SDIO1_CTRL  32
#define LS2K0300_IO_INT_SDIO0_DMA   33
#define LS2K0300_IO_INT_SDIO1_DMA   34
#define LS2K0300_IO_INT_ENCRYPT_DMA 35
#define LS2K0300_IO_INT_AES         36
#define LS2K0300_IO_INT_DES         37
#define LS2K0300_IO_INT_SM3         38
#define LS2K0300_IO_INT_SM4         39
#define LS2K0300_IO_INT_RTC         40
#define LS2K0300_IO_INT_TOY         41
#define LS2K0300_IO_INT_RTC_TICK    42
#define LS2K0300_IO_INT_TOY_TICK    43
#define LS2K0300_IO_INT_SPI0        44
#define LS2K0300_IO_INT_SPI1        45
#define LS2K0300_IO_INT_EHCI        46
#define LS2K0300_IO_INT_OHCI        47
#define LS2K0300_IO_INT_OTG         48
#define LS2K0300_IO_INT_GMAC0       49
#define LS2K0300_IO_INT_GMAC1       50
#define LS2K0300_IO_INT_DC          51
#define LS2K0300_IO_INT_THSENS      52
#define LS2K0300_IO_INT_GPIO_0_15   53
#define LS2K0300_IO_INT_GPIO_16_31  54
#define LS2K0300_IO_INT_GPIO_32_47  55
#define LS2K0300_IO_INT_GPIO_48_63  56
#define LS2K0300_IO_INT_GPIO_64_79  57
#define LS2K0300_IO_INT_GPIO_80_95  58
#define LS2K0300_IO_INT_GPIO_96_105 59
/* #define LS2K0300_IO_INT_RESERVED    60 */
#define LS2K0300_IO_INT_DDR_ECC0    61
#define LS2K0300_IO_INT_DDR_ECC1    62
/* #define LS2K0300_IO_INT_RESERVED    63 */

#define LS2K0300_IO_ENTRY_NUM       8
#define LS2K0300_IO_INT_LOW_NUM     32
#define LS2K0300_IO_INT_HIGH_NUM    32
#define LS2K0300_IO_INT_NUM         (LS2K0300_IO_INT_LOW_NUM + LS2K0300_IO_INT_HIGH_NUM)

#define LS2K0300_IP0                0x10
#define LS2K0300_IP1                0x20
#define LS2K0300_IP2                0x40
#define LS2K0300_IP3                0x80

void rt_hw_io_interrupt_mask(int vector);
void rt_hw_io_interrupt_umask(int vector);
rt_isr_handler_t rt_hw_io_interrupt_install(int vector, rt_isr_handler_t handler,
                                            void *param, const char *name);
int rt_hw_io_interrupt_init(void);

#endif /* IO_INTERRUPT_H__ */
