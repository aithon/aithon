#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for Aithon board.
 */

/*
 * Board identifier.
 */
#define BOARD_AITHON
#define BOARD_NAME            "Aithon"

/*
 * Board frequencies.
 * NOTE: Aithon has no LSE.
 */
#define STM32_LSECLK          0
#define STM32_HSECLK          8000000

/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD             330

/*
 * MCU type as defined in the ST header.
 */
#define STM32F427_437xx

/*
 * IO pins assignments.
 */
#define GPIOA_ANALOG3            0
#define GPIOA_ANALOG2            1
#define GPIOA_ANALOG1            2
#define GPIOA_ANALOG0            3
#define GPIOA_BUZZER             4
#define GPIOA_SPI1_SCK           5
#define GPIOA_SPI1_MISO          6
#define GPIOA_SPI1_MOSI          7
#define GPIOA_LCD_DATA           8
#define GPIOA_USB_DEV_5V         9
#define GPIOA_LCD_CLK            10
#define GPIOA_USB_DEV_DM         11
#define GPIOA_USB_DEV_DP         12
#define GPIOA_SWDIO              13
#define GPIOA_SWCLK              14
#define GPIOA_SPI1_NSS           15

#define GPIOB_ANALOG9            0
#define GPIOB_ANALOG8            1
#define GPIOB_BOOT1              2
#define GPIOB_DIGITAL12          3
#define GPIOB_DIGITAL11          4
#define GPIOB_DIGITAL10          5
#define GPIOB_USART1_TX          6
#define GPIOB_USART1_RX          7
#define GPIOB_I2C1_SCL           8
#define GPIOB_I2C1_SDA           9
#define GPIOB_I2C2_SCL_USART3_TX 10
#define GPIOB_I2C2_SDA_USART3_RX 11
#define GPIOB_USB_HOST_FAULT     12
#define GPIOB_USB_HOST_EN        13
#define GPIOB_USB_HOST_DM        14
#define GPIOB_USB_HOST_DP        15


#define GPIOC_ANALOG7            0
#define GPIOC_ANALOG6            1
#define GPIOC_ANALOG5            2
#define GPIOC_ANALOG4            3
#define GPIOC_BAT_SENSE          4
#define GPIOC_SERVO_SENSE        5
#define GPIOC_SERVO4             6
#define GPIOC_SERVO5             7
#define GPIOC_SERVO6             8
#define GPIOC_SERVO7             9
#define GPIOC_SPI3_SCK           10
#define GPIOC_SPI3_MISO          11
#define GPIOC_SPI3_MOSI          12
#define GPIOC_DIGITAL2           13
#define GPIOC_DIGITAL1           14
#define GPIOC_DIGITAL0           15

#define GPIOD_LCD_E              0
#define GPIOD_LCD_RS             1
#define GPIOD_SPI3_NSS           2
#define GPIOD_DIGITAL14          3
#define GPIOD_LED1               4
#define GPIOD_USART2_TX          5
#define GPIOD_USART2_RX          6
#define GPIOD_DIGITAL13          7
#define GPIOD_MOTOR0_FAULT       8
#define GPIOD_MOTOR1_FAULT       9
#define GPIOD_BTN0               10
#define GPIOD_BTN1               11
#define GPIOD_SERVO0             12
#define GPIOD_SERVO1             13
#define GPIOD_SERVO2             14
#define GPIOD_SERVO3             15

#define GPIOE_DIGITAL9           0
#define GPIOE_DIGITAL8           1
#define GPIOE_DIGITAL7           2
#define GPIOE_DIGITAL6           3
#define GPIOE_DIGITAL5           4
#define GPIOE_DIGITAL4           5
#define GPIOE_DIGITAL3           6
#define GPIOE_MOTOR0_A           7
#define GPIOE_MOTOR0_B           8
#define GPIOE_MOTOR0_EN          9
#define GPIOE_MOTOR1_A           10
#define GPIOE_MOTOR1_EN          11
#define GPIOE_MOTOR1_B           12
#define GPIOE_DIGITAL15          13
#define GPIOE_DIGITAL16          14
#define GPIOE_LED0               15

