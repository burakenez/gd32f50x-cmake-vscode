/*!
    \file    main.c
    \brief   system clock switch demo

    \version 2026-02-25, V1.0.4, firmware for GD32F50x
*/

/*
    Copyright (c) 2026, GigaDevice Semiconductor Inc.

    All rights reserved.

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
#include <stdio.h>

/* The following is to prevent Vcore fluctuations caused by frequency switching.
   It is strongly recommended to include it to avoid issues caused by self-removal. */
#define RCU_MODIFY_DE_2(__delay)   do{                                    \
                                    volatile uint32_t i, reg;             \
                                    if(0U != (__delay)){                  \
                                        /* Insert a software delay */     \
                                        for(i=0U; i<(__delay); i++){      \
                                        }                                 \
                                        reg = RCU_CFG0;                   \
                                        reg &= ~(RCU_CFG0_AHBPSC);        \
                                        reg |= RCU_AHB_CKSYS_DIV2;        \
                                        /* AHB = SYSCLK/2 */              \
                                        RCU_CFG0 = reg;                   \
                                        /* Insert a software delay */     \
                                        for(i=0U; i<(__delay); i++){      \
                                        }                                 \
                                        reg = RCU_CFG0;                   \
                                        reg &= ~(RCU_CFG0_AHBPSC);        \
                                        reg |= RCU_AHB_CKSYS_DIV4;        \
                                        /* AHB = SYSCLK/4 */              \
                                        RCU_CFG0 = reg;                   \
                                        /* Insert a software delay */     \
                                        for(i=0U; i<(__delay); i++){      \
                                        }                                 \
                                    }                                     \
                                }while(0U)

/* The following is to prevent Vcore fluctuations caused by frequency switching.
   It is strongly recommended to include it to avoid issues caused by self-removal. */
#define RCU_MODIFY_UP_2(__delay)  do{                                     \
                                      volatile uint32_t i,reg;            \
                                      if(0U != (__delay)){                \
                                          for(i=0U; i<(__delay); i++){    \
                                          }                               \
                                          reg = RCU_CFG0;                 \
                                          reg &= ~(RCU_CFG0_AHBPSC);      \
                                          reg |= RCU_AHB_CKSYS_DIV2;      \
                                          RCU_CFG0 = reg;                 \
                                          /* Insert a software delay */   \
                                          for(i=0U; i<(__delay); i++){    \
                                          }                               \
                                          reg = RCU_CFG0;                 \
                                          reg &= ~(RCU_CFG0_AHBPSC);      \
                                          reg |= RCU_AHB_CKSYS_DIV1;      \
                                          RCU_CFG0 = reg;                 \
                                          /* Insert a software delay */   \
                                          for(i=0U; i<(__delay); i++){    \
                                          }                               \
                                      }                                   \
                                  }while(0U)

static void _delay(uint32_t timeout);
static void switch_system_clock_to_108m_hxtal(void);
static void switch_system_clock_to_72m_irc8m(void);
static void _soft_delay_(uint32_t time);

/* software delay to prevent the impact of Vcore fluctuations.
   It is strongly recommended to include it to avoid issues caused by self-removal. */
static void _soft_delay_(uint32_t time)
{
    __IO uint32_t i;
    for(i = 0U; i < time * 10U; i++) {
    }
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    gd_eval_com_init(EVAL_COM0);
    printf("\r\nCK_SYS switch test demo\r\n");

    /* disable the USART */
    usart_disable(EVAL_COM0);

    /* switch system clock to 108MHz by HXTAL */
    switch_system_clock_to_108m_hxtal();
    gd_eval_com_init(EVAL_COM0);

    /* print out the clock frequency of system */
    printf("\r\nCK_SYS is %d", rcu_clock_freq_get(CK_SYS));
    _delay(1000);

    /* switch system clock to 72MHz by IRC8M */
    switch_system_clock_to_72m_irc8m();
    gd_eval_com_init(EVAL_COM0);

    /* print out the clock frequency of system */
    printf("\r\nCK_SYS is %d", rcu_clock_freq_get(CK_SYS));
    while(1) {
    }
}

/*!
    \brief      delay function
    \param[in]  timeout: time out
    \param[out] none
    \retval     none
*/
static void _delay(uint32_t timeout)
{
    __IO uint32_t i, j;
    for(i = 0U; i < timeout; i++) {
        for(j = 0U; j < 500U; j++) {
        }
    }
}

