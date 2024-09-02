/**
 ****************************************************************************************************
 * @file        stepper_motor.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-14
 * @brief       步进电机 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 F407电机开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211014
 * 第一次发布
 *
 ****************************************************************************************************
 */
 
#include "./BSP/STEPPER_MOTOR/stepper_motor.h"
#include "./BSP/TIMER/stepper_tim.h"

/**
 * @brief       初始化步进电机相关IO口, 并使能时钟
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void stepper_init(uint16_t arr, uint16_t psc)
{
    GPIO_InitTypeDef gpio_init_struct;

    STEPPER_DIR1_GPIO_CLK_ENABLE();                                 /* DIR1时钟使能 */
    STEPPER_DIR2_GPIO_CLK_ENABLE();                                 /* DIR2时钟使能 */
    STEPPER_DIR3_GPIO_CLK_ENABLE();                                 /* DIR3时钟使能 */
    STEPPER_DIR4_GPIO_CLK_ENABLE();                                 /* DIR4时钟使能 */
            
    STEPPER_EN1_GPIO_CLK_ENABLE();                                  /* EN1时钟使能 */
    STEPPER_EN2_GPIO_CLK_ENABLE();                                  /* EN2时钟使能 */
    STEPPER_EN3_GPIO_CLK_ENABLE();                                  /* EN3时钟使能 */
    STEPPER_EN4_GPIO_CLK_ENABLE();                                  /* EN4时钟使能 */
    

    gpio_init_struct.Pin = STEPPER_DIR1_GPIO_PIN;                   /* DIR1引脚 */
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;                    /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLDOWN;                          /* 下拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_LOW;                   /* 低速 */
    HAL_GPIO_Init(STEPPER_DIR1_GPIO_PORT, &gpio_init_struct);       /* 初始化DIR1引脚 */

    gpio_init_struct.Pin = STEPPER_DIR2_GPIO_PIN;                   /* DIR2引脚 */
    HAL_GPIO_Init(STEPPER_DIR2_GPIO_PORT, &gpio_init_struct);       /* 初始化DIR2引脚 */

    gpio_init_struct.Pin = STEPPER_DIR3_GPIO_PIN;                   /* DIR3引脚 */
    HAL_GPIO_Init(STEPPER_DIR3_GPIO_PORT, &gpio_init_struct);       /* 初始化DIR3引脚 */

    gpio_init_struct.Pin = STEPPER_DIR4_GPIO_PIN;                   /* DIR4引脚 */
    HAL_GPIO_Init(STEPPER_DIR4_GPIO_PORT, &gpio_init_struct);       /* 初始化DIR4引脚 */
    
    /*   脱机引脚初始化   */
    
    gpio_init_struct.Pin = STEPPER_EN1_GPIO_PIN;                    /* EN1引脚 */
    HAL_GPIO_Init(STEPPER_EN1_GPIO_PORT, &gpio_init_struct);        /* 初始化EN1引脚 */
    
    gpio_init_struct.Pin = STEPPER_EN2_GPIO_PIN;                    /* EN2引脚 */
    HAL_GPIO_Init(STEPPER_EN2_GPIO_PORT, &gpio_init_struct);        /* 初始化EN2引脚 */
    
    gpio_init_struct.Pin = STEPPER_EN3_GPIO_PIN;                    /* EN3引脚 */
    HAL_GPIO_Init(STEPPER_EN3_GPIO_PORT, &gpio_init_struct);        /* 初始化EN3引脚 */
    
    gpio_init_struct.Pin = STEPPER_EN4_GPIO_PIN;                    /* EN4引脚 */
    HAL_GPIO_Init(STEPPER_EN4_GPIO_PORT, &gpio_init_struct);        /* 初始化EN4引脚 */
    
    atim_timx_oc_chy_init(arr, psc);                                /* 初始化PUL引脚，以及脉冲模式等 */
}

/**
 * @brief       开启步进电机
 * @param       motor_num: 步进电机接口序号
 * @retval      无
 */