#define GPIOH_OSC_IN             0
#define GPIOH_OSC_OUT            1

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_2M(n)            (0U << ((n) * 2))
#define PIN_OSPEED_25M(n)           (1U << ((n) * 2))
#define PIN_OSPEED_50M(n)           (2U << ((n) * 2))
#define PIN_OSPEED_100M(n)          (3U << ((n) * 2))
#define PIN_PUDR_FLOATING(n)        (0U << ((n) * 2))
#define PIN_PUDR_PULLUP(n)          (1U << ((n) * 2))
#define PIN_PUDR_PULLDOWN(n)        (2U << ((n) * 2))
#define PIN_AFIO_AF(n, v)           ((v##U) << ((n % 8) * 4))

/*
 * Port A setup:
 * PA0  - GPIOA_ANALOG3             (analog input)
 * PA1  - GPIOA_ANALOG2             (analog input)
 * PA2  - GPIOA_ANALOG1             (analog input)
 * PA3  - GPIOA_ANALOG0             (analog input)
 * PA4  - GPIOA_BUZZER              (output pulldown)
 * PA5  - GPIOA_SPI1_SCK            (alternate function 5)
 * PA6  - GPIOA_SPI1_MISO           (alternate function 5)
 * PA7  - GPIOA_SPI1_MOSI           (alternate function 5)
 * PA8  - GPIOA_LCD_DATA            (output pushpull)
 * PA9  - GPIOA_USB_DEV_5V          (input pulldown)
 * PA10 - GPIOA_LCD_CLK             (output pushpull)
 * PA11 - GPIOA_USB_DEV_DM          (alternate function 10)
 * PA12 - GPIOA_USB_DEV_DP          (alternate function 10)
 * PA13 - GPIOA_SWDIO               (alternate function 0 / pulldown)
 * PA14 - GPIOA_SWCLK               (alternate function 0 / pullup)
 * PA15 - GPIOA_SPI1_NSS            (alternate function 5)
 */
#define VAL_GPIOA_MODER    (PIN_MODE_ANALOG(GPIOA_ANALOG3) | \
                            PIN_MODE_ANALOG(GPIOA_ANALOG2) | \
                            PIN_MODE_ANALOG(GPIOA_ANALOG1) | \
                            PIN_MODE_ANALOG(GPIOA_ANALOG0) | \
                            PIN_MODE_ALTERNATE(GPIOA_BUZZER) | \
                            PIN_MODE_ALTERNATE(GPIOA_SPI1_SCK) | \
                            PIN_MODE_ALTERNATE(GPIOA_SPI1_MISO) | \
                            PIN_MODE_ALTERNATE(GPIOA_SPI1_MOSI) | \
                            PIN_MODE_OUTPUT(GPIOA_LCD_DATA) | \
                            PIN_MODE_INPUT(GPIOA_USB_DEV_5V) | \
                            PIN_MODE_OUTPUT(GPIOA_LCD_CLK) | \
                            PIN_MODE_ALTERNATE(GPIOA_USB_DEV_DM) | \
                            PIN_MODE_ALTERNATE(GPIOA_USB_DEV_DP) | \
                            PIN_MODE_ALTERNATE(GPIOA_SWDIO) | \
                            PIN_MODE_ALTERNATE(GPIOA_SWCLK) | \
                            PIN_MODE_ALTERNATE(GPIOA_SPI1_NSS))
#define VAL_GPIOA_OTYPER   0x00000000
#define VAL_GPIOA_OSPEEDR  0xFFFFFFFF
#define VAL_GPIOA_PUPDR    (PIN_PUDR_PULLDOWN(GPIOA_BUZZER) | \
                            PIN_PUDR_PULLUP(GPIOA_USB_DEV_5V) | \
                            PIN_PUDR_PULLUP(GPIOA_SWDIO) | \
                            PIN_PUDR_PULLDOWN(GPIOA_SWCLK))
