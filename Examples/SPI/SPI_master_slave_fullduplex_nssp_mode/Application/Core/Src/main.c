/*!
    \file    main.c
    \brief   SPI master and slave fullduplex communication with nssp mode

    \version 2026-02-25, V1.0.4, firmware for GD32F50x
*/

/*
    Copyright (c) 2026, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f50x.h"
#include "gd32f503v_eval.h"

#define arraysize         10

uint8_t spi0_send_array[arraysize] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA};
uint8_t spi2_send_array[arraysize] = {0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA};
uint8_t spi0_receive_array[arraysize];
uint8_t spi2_receive_array[arraysize];
ErrStatus memory_compare(uint8_t *src, uint8_t *dst, uint8_t length);

void rcu_config(void);
void gpio_config(void);
void dma_config(void);
void spi_config(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* initialize led2 and led3 */
    gd_eval_led_init(LED2);
    gd_eval_led_init(LED3);

    /* enable peripheral clocks */
    rcu_config();
    /* configure GPIO peripheral */
    gpio_config();
    /* configure DMA peripheral */
    dma_config();
    /* configure SPI peripheral */
    spi_config();

    /* enable SPI NSSP mode */
    spi_nssp_mode_enable(SPI0);
    spi_nss_output_enable(SPI0);

    /* enable SPI */
    spi_enable(SPI2);
    spi_enable(SPI0);

    /* enable DMA channel */
    dma_channel_enable(DMA0, DMA_CH1);
    dma_channel_enable(DMA0, DMA_CH2);
    dma_channel_enable(DMA1, DMA_CH0);
    dma_channel_enable(DMA1, DMA_CH1);

    /* enable SPI DMA */
    spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);
    spi_dma_enable(SPI2, SPI_DMA_RECEIVE);
    spi_dma_enable(SPI0, SPI_DMA_TRANSMIT);
    spi_dma_enable(SPI0, SPI_DMA_RECEIVE);

    /* wait for DMA transmit to complete */
    while(!dma_flag_get(DMA0, DMA_CH2, DMA_FLAG_FTF)) {
    }
    while(!dma_flag_get(DMA1, DMA_CH1, DMA_FLAG_FTF)) {
    }
    while(!dma_flag_get(DMA1, DMA_CH0, DMA_FLAG_FTF)) {
    }
    while(!dma_flag_get(DMA0, DMA_CH1, DMA_FLAG_FTF)) {
    }

    /* compare received data with sent data */
    if(memory_compare(spi2_receive_array, spi0_send_array, arraysize)) {
        gd_eval_led_on(LED2);
    } else {
        gd_eval_led_off(LED2);
    }
    if(memory_compare(spi0_receive_array, spi2_send_array, arraysize)) {
        gd_eval_led_on(LED3);
    } else {
        gd_eval_led_off(LED3);
    }

    while(1) {
    }
}

/*!
    \brief      configure different peripheral clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable GPIOA clock */ 
    rcu_periph_clock_enable(RCU_GPIOA); 
    /* enable GPIOC clock */ 
    rcu_periph_clock_enable(RCU_GPIOC); 
    /* enable DMA0 clock */ 
    rcu_periph_clock_enable(RCU_DMA0); 
    /* enable DMA1 clock */ 
    rcu_periph_clock_enable(RCU_DMA1); 
    /* enable SPI0 clock */ 
    rcu_periph_clock_enable(RCU_SPI0); 
    /* enable SPI2 clock */ 
    rcu_periph_clock_enable(RCU_SPI2); 
    /* enable DMAMUX clock */ 
    rcu_periph_clock_enable(RCU_DMAMUX); 
    /* enable AF clock */ 
    rcu_periph_clock_enable(RCU_AF); 
}

/*!
    \brief      configure the GPIO peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    /* configure SPI0 GPIO: SCK/PA5, MISO/PA6, MOSI/PA7, NSS/PA15 */
    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_LEVEL2, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_15);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_LEVEL2, GPIO_PIN_15);

    /* configure SPI2 GPIO: NSS/PA4, SCK/PC10, MISO/PC11, MOSI/PC12 */
    gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_4);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_LEVEL2, GPIO_PIN_4);

    gpio_af_set(GPIOC, GPIO_AF_2, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_LEVEL2, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
}

/*!
    \brief      configure dma peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void dma_config(void)
{
    dma_parameter_struct dma_init_struct;

    /* configure SPI0 transmit dma: DMA0, DMA_CH2 */
    dma_deinit(DMA0, DMA_CH2);
    dma_init_struct.request      = DMA_REQUEST_SPI0_TX;
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI0);
    dma_init_struct.memory_addr  = (uint32_t)spi0_send_array;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority     = DMA_PRIORITY_LOW;
    dma_init_struct.number       = arraysize;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init(DMA0, DMA_CH2, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH2);
    dma_memory_to_memory_disable(DMA0, DMA_CH2);

    /* configure SPI0 receive dma: DMA0, DMA_CH1 */
    dma_deinit(DMA0, DMA_CH1);
    dma_init_struct.request      = DMA_REQUEST_SPI0_RX;
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI0);
    dma_init_struct.memory_addr  = (uint32_t)spi0_receive_array;
    dma_init_struct.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA0, DMA_CH1, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH1);
    dma_memory_to_memory_disable(DMA0, DMA_CH1);

    /* configure SPI2 transmit dma: DMA1,DMA_CH1 */
    dma_deinit(DMA1, DMA_CH1);
    dma_init_struct.request      = DMA_REQUEST_SPI2_TX;
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);
    dma_init_struct.memory_addr  = (uint32_t)spi2_send_array;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.priority     = DMA_PRIORITY_MEDIUM;
    dma_init(DMA1, DMA_CH1, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA1, DMA_CH1);
    dma_memory_to_memory_disable(DMA1, DMA_CH1);

    /* configure SPI2 receive dma: DMA1,DMA_CH0 */
    dma_deinit(DMA1, DMA_CH0);
    dma_init_struct.request      = DMA_REQUEST_SPI2_RX;
    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);
    dma_init_struct.memory_addr  = (uint32_t)spi2_receive_array;
    dma_init_struct.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA1, DMA_CH0, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA1, DMA_CH0);
    dma_memory_to_memory_disable(DMA1, DMA_CH0);
}

/*!
    \brief      configure spi peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_config(void)
{
    spi_parameter_struct spi_init_struct;

    /* configure SPI0 parameters */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_HARD;
    spi_init_struct.prescale             = SPI_PSC_256;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);

    /* configure SPI2 parameters */
    spi_init_struct.device_mode = SPI_SLAVE;
    spi_init_struct.nss         = SPI_NSS_HARD;
    spi_init(SPI2, &spi_init_struct);
}

/*!
    \brief      memory compare function
    \param[in]  src: source data pointer
    \param[in]  dst: destination data pointer
    \param[in]  length: the compare data length
    \param[out] none
    \retval     ErrStatus : ERROR or SUCCESS
*/
ErrStatus memory_compare(uint8_t *src, uint8_t *dst, uint8_t length)
{
    while(length--) {
        if(*src++ != *dst++) {
            return ERROR;
        }
    }
    return SUCCESS;
}