void stepper_star(uint8_t motor_num, uint8_t dir)
{
    switch(motor_num)
    {
        /* 开启对应PWM通道 */
        case STEPPER_MOTOR_1 :
        {
            ST1_DIR(dir);
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM1||g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM2) 
            {
                HAL_TIM_PWM_Start(&g_atimx_handle, ATIM_TIMX_PWM_CH1);       
            }
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_TOGGLE)
            {
                HAL_TIM_OC_Start_IT(&g_atimx_handle, ATIM_TIMX_PWM_CH1);     
            }         
            break;
        }
        case STEPPER_MOTOR_2 :
        {
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM1||g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM2) 
            {
                HAL_TIM_PWM_Start(&g_atimx_handle, ATIM_TIMX_PWM_CH2);       
            }
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_TOGGLE)
            {
                HAL_TIM_OC_Start_IT(&g_atimx_handle, ATIM_TIMX_PWM_CH2);      
            }
            break;
        }
        case STEPPER_MOTOR_3 :
        {
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM1||g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM2) 
            {
                HAL_TIM_PWM_Start(&g_atimx_handle, ATIM_TIMX_PWM_CH3);       
            }
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_TOGGLE)
            {
                HAL_TIM_OC_Start_IT(&g_atimx_handle, ATIM_TIMX_PWM_CH3);     
            }
            break;  
        }
        case STEPPER_MOTOR_4 :
        {
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM1||g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM2) 
            {
                HAL_TIM_PWM_Start(&g_atimx_handle, ATIM_TIMX_PWM_CH4);        
            }
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_TOGGLE)
            {
                HAL_TIM_OC_Start_IT(&g_atimx_handle, ATIM_TIMX_PWM_CH4);      
            }
            break;
        }
        default : break;
    }
}

/**
 * @brief       关闭步进电机
 * @param       motor_num: 步进电机接口序号
 * @retval      无
 */
void stepper_stop(uint8_t motor_num)
{
    switch(motor_num)
    {
        case STEPPER_MOTOR_1 :
        {
            /* 关闭对应PWM通道 */
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM1||g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM2) 
            {
                HAL_TIM_PWM_Stop(&g_atimx_handle, ATIM_TIMX_PWM_CH1);       
            }
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_TOGGLE)
            {
                HAL_TIM_OC_Stop_IT(&g_atimx_handle, ATIM_TIMX_PWM_CH1);     
            }
            break;
        }
        case STEPPER_MOTOR_2 :
        {
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM1||g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM2) 
            {
                HAL_TIM_PWM_Stop(&g_atimx_handle, ATIM_TIMX_PWM_CH2);      
            }
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_TOGGLE)
            {
                HAL_TIM_OC_Stop_IT(&g_atimx_handle, ATIM_TIMX_PWM_CH2);    
            }
            break;
        }
        case STEPPER_MOTOR_3 :
        {
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM1||g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM2) 
            {
                HAL_TIM_PWM_Stop(&g_atimx_handle, ATIM_TIMX_PWM_CH3);      
            }
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_TOGGLE)
            {
                HAL_TIM_OC_Stop_IT(&g_atimx_handle, ATIM_TIMX_PWM_CH3);  
            }
            break;  
        }
        case STEPPER_MOTOR_4 :
        {
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM1||g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_PWM2) 
            {
                HAL_TIM_PWM_Stop(&g_atimx_handle, ATIM_TIMX_PWM_CH4);       
            }
            if(g_atimx_oc_chy_handle.OCMode == TIM_OCMODE_TOGGLE)
            {
                HAL_TIM_OC_Stop_IT(&g_atimx_handle, ATIM_TIMX_PWM_CH4);   
            }
            break;
        }
        default : break;
    }
}

/**
 * @brief       设置电机速度
 * @param       speed  : 设置重装载值大小
 * @param       Channel: 设置定时器通道
 * @retval      无
*/
void stepper_pwmt_speed(uint16_t speed,uint32_t Channel)
{
    __HAL_TIM_SetAutoreload(&g_atimx_handle,speed);
    __HAL_TIM_SetCompare(&g_atimx_handle,Channel,__HAL_TIM_GET_AUTORELOAD(&g_atimx_handle)>>1);
}