#define VAL_GPIOA_ODR      0xFFFFFFFF
#define VAL_GPIOA_AFRL     (PIN_AFIO_AF(GPIOA_SPI1_SCK, 5) | \
                            PIN_AFIO_AF(GPIOA_SPI1_MISO, 5) | \
                            PIN_AFIO_AF(GPIOA_SPI1_MOSI, 5))
#define VAL_GPIOA_AFRH     (PIN_AFIO_AF(GPIOA_USB_DEV_DM, 10) | \
                            PIN_AFIO_AF(GPIOA_USB_DEV_DP, 10) | \
                            PIN_AFIO_AF(GPIOA_SWDIO, 0) | \
                            PIN_AFIO_AF(GPIOA_SWCLK, 0) | \
                            PIN_AFIO_AF(GPIOA_SPI1_NSS, 5))


/*
 * Port B setup:
 * PB0  - GPIOB_ANALOG9             (analog input)
 * PB1  - GPIOB_ANALOG8             (analog input)
 * PB2  - GPIOB_BOOT1               (input floating)
 * PB3  - GPIOB_DIGITAL12           (input floating)
 * PB4  - GPIOB_DIGITAL11           (input floating)
 * PB5  - GPIOB_DIGITAL10           (input floating)
 * PB6  - GPIOB_USART1_TX           (alternate function 7)
 * PB7  - GPIOB_USART1_RX           (alternate function 7)
 * PB8  - GPIOB_I2C1_SCL            (alternate function 4)
 * PB9  - GPIOB_I2C1_SDA            (alternate function 4)
 * PB10 - GPIOB_I2C2_SCL_USART3_TX  (alternate function 7)
 * PB11 - GPIOB_I2C2_SDA_USART3_RX  (alternate function 7)
 * PB12 - GPIOB_USB_HOST_FAULT      (input pullup)
 * PB13 - GPIOB_USB_HOST_EN         (alternate function 10)
 * PB14 - GPIOB_USB_HOST_DM         (alternate function 10)
 * PB15 - GPIOB_USB_HOST_DP         (alternate function 10)
 */
#define VAL_GPIOB_MODER    (PIN_MODE_ANALOG(GPIOB_ANALOG9) | \
                            PIN_MODE_ANALOG(GPIOB_ANALOG8) | \
                            PIN_MODE_INPUT(GPIOB_BOOT1) | \
                            PIN_MODE_INPUT(GPIOB_DIGITAL12) | \
                            PIN_MODE_INPUT(GPIOB_DIGITAL11) | \
                            PIN_MODE_INPUT(GPIOB_DIGITAL10) | \
                            PIN_MODE_ALTERNATE(GPIOB_USART1_TX) | \
                            PIN_MODE_ALTERNATE(GPIOB_USART1_RX) | \
                            PIN_MODE_ALTERNATE(GPIOB_I2C1_SCL) | \
                            PIN_MODE_ALTERNATE(GPIOB_I2C1_SDA) | \
                            PIN_MODE_ALTERNATE(GPIOB_I2C2_SCL_USART3_TX) | \
                            PIN_MODE_ALTERNATE(GPIOB_I2C2_SDA_USART3_RX) | \
                            PIN_MODE_INPUT(GPIOB_USB_HOST_FAULT) | \
                            PIN_MODE_OUTPUT(GPIOB_USB_HOST_EN) | \
                            PIN_MODE_ALTERNATE(GPIOB_USB_HOST_DM) | \
                            PIN_MODE_ALTERNATE(GPIOB_USB_HOST_DP))
#define VAL_GPIOB_OTYPER   0x00000000
#define VAL_GPIOB_OSPEEDR  0xFFFFFFFF
#define VAL_GPIOB_PUPDR    (PIN_PUDR_PULLUP(GPIOB_USB_HOST_FAULT))
#define VAL_GPIOB_ODR      0xFFFFFFFF
#define VAL_GPIOB_AFRL     (PIN_AFIO_AF(GPIOB_USART1_TX, 7) | \
                            PIN_AFIO_AF(GPIOB_USART1_RX, 7))
