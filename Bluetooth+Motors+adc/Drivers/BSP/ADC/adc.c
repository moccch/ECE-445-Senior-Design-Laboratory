/**
 ****************************************************************************************************
 * @file        adc.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-18
 * @brief       ADC ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� F407���������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com/forum.php
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20211018
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/ADC/adc.h"
#include "./SYSTEM/delay/delay.h"


ADC_HandleTypeDef g_adc_handle;   /* ADC��� */

/**
 * @brief       ADC��ʼ������
 * @param       ��
 * @retval      ��
 */
void adc_init(void)
{
    g_adc_handle.Instance = ADC_ADCX;
    g_adc_handle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;            /* 4��Ƶ��21Mhz */
    g_adc_handle.Init.Resolution = ADC_RESOLUTION_12B;                          /* 12λģʽ */
    g_adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;                          /* �Ҷ��� */
    g_adc_handle.Init.ScanConvMode = DISABLE;                                   /* ��ɨ��ģʽ */
    g_adc_handle.Init.ContinuousConvMode = DISABLE;                             /* �ر�����ת�� */
    g_adc_handle.Init.NbrOfConversion = 1;                                      /* ��ʵ��ֻʹ�õ�һ���������� */
    g_adc_handle.Init.DiscontinuousConvMode = DISABLE;                          /* ��ֹ����������ģʽ */
    g_adc_handle.Init.NbrOfDiscConversion = 0;                                  /* ����������ͨ����Ϊ0 */
    g_adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;                    /* ������� */
    g_adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;     /* ʹ��������� */
    g_adc_handle.Init.DMAContinuousRequests = DISABLE;                          /* �ر�DMA���� */
    HAL_ADC_Init(&g_adc_handle);                                                /* ��ʼ��ADC */
}

/**
 * @brief       ADC�ײ��������������ã�ʱ��ʹ��
                �˺����ᱻHAL_ADC_Init()����
 * @param       hadc:ADC���
 * @retval      ��
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    if(hadc->Instance == ADC_ADCX)
    {
        GPIO_InitTypeDef gpio_init_struct;
        
        ADC_ADCX_CHY_CLK_ENABLE();                                                  /* ʹ��ADCxʱ�� */
        ADC_ADCX_CHY_GPIO_CLK_ENABLE();                                             /* ����GPIOʱ�� */

        gpio_init_struct.Pin = ADC_ADCX_CHY_GPIO_PIN;                               /* ADC�ɼ���ӦIO */
        gpio_init_struct.Mode = GPIO_MODE_ANALOG;                                   /* ģ������ */
        gpio_init_struct.Pull = GPIO_PULLUP;                                        /* ���� */
        HAL_GPIO_Init(ADC_ADCX_CHY_GPIO_PORT, &gpio_init_struct);                   /* ��ʼ��IO */
    }
}

/**
 * @brief       ADCͨ������
 * @param       adcx : adc���ָ��,ADC_HandleTypeDef
 * @param       ch   : ͨ����, ADC_CHANNEL_0~ADC_CHANNEL_18
 * @param       stime: ����ʱ��  0~7, ��Ӧ��ϵΪ:
 *   @arg       ADC_SAMPLETIME_3CYCLES,  3��ADCʱ������        ADC_SAMPLETIME_15CYCLES, 15��ADCʱ������
 *   @arg       ADC_SAMPLETIME_28CYCLES, 28��ADCʱ������       ADC_SAMPLETIME_56CYCLES, 56��ADCʱ������
 *   @arg       ADC_SAMPLETIME_84CYCLES, 84��ADCʱ������       ADC_SAMPLETIME_112CYCLES,112��ADCʱ������
 *   @arg       ADC_SAMPLETIME_144CYCLES,144��ADCʱ������      ADC_SAMPLETIME_480CYCLES,480��ADCʱ������
 * @param       rank: ��ͨ���ɼ�ʱ��Ҫ���õĲɼ����,
                �����㶨��channle1��rank=1��channle2 ��rank=2��
                ��ô��Ӧ����DMA����ռ�ı�������AdcDMA[0] ��i��channle1��ת�������AdcDMA[1]����ͨ��2��ת������� 
                ��ͨ��DMA����Ϊ ADC_REGULAR_RANK_1
 *   @arg       ���1~16��ADC_REGULAR_RANK_1~ADC_REGULAR_RANK_16
 * @retval      ��
 */
void adc_channel_set(ADC_HandleTypeDef *adc_handle, uint32_t ch, uint32_t rank, uint32_t stime)
{
    ADC_ChannelConfTypeDef adc_channel;                                 /* ���ö�ӦADCͨ�� */

    adc_channel.Channel = ch;                                           /* ADCͨ�� */
    adc_channel.Rank = rank;                                            /* ���ò������� */
    adc_channel.SamplingTime = stime;                                   /* ���ò���ʱ�� */
    HAL_ADC_ConfigChannel( adc_handle, &adc_channel);   
}

/**
 * @brief       ��ȡADCֵ
 * @param       ch: ͨ��ֵ 0~18��ȡֵ��ΧΪ��ADC_CHANNEL_0~ADC_CHANNEL_18
 * @retval      ��
 */
uint32_t adc_get_result(uint32_t ch)
{
    adc_channel_set(&g_adc_handle , ch, 1, ADC_SAMPLETIME_480CYCLES);   /* ����ͨ�������кͲ���ʱ�� */

    HAL_ADC_Start(&g_adc_handle);                                       /* ����ADC */

    HAL_ADC_PollForConversion(&g_adc_handle, 10);                       /* ��ѯת�� */

    return (uint16_t)HAL_ADC_GetValue(&g_adc_handle);                   /* �������һ��ADC1�������ת����� */
}

/**
 * @brief       ����ADC��ƽ��ֵ���˲���
 * @param       ch      : ͨ����, 0~18
 * @param       times   : �ۼƴ���
 * @retval      ͨ��chת������ۼ�times�ε�ƽ��ֵ
 */
uint32_t adc_get_result_average(uint32_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++)                 /* ��ȡtimes������ */
    {
        temp_val += adc_get_result(ch);         /* �ۼ� */
        delay_ms(5);
    }
    return temp_val / times;                    /* ����ƽ��ֵ */
}