/*!
    \brief      switch system clock to 108M by HXTAL
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void switch_system_clock_to_108m_hxtal(void)
{
    uint32_t timeout = 0U;
    uint32_t stab_flag = 0U;
    __IO uint32_t reg_temp;

    if(((RCU_CFG0 & RCU_CFG0_SCSS) == RCU_SCSS_PLL0P)){
        /* It is strongly recommended to use it to avoid issues caused by self-removal. */
        /* stepwise frequency reduction */
        RCU_MODIFY_DE_2(50U);
    }

    /* select HXTAL as system clock source, deinitialize the RCU */
    rcu_system_clock_source_config(RCU_CKSYSSRC_HXTAL);
    /* It is strongly recommended to include it to avoid issues caused by self-removal. */
    _soft_delay_(200U);
    rcu_deinit();

    /* enable HXTAL */
    RCU_CTL |= RCU_CTL_HXTALEN;

    /* wait until HXTAL is stable or the startup time is longer than HXTAL_STARTUP_TIMEOUT */
    do {
        timeout++;
        stab_flag = (RCU_CTL & RCU_CTL_HXTALSTB);
    } while((0U == stab_flag) && (HXTAL_STARTUP_TIMEOUT != timeout));

    /* if fail */
    if(0U == (RCU_CTL & RCU_CTL_HXTALSTB)) {
        while(1) {
        }
    }

    /* HXTAL is stable */
    /* AHB = SYSCLK/4. AHB clock will be modified to SYSCLK after frequency switching finished to prevent Vcore fluctuations */
    RCU_CFG0 |= RCU_AHB_CKSYS_DIV4;
    /* APB2 = AHB/1 */
    RCU_CFG0 |= RCU_APB2_CKAHB_DIV1;
    /* APB1 = AHB/2 */
    RCU_CFG0 |= RCU_APB1_CKAHB_DIV2;

    /* CK_PLL = (HXTAL)/2 * 27 = 108 MHz */
    RCU_CFG1 &= ~(RCU_CFG1_PLL0SEL | RCU_CFG1_PREDIV0 | RCU_CFG1_PREDIV1);
    RCU_CFG1 |= (RCU_PLL0SRC_HXTAL  |  RCU_PREDIV0_DIV2);
    RCU_CFG0 &= ~(RCU_CFG0_PLL0MF_0_3 | RCU_CFG0_PLL0MF_4_5);
    RCU_CFG0 |= RCU_PLL0_MUL27;

    /* enable PLL */
    RCU_CTL |= RCU_CTL_PLL0EN;

    /* wait until PLL is stable */
    while(0U == (RCU_CTL & RCU_CTL_PLL0STB)) {
    }

    /* select FMC clock is pll0 and div is 1 */
    RCU_ADDCTL &= ~(RCU_ADDCTL_FMCSEL | RCU_ADDCTL_FMCDIV);
    RCU_ADDCTL |=  RCU_FMC_CK_PLL0 | RCU_FMC_DIV1;

    reg_temp = RCU_CFG0;
    /* select PLL0P as system clock */
    reg_temp &= ~RCU_CFG0_SCS;
    reg_temp |= RCU_CKSYSSRC_PLL0P;
    RCU_CFG0 = reg_temp;

    /* wait until PLL is selected as system clock */
    while(RCU_SCSS_PLL0P != (RCU_CFG0 & RCU_CFG0_SCSS)) {
    }

    /* It is strongly recommended to use it to avoid issues caused by self-removal. */
    RCU_MODIFY_UP_2(50U);
}

/*!
    \brief      switch system clock to 72M by IRC8M
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void switch_system_clock_to_72m_irc8m(void)
{
    uint32_t timeout = 0U;
    uint32_t stab_flag = 0U;
    __IO uint32_t reg_temp;

    if(((RCU_CFG0 & RCU_CFG0_SCSS) == RCU_SCSS_PLL0P)){
        /* It is strongly recommended to use it to avoid issues caused by self-removal. */
        /* stepwise frequency reduction */
        RCU_MODIFY_DE_2(50U);
    }

    /* select IRC8M as system clock source, deinitialize the RCU */
    rcu_system_clock_source_config(RCU_CKSYSSRC_IRC8M);
    /* It is strongly recommended to include it to avoid issues caused by self-removal. */
    _soft_delay_(200U);
    rcu_deinit();

    /* enable IRC8M */
    RCU_CTL |= RCU_CTL_IRC8MEN;

    /* wait until IRC8M is stable or the startup time is longer than IRC8M_STARTUP_TIMEOUT */
    do {
        timeout++;
        stab_flag = (RCU_CTL & RCU_CTL_IRC8MSTB);
    } while((0U == stab_flag) && (IRC8M_STARTUP_TIMEOUT != timeout));

    /* if fail */
    if(0U == (RCU_CTL & RCU_CTL_IRC8MSTB)) {
        while(1) {
        }
    }

    /* IRC8M is stable */
    /* AHB = SYSCLK/4. AHB clock will be modified to SYSCLK after frequency switching finished to prevent Vcore fluctuations */
    RCU_CFG0 |= RCU_AHB_CKSYS_DIV4;
    /* APB2 = AHB/1 */
    RCU_CFG0 |= RCU_APB2_CKAHB_DIV1;
    /* APB1 = AHB/2 */
    RCU_CFG0 |= RCU_APB1_CKAHB_DIV2;

    /* CK_PLL = (CK_IRC8M)/2 * 18 = 72 MHz */
    RCU_CFG1 &= ~(RCU_CFG1_PLL0SEL | RCU_CFG1_PREDIV0 | RCU_CFG1_PREDIV1);
    RCU_CFG1 |= (RCU_PLL0SRC_IRC8M  |  RCU_PREDIV0_DIV2);
    RCU_CFG0 &= ~(RCU_CFG0_PLL0MF_0_3 | RCU_CFG0_PLL0MF_4_5);
    RCU_CFG0 |= RCU_PLL0_MUL18;

    /* enable PLL */
    RCU_CTL |= RCU_CTL_PLL0EN;

    /* wait until PLL is stable */
    while(0U == (RCU_CTL & RCU_CTL_PLL0STB)) {
    }


    /* select FMC clock is pll0 and div is 1 */
    RCU_ADDCTL &= ~(RCU_ADDCTL_FMCSEL | RCU_ADDCTL_FMCDIV);
    RCU_ADDCTL |=  RCU_FMC_CK_PLL0 | RCU_FMC_DIV1;

    reg_temp = RCU_CFG0;
    /* select PLL0P as system clock */
    reg_temp &= ~RCU_CFG0_SCS;
    reg_temp |= RCU_CKSYSSRC_PLL0P;
    RCU_CFG0 = reg_temp;

    /* wait until PLL is selected as system clock */
    while(RCU_SCSS_PLL0P != (RCU_CFG0 & RCU_CFG0_SCSS)) {
    }

    /* It is strongly recommended to use it to avoid issues caused by self-removal. */
    RCU_MODIFY_UP_2(50U);
}