#define VAL_GPIOB_AFRH     (PIN_AFIO_AF(GPIOB_I2C1_SCL, 4) | \
                            PIN_AFIO_AF(GPIOB_I2C1_SDA, 4) | \
                            PIN_AFIO_AF(GPIOB_I2C2_SCL_USART3_TX, 7) | \
                            PIN_AFIO_AF(GPIOB_I2C2_SDA_USART3_RX, 7) | \
                            PIN_AFIO_AF(GPIOB_USB_HOST_DM, 10) | \
                            PIN_AFIO_AF(GPIOB_USB_HOST_DP, 10))

/*
 * Port C setup:
 * PC0  - GPIOC_ANALOG7             (analog input)
 * PC1  - GPIOC_ANALOG6             (analog input)
 * PC2  - GPIOC_ANALOG5             (analog input)
 * PC3  - GPIOC_ANALOG4             (analog input)
 * PC4  - GPIOC_BAT_SENSE           (analog input)
 * PC5  - GPIOC_SERVO_SENSE         (analog input)
 * PC6  - GPIOC_SERVO4              (alternate function 3)
 * PC7  - GPIOC_SERVO5              (alternate function 3)
 * PC8  - GPIOC_SERVO6              (alternate function 3)
 * PC9  - GPIOC_SERVO7              (alternate function 3)
 * PC10 - GPIOC_SPI3_SCK            (alternate function 6)
 * PC11 - GPIOC_SPI3_MISO           (alternate function 6)
 * PC12 - GPIOC_SPI3_MOSI           (alternate function 6)
 * PC13 - GPIOC_DIGITAL2            (input floating)
 * PC14 - GPIOC_DIGITAL1            (input floating)
 * PC15 - GPIOC_DIGITAL0            (input floating)
 */
#define VAL_GPIOC_MODER    (PIN_MODE_ANALOG(GPIOC_ANALOG7) | \
                            PIN_MODE_ANALOG(GPIOC_ANALOG6) | \
                            PIN_MODE_ANALOG(GPIOC_ANALOG5) | \
                            PIN_MODE_ANALOG(GPIOC_ANALOG4) | \
                            PIN_MODE_ANALOG(GPIOC_BAT_SENSE) | \
                            PIN_MODE_ANALOG(GPIOC_SERVO_SENSE) | \
                            PIN_MODE_ALTERNATE(GPIOC_SERVO4) | \
                            PIN_MODE_ALTERNATE(GPIOC_SERVO5) | \
                            PIN_MODE_ALTERNATE(GPIOC_SERVO6) | \
                            PIN_MODE_ALTERNATE(GPIOC_SERVO7) | \
                            PIN_MODE_ALTERNATE(GPIOC_SPI3_SCK) | \
                            PIN_MODE_ALTERNATE(GPIOC_SPI3_MISO) | \
                            PIN_MODE_ALTERNATE(GPIOC_SPI3_MOSI) | \
                            PIN_MODE_INPUT(GPIOC_DIGITAL2) | \
                            PIN_MODE_INPUT(GPIOC_DIGITAL1) | \
                            PIN_MODE_INPUT(GPIOC_DIGITAL0))
#define VAL_GPIOC_OTYPER   0x00000000
#define VAL_GPIOC_OSPEEDR  0x03FFFFFF
#define VAL_GPIOC_PUPDR    0x00000000
#define VAL_GPIOC_ODR      0xFFFFFFFF
#define VAL_GPIOC_AFRL     (PIN_AFIO_AF(GPIOC_SERVO4, 3) |\
                            PIN_AFIO_AF(GPIOC_SERVO5, 3))
#define VAL_GPIOC_AFRH     (PIN_AFIO_AF(GPIOC_SERVO6, 3) | \
                            PIN_AFIO_AF(GPIOC_SERVO7, 3) | \
                            PIN_AFIO_AF(GPIOC_SPI3_SCK, 6) | \
                            PIN_AFIO_AF(GPIOC_SPI3_MISO, 6) | \
                            PIN_AFIO_AF(GPIOC_SPI3_MOSI, 6))

