/*!
    \file    main.c
    \brief   main flash sectors write protection

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
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "gd32f503v_eval.h"

typedef enum {FAILED = 0, PASSED = !FAILED} test_state;
/* 2KB per page in bank0 */
#define FLASH_PAGE_SIZE ((uint16_t)0x800)

/* OB_WP_6 protects page 12 and 13, OB_WP_7 protects page 14 and 15 */
#define FMC_PAGES_PROTECTED (OB_WP_6 | OB_WP_7)

/* Calculate start and end address for page 12~15 (protected by OB_WP_6 and OB_WP_7) */
#define BANK0_WRITE_START_ADDR  ((uint32_t)(0x08000000 + 12 * FLASH_PAGE_SIZE))
#define BANK0_WRITE_END_ADDR    ((uint32_t)(0x08000000 + 16 * FLASH_PAGE_SIZE))

uint32_t erase_counter = 0x0, Address = 0x0;
uint16_t data = 0x1753;
uint32_t wp_value = 0xFFFFFFFF, protected_pages = 0x0;
uint32_t page_num;
__IO fmc_state_enum fmc_state = FMC_READY;
ErrStatus err = ERROR;
__IO test_state program_state = PASSED;

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
    systick_config();
    /* initialize led on the board */
    gd_eval_led_init(LED1);
    gd_eval_led_init(LED2);
    gd_eval_key_init(KEY_USER, KEY_MODE_GPIO);
    
    /* Get pages write protection status */
    wp_value = ob_write_protection_get();
    
    /* Get the number of pages to be erased */
    page_num = (BANK0_WRITE_END_ADDR - BANK0_WRITE_START_ADDR) / FLASH_PAGE_SIZE;

    /* The selected pages are not write protected */
    if(((~wp_value) & FMC_PAGES_PROTECTED) == 0x00) {
        /* Clear All pending flags */
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        
        fmc_unlock();
        /* erase the FLASH pages */
        for(erase_counter = 0; (erase_counter < page_num) && (fmc_state == FMC_READY); erase_counter++) {
            fmc_state = fmc_page_erase(BANK0_WRITE_START_ADDR + (FLASH_PAGE_SIZE * erase_counter));
        }

        /* FLASH Half Word program of data 0x1753 */
        Address = BANK0_WRITE_START_ADDR;

        while((Address < BANK0_WRITE_END_ADDR) && (fmc_state == FMC_READY)) {
            fmc_state = fmc_halfword_program(Address, data);
            Address = Address + 2;
        }

        /* Check the correctness of written data */
        Address = BANK0_WRITE_START_ADDR;

        while((Address < BANK0_WRITE_END_ADDR) && (program_state != FAILED)) {
            if((*(__IO uint16_t *) Address) != data) {
                program_state = FAILED;
            }
            Address += 2;
        }
        fmc_lock();
    } else {
        /* Error to program the flash : The desired pages are write protected */
        program_state = FAILED;
    }
    if(program_state == PASSED) {
        gd_eval_led_on(LED1);
    } else {
        gd_eval_led_on(LED2);
    }

    while(1) {
        if(RESET == gd_eval_key_state_get(KEY_USER)) {
            delay_1ms(50);
            while(RESET == gd_eval_key_state_get(KEY_USER)) {
            }
            /* clear all error flags */
            fmc_flag_clear(FMC_FLAG_BANK0_END);
            fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
            fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
            
            /* Get pages already write protected */
            protected_pages = ~(wp_value) & FMC_PAGES_PROTECTED;
            if(protected_pages == FMC_PAGES_PROTECTED) {
                /* disable pages write protection */
                fmc_unlock();
                ob_unlock();
                err = ob_write_protection_disable(FMC_PAGES_PROTECTED);
                ob_start();
                fmc_state = fmc_bank0_ready_wait(FMC_TIMEOUT_COUNT);
                fmc_nwa_enable();
                fmc_lock();
                ob_lock();
                /* Generate System Reset to load the new option byte values */
                if(FMC_READY == fmc_state) {
                    NVIC_SystemReset();
                }
            } else {
                /* enable pages write protection */
                fmc_unlock();
                ob_unlock();
                err = ob_write_protection_enable(FMC_PAGES_PROTECTED);
                ob_start();
                fmc_state = fmc_bank0_ready_wait(FMC_TIMEOUT_COUNT);
                fmc_nwa_enable();
                fmc_lock();
                ob_lock();
                /* Generate System Reset to load the new option byte values */
                if(FMC_READY == fmc_state) {
                    NVIC_SystemReset();
                }
            }
        }
    }
}
