/**
 ****************************************************************************************************
 * @file        adc.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-18
 * @brief       ADC 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 F407电机开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com/forum.php
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211018
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/ADC/adc.h"
#include "./SYSTEM/delay/delay.h"


ADC_HandleTypeDef g_adc_handle;   /* ADC句柄 */

/**
 * @brief       ADC初始化函数
 * @param       无
 * @retval      无
 */
void adc_init(void)
{
    g_adc_handle.Instance = ADC_ADCX;
    g_adc_handle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;            /* 4分频，21Mhz */
    g_adc_handle.Init.Resolution = ADC_RESOLUTION_12B;                          /* 12位模式 */
    g_adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;                          /* 右对齐 */
    g_adc_handle.Init.ScanConvMode = DISABLE;                                   /* 非扫描模式 */
    g_adc_handle.Init.ContinuousConvMode = DISABLE;                             /* 关闭连续转换 */
    g_adc_handle.Init.NbrOfConversion = 1;                                      /* 本实验只使用到一个规则序列 */
    g_adc_handle.Init.DiscontinuousConvMode = DISABLE;                          /* 禁止不连续采样模式 */
    g_adc_handle.Init.NbrOfDiscConversion = 0;                                  /* 不连续采样通道数为0 */
    g_adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;                    /* 软件触发 */
    g_adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;     /* 使用软件触发 */
    g_adc_handle.Init.DMAContinuousRequests = DISABLE;                          /* 关闭DMA请求 */
    HAL_ADC_Init(&g_adc_handle);                                                /* 初始化ADC */
}

/**
 * @brief       ADC底层驱动，引脚配置，时钟使能
                此函数会被HAL_ADC_Init()调用
 * @param       hadc:ADC句柄
 * @retval      无
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    if(hadc->Instance == ADC_ADCX)
    {
        GPIO_InitTypeDef gpio_init_struct;
        
        ADC_ADCX_CHY_CLK_ENABLE();                                                  /* 使能ADCx时钟 */
        ADC_ADCX_CHY_GPIO_CLK_ENABLE();                                             /* 开启GPIO时钟 */

        gpio_init_struct.Pin = ADC_ADCX_CHY_GPIO_PIN;                               /* ADC采集对应IO */
        gpio_init_struct.Mode = GPIO_MODE_ANALOG;                                   /* 模拟输入 */
        gpio_init_struct.Pull = GPIO_PULLUP;                                        /* 上拉 */
        HAL_GPIO_Init(ADC_ADCX_CHY_GPIO_PORT, &gpio_init_struct);                   /* 初始化IO */
    }
}

/**
 * @brief       ADC通道设置
 * @param       adcx : adc句柄指针,ADC_HandleTypeDef
 * @param       ch   : 通道号, ADC_CHANNEL_0~ADC_CHANNEL_18
 * @param       stime: 采样时间  0~7, 对应关系为:
 *   @arg       ADC_SAMPLETIME_3CYCLES,  3个ADC时钟周期        ADC_SAMPLETIME_15CYCLES, 15个ADC时钟周期
 *   @arg       ADC_SAMPLETIME_28CYCLES, 28个ADC时钟周期       ADC_SAMPLETIME_56CYCLES, 56个ADC时钟周期
 *   @arg       ADC_SAMPLETIME_84CYCLES, 84个ADC时钟周期       ADC_SAMPLETIME_112CYCLES,112个ADC时钟周期
 *   @arg       ADC_SAMPLETIME_144CYCLES,144个ADC时钟周期      ADC_SAMPLETIME_480CYCLES,480个ADC时钟周期
 * @param       rank: 多通道采集时需要设置的采集编号,
                假设你定义channle1的rank=1，channle2 的rank=2，
                那么对应你在DMA缓存空间的变量数组AdcDMA[0] 就i是channle1的转换结果，AdcDMA[1]就是通道2的转换结果。 
                单通道DMA设置为 ADC_REGULAR_RANK_1
 *   @arg       编号1~16：ADC_REGULAR_RANK_1~ADC_REGULAR_RANK_16
 * @retval      无
 */
void adc_channel_set(ADC_HandleTypeDef *adc_handle, uint32_t ch, uint32_t rank, uint32_t stime)
{
    ADC_ChannelConfTypeDef adc_channel;                                 /* 配置对应ADC通道 */

    adc_channel.Channel = ch;                                           /* ADC通道 */
    adc_channel.Rank = rank;                                            /* 设置采样序列 */
    adc_channel.SamplingTime = stime;                                   /* 设置采样时间 */
    HAL_ADC_ConfigChannel( adc_handle, &adc_channel);   
}

/**
 * @brief       获取ADC值
 * @param       ch: 通道值 0~18，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_18
 * @retval      无
 */
uint32_t adc_get_result(uint32_t ch)
{
    adc_channel_set(&g_adc_handle , ch, 1, ADC_SAMPLETIME_480CYCLES);   /* 设置通道，序列和采样时间 */

    HAL_ADC_Start(&g_adc_handle);                                       /* 开启ADC */

    HAL_ADC_PollForConversion(&g_adc_handle, 10);                       /* 轮询转换 */

    return (uint16_t)HAL_ADC_GetValue(&g_adc_handle);                   /* 返回最近一次ADC1规则组的转换结果 */
}

/**
 * @brief       计算ADC的平均值（滤波）
 * @param       ch      : 通道号, 0~18
 * @param       times   : 累计次数
 * @retval      通道ch转换结果累加times次的平均值
 */
uint32_t adc_get_result_average(uint32_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++)                 /* 获取times次数据 */
    {
        temp_val += adc_get_result(ch);         /* 累加 */
        delay_ms(5);
    }
    return temp_val / times;                    /* 返回平均值 */
}