/*
 * Port D setup:
 * PD0  - GPIOD_LCD_E               (output pushpull)
 * PD1  - GPIOD_LCD_RS              (output pushpull)
 * PD2  - GPIOD_SPI3_NSS            (output pushpull)
 * PD3  - GPIOD_DIGITAL14           (input floating)
 * PD4  - GPIOD_LED1                (output pushpull)
 * PD5  - GPIOD_USART2_TX           (alternate function 7)
 * PD6  - GPIOD_USART2_RX           (alternate function 7)
 * PD7  - GPIOD_DIGITAL13           (input floating)
 * PD8  - GPIOD_MOTOR0_FAULT        (input pullup)
 * PD9  - GPIOD_MOTOR1_FAULT        (input pullup)
 * PD10 - GPIOD_BTN0                (input pullup)
 * PD11 - GPIOD_BTN1                (input pullup)
 * PD12 - GPIOD_SERVO0              (alternate function 2)
 * PD13 - GPIOD_SERVO1              (alternate function 2)
 * PD14 - GPIOD_SERVO2              (alternate function 2)
 * PD15 - GPIOD_SERVO3              (alternate function 2)
 */
#define VAL_GPIOD_MODER    (PIN_MODE_OUTPUT(GPIOD_LCD_E) | \
                            PIN_MODE_OUTPUT(GPIOD_LCD_RS) | \
                            PIN_MODE_OUTPUT(GPIOD_SPI3_NSS) | \
                            PIN_MODE_INPUT(GPIOD_DIGITAL14) | \
                            PIN_MODE_OUTPUT(GPIOD_LED1) | \
                            PIN_MODE_ALTERNATE(GPIOD_USART2_TX) | \
                            PIN_MODE_ALTERNATE(GPIOD_USART2_RX) | \
                            PIN_MODE_INPUT(GPIOD_DIGITAL13) | \
                            PIN_MODE_INPUT(GPIOD_MOTOR0_FAULT) | \
                            PIN_MODE_INPUT(GPIOD_MOTOR1_FAULT) | \
                            PIN_MODE_INPUT(GPIOD_BTN0) | \
                            PIN_MODE_INPUT(GPIOD_BTN1) | \
                            PIN_MODE_ALTERNATE(GPIOD_SERVO0) | \
                            PIN_MODE_ALTERNATE(GPIOD_SERVO1) | \
                            PIN_MODE_ALTERNATE(GPIOD_SERVO2) | \
                            PIN_MODE_ALTERNATE(GPIOD_SERVO3))
#define VAL_GPIOD_OTYPER   0x00000000
#define VAL_GPIOD_OSPEEDR  0xFFFFFFFF
#define VAL_GPIOD_PUPDR    (PIN_PUDR_PULLUP(GPIOD_MOTOR0_FAULT) | \
                            PIN_PUDR_PULLUP(GPIOD_MOTOR1_FAULT) | \
                            PIN_PUDR_PULLUP(GPIOD_BTN0) | \
                            PIN_PUDR_PULLUP(GPIOD_BTN1))
#define VAL_GPIOD_ODR      0xFFFFFFFF
#define VAL_GPIOD_AFRL     (PIN_AFIO_AF(GPIOD_USART2_TX, 7) | \
                            PIN_AFIO_AF(GPIOD_USART2_RX, 7))
#define VAL_GPIOD_AFRH     (PIN_AFIO_AF(GPIOD_SERVO0, 2) | \
                            PIN_AFIO_AF(GPIOD_SERVO1, 2) | \
                            PIN_AFIO_AF(GPIOD_SERVO2, 2) | \
                            PIN_AFIO_AF(GPIOD_SERVO3, 2))

