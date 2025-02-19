#include "b_config.h"
#include "hal/inc/b_hal_gpio.h"

//         Register Address

#define GPIO_REG_OFF (0x400UL)
#define GPIO_REG_BASE (0x42020000UL)

// ref stm32h503xx.h
typedef struct
{
    volatile uint32_t MODE;   /*!< GPIO port mode register,               Address offset: 0x00   */
    volatile uint32_t OTYPE;  /*!< GPIO port output type register,        Address offset: 0x04  */
    volatile uint32_t OSPEED; /*!< GPIO port output speed register,       Address offset: 0x08 */
    volatile uint32_t PUPDR;  /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C   */
    volatile uint32_t IDR;  /*!< GPIO port input data register,         Address offset: 0x10      */
    volatile uint32_t ODR;  /*!< GPIO port output data register,        Address offset: 0x14      */
    volatile uint32_t BSRR; /*!< GPIO port bit set/reset  register,     Address offset: 0x18      */
    volatile uint32_t LCKR; /*!< GPIO port configuration lock register, Address offset: 0x1C      */
    volatile uint32_t AFR[2]; /*!< GPIO alternate function registers, Address offset: 0x20-0x24 */
    volatile uint32_t BRR; /*!< GPIO Bit Reset register,               Address offset: 0x28      */
    volatile uint32_t HSLVR;   /*!< GPIO high-speed low voltage register,  Address offset: 0x2C   */
    volatile uint32_t SECCFGR; /*!< GPIO secure configuration register,    Address offset: 0x30 */
} McuGpioReg_t;

void bMcuGpioConfig(bHalGPIOPort_t port, bHalGPIOPin_t pin, bHalGPIODir_t dir, bHalGPIOPull_t pull)
{
    uint32_t      mode_val   = 0;
    uint32_t      otype_val  = 0;
    uint32_t      ospeed_val = 3;
    uint32_t      pupd_val   = 0;
    McuGpioReg_t *pGpio      = (McuGpioReg_t *)(GPIO_REG_BASE + port * GPIO_REG_OFF);

    if (!B_HAL_GPIO_ISVALID(port, pin))
    {
        return;
    }

    if (dir == B_HAL_GPIO_OUTPUT)
    {
        mode_val   = (pin == B_HAL_PINAll) ? 0x55555555 : 1;
        otype_val  = (pin == B_HAL_PINAll) ? 0x00000000 : 0;
        ospeed_val = (pin == B_HAL_PINAll) ? 0xffffffff : 3;
        pupd_val   = (pin == B_HAL_PINAll) ? 0x00000000 : 0;
    }

    if (pull != B_HAL_GPIO_NOPULL)
    {
        if (pull == B_HAL_GPIO_PULLUP)
        {
            pupd_val = (pin == B_HAL_PINAll) ? 0x55555555 : 1;
        }
        else
        {
            pupd_val = (pin == B_HAL_PINAll) ? 0xAAAAAAAA : 2;
        }
    }

    if (pin == B_HAL_PINAll)
    {
        pGpio->MODE   = mode_val;
        pGpio->OTYPE  = otype_val;
        pGpio->OSPEED = ospeed_val;
        pGpio->PUPDR  = pupd_val;
    }
    else
    {
        pGpio->MODE &= ~(0x00000003 << (pin * 2));
        pGpio->MODE |= (mode_val << (pin * 2));
        pGpio->OTYPE &= ~(0x00000001 << (pin * 1));
        pGpio->OTYPE |= (otype_val << (pin * 1));
        pGpio->OSPEED &= ~(0x00000003 << (pin * 2));
        pGpio->OSPEED |= (ospeed_val << (pin * 2));
        pGpio->PUPDR &= ~(0x00000003 << (pin * 2));
        pGpio->PUPDR |= (pupd_val << (pin * 2));
    }
}

void bMcuGpioWritePin(bHalGPIOPort_t port, bHalGPIOPin_t pin, uint8_t s)
{
    uint32_t      cs_val = 0x00000001 << pin;
    McuGpioReg_t *pGpio  = (McuGpioReg_t *)(GPIO_REG_BASE + port * GPIO_REG_OFF);
    if (!B_HAL_GPIO_ISVALID(port, pin) || pin == B_HAL_PINAll)
    {
        return;
    }
    if (s == 0)
    {
        cs_val <<= 16;
    }
    pGpio->BSRR = cs_val;
}

uint8_t bMcuGpioReadPin(bHalGPIOPort_t port, bHalGPIOPin_t pin)
{
    uint32_t      id_val = 0;
    McuGpioReg_t *pGpio  = (McuGpioReg_t *)(GPIO_REG_BASE + port * GPIO_REG_OFF);
    if (!B_HAL_GPIO_ISVALID(port, pin) || pin == B_HAL_PINAll)
    {
        return 0;
    }
    id_val = pGpio->IDR;
    return ((id_val & (0x0001 << pin)) != 0);
}

void bMcuGpioWritePort(bHalGPIOPort_t port, uint16_t dat)
{
    McuGpioReg_t *pGpio = (McuGpioReg_t *)(GPIO_REG_BASE + port * GPIO_REG_OFF);
    if (!B_HAL_GPIO_ISVALID(port, 0))
    {
        return;
    }
    pGpio->ODR = dat;
}

uint16_t bMcuGpioReadPort(bHalGPIOPort_t port)
{
    uint32_t      id_val = 0;
    McuGpioReg_t *pGpio  = (McuGpioReg_t *)(GPIO_REG_BASE + port * GPIO_REG_OFF);
    if (!B_HAL_GPIO_ISVALID(port, 0))
    {
        return 0;
    }
    id_val = pGpio->IDR;
    return (id_val & 0xffff);
}
