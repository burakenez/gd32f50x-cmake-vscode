/*!
    \file    main.c
    \brief   HASH SHA256 dma example

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

#include "gd32f503v_eval.h"
#include "main.h"
#include <string.h>

/* Hash SHA256 digest computation */
static void hash_sha256_dma(void);
/* printf data in words */
static void data_display(uint32_t datalength);
/* check data in bytes */
static void data_check(uint8_t *src, uint8_t *dst, uint32_t len);

uint32_t i = 0, len = 0;
__IO uint8_t check_flag = 0;
hau_digest_parameter_struct digest_para;

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

int main(void)
{
    /************************************************************/
    /* Enabling the parameter checking feature of the firmware  */
    /* will reduce code execution efficiency. It is recommended */
    /* that the parameter checking feature be used only during  */
    /* the code development phase.                              */
    /************************************************************/
#ifdef __FIRMWARE_VERSION_DEFINE
    uint32_t fw_ver = 0;
#endif

    gd_eval_led_init(LED1);
    gd_eval_com_init(EVAL_COM0);
    /* enable HAU clock */
    rcu_periph_clock_enable(RCU_HAU);
    /* Enable DMA1 clock */
    rcu_periph_clock_enable(RCU_DMA1);
    rcu_periph_clock_enable(RCU_DMAMUX);

#ifdef __FIRMWARE_VERSION_DEFINE
    fw_ver = gd32f50x_firmware_version_get();
    /* print firmware version */
    printf("\r\ngd32f50x series firmware version: V%d.%d.%d", (uint8_t)(fw_ver >> 24), (uint8_t)(fw_ver >> 16), (uint8_t)(fw_ver >> 8));
#endif /* __FIRMWARE_VERSION_DEFINE */

    len = sizeof(message_input);  
    printf("\r\nmessage to be input:\r\n\r\n");
    for(i = 0; i < len ; i++){
        printf("%c", message_input[i]);
    }

    /* SHA-256 digest computation */
    hash_sha256_dma();
    printf("  \r\n\r\nSHA256 message digest (256 bits):\r\n\r\n");
    data_display(32U);
    data_check((uint8_t *)&digest_para.out[0], (uint8_t *)&expected_data_sha256[0], 32U);

    if(0U == check_flag){
        gd_eval_led_on(LED1);
        printf("\r\nSHA256 digest computation success!\r\n");
    }

    while(1){
    }
}

/*!
    \brief      Hash SHA256 digest computation
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void hash_sha256_dma(void)
{
    dma_parameter_struct dma_init_struct;
    __IO uint16_t num_last_valid = 0;
    __IO uint32_t meg_burst = 0; 

    /* HAU configuration */
    hau_deinit();
    hau_init(HAU_SWAPPING_8BIT);

    if(len % 4){
        meg_burst++;
    }
    meg_burst += (len/4);
    num_last_valid = 8 * (len % 4);
    /* configure the number of valid bits in last word of the message */
    hau_last_word_validbits_num_config(num_last_valid);
      
    /* DMA configuration */
    dma_deinit(DMA1, DMA_CH1);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.request       = DMA_REQUEST_HAU_IN;
    dma_init_struct.periph_addr   = (uint32_t)(&HAU_DI);
    dma_init_struct.periph_width  = DMA_PERIPHERAL_WIDTH_32BIT;
    dma_init_struct.periph_inc    = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_addr   = (uint32_t)message_input;
    dma_init_struct.memory_width  = DMA_MEMORY_WIDTH_32BIT;
    dma_init_struct.memory_inc    = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.direction     = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.number        = meg_burst;
    dma_init_struct.priority      = DMA_PRIORITY_ULTRA_HIGH;
    dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    dma_init(DMA1,DMA_CH1, &dma_init_struct);
    /* enable DMA channel */
    dma_channel_enable(DMA1, DMA_CH1);
    
    hau_dma_enable();

    /* wait until the last transfer from OUT FIFO */
    while(!dma_flag_get(DMA1, DMA_CH1, DMA_FLAG_FTF)){
    }

    /* wait until the busy flag is reset */
    while(hau_flag_get(HAU_FLAG_BUSY)){
    }
    
    hau_digest_read(&digest_para);

    /* change formatting */
    digest_para.out[0] = __REV(digest_para.out[0]);
    digest_para.out[1] = __REV(digest_para.out[1]);
    digest_para.out[2] = __REV(digest_para.out[2]);
    digest_para.out[3] = __REV(digest_para.out[3]);
    digest_para.out[4] = __REV(digest_para.out[4]);
    digest_para.out[5] = __REV(digest_para.out[5]);
    digest_para.out[6] = __REV(digest_para.out[6]);
    digest_para.out[7] = __REV(digest_para.out[7]);
}

/*!
    \brief      printf data in words
    \param[in]  datalength: length of the data to display
    \param[out] none
    \retval     none
*/
static void data_display(uint32_t datalength)
{
    uint32_t i =0U;
    uint32_t count = 0U;
    uint8_t *temp = (uint8_t *)&digest_para.out;

    for(i = 0; i < datalength; i++){
        printf(" 0x%02X", (uint8_t)temp[i]);
        count++;

        if(4U == count){
            count = 0U;
            printf("\r\n");
        }
    }
}

/*!
    \brief      check data in bytes
    \param[in]  src: source
    \param[in]  dst: destination
    \param[in]  len: length of the data to check
    \param[out] none
    \retval     none
*/
static void data_check(uint8_t *src, uint8_t *dst, uint32_t len)
{
    if(memcmp((const void*)src, (const void*)dst, len) != 0){
        check_flag++;
    }
}