/*
 * Port E setup.
 * PE0  - GPIOE_DIGITAL9            (input floating)
 * PE1  - GPIOE_DIGITAL8            (input floating)
 * PE2  - GPIOE_DIGITAL7            (input floating)
 * PE3  - GPIOE_DIGITAL6            (input floating)
 * PE4  - GPIOE_DIGITAL5            (input floating)
 * PE5  - GPIOE_DIGITAL4            (input floating)
 * PE6  - GPIOE_DIGITAL3            (input floating)
 * PE7  - GPIOE_MOTOR0_A            (output push-pull)
 * PE8  - GPIOE_MOTOR0_B            (output push-pull)
 * PE9  - GPIOE_MOTOR0_EN           (alternate function 1)
 * PE10 - GPIOE_MOTOR1_A            (output push-pull)
 * PE11 - GPIOE_MOTOR1_EN           (alternate function 1)
 * PE12 - GPIOE_MOTOR1_B            (output push-pull)
 * PE13 - GPIOE_DIGITAL15           (input floating)
 * PE14 - GPIOE_DIGITAL16           (input floating)
 * PE15 - GPIOE_LED0                (output push-pull)
 */
#define VAL_GPIOE_MODER    (PIN_MODE_INPUT(GPIOE_DIGITAL9) | \
                            PIN_MODE_INPUT(GPIOE_DIGITAL8) | \
                            PIN_MODE_INPUT(GPIOE_DIGITAL7) | \
                            PIN_MODE_INPUT(GPIOE_DIGITAL6) | \
                            PIN_MODE_INPUT(GPIOE_DIGITAL5) | \
                            PIN_MODE_INPUT(GPIOE_DIGITAL4) | \
                            PIN_MODE_INPUT(GPIOE_DIGITAL3) | \
                            PIN_MODE_INPUT(GPIOE_DIGITAL3) | \
                            PIN_MODE_OUTPUT(GPIOE_MOTOR0_A) | \
                            PIN_MODE_OUTPUT(GPIOE_MOTOR0_B) | \
                            PIN_MODE_ALTERNATE(GPIOE_MOTOR0_EN) | \
                            PIN_MODE_OUTPUT(GPIOE_MOTOR1_A) | \
                            PIN_MODE_ALTERNATE(GPIOE_MOTOR1_EN) | \
                            PIN_MODE_OUTPUT(GPIOE_MOTOR1_B) | \
                            PIN_MODE_INPUT(GPIOE_DIGITAL15) | \
                            PIN_MODE_INPUT(GPIOE_DIGITAL16) | \
                            PIN_MODE_OUTPUT(GPIOE_LED0))
#define VAL_GPIOE_OTYPER   0x00000000
#define VAL_GPIOE_OSPEEDR  0xFFFFFFFF
#define VAL_GPIOE_PUPDR    0x00000000
#define VAL_GPIOE_ODR      0xFFFFFFFF
#define VAL_GPIOE_AFRL     0x00000000
#define VAL_GPIOE_AFRH     (PIN_AFIO_AF(GPIOE_MOTOR0_EN, 1) | \
                            PIN_AFIO_AF(GPIOE_MOTOR1_EN, 1))

/*
 * Port H setup.
 * All input with pull-up except:
 * PH0  - GPIOH_OSC_IN          (input floating).
 * PH1  - GPIOH_OSC_OUT         (input floating).
 */
#define VAL_GPIOH_MODER     (PIN_MODE_INPUT(GPIOH_OSC_IN) | \
                             PIN_MODE_INPUT(GPIOH_OSC_OUT))
#define VAL_GPIOH_OTYPER    0x00000000
#define VAL_GPIOH_OSPEEDR   0xFFFFFFFF
#define VAL_GPIOH_PUPDR     (PIN_PUDR_FLOATING(GPIOH_OSC_IN) | \
                             PIN_PUDR_FLOATING(GPIOH_OSC_OUT))
#define VAL_GPIOH_ODR       0xFFFFFFFF
#define VAL_GPIOH_AFRL      0x00000000
#define VAL_GPIOH_AFRH      0x00000000

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
