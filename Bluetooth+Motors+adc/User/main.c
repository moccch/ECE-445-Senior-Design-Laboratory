/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MW579ģ����豸�͹��Ĳ���ʵ��
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� F407���������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/LCD/lcd.h"
//#include "demo.h"
#include "./BSP/ATK_MW579/atk_mw579.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/TIMER/stepper_tim.h"
#include "./BSP/STEPPER_MOTOR/stepper_motor.h"
#include <string.h>

#include "./BSP/RTC/rtc.h"
#include "./USMART/usmart.h"

#define DEMO_BLE_NAME           "ATK-MW579"                         /* �������� */
#define DEMO_BLE_HELLO          "HELLO ATK-MW579"                   /* ������ӭ�� */
#define DEMO_BLE_ADPTIM         5                                   /* �㲥�ٶ� */

/**
 * @brief       ��ʾʵ����Ϣ
 * @param       ��
 * @retval      ��
 */
void show_mesg(void)
{
    /* LCD��ʾʵ����Ϣ */
    lcd_show_string(10, 10, 220, 32, 32, "STM32", RED);
    lcd_show_string(10, 47, 220, 24, 24, "ATK-MW579", RED);
    //lcd_show_string(10, 76, 220, 16, 16, "ATOM@ALIENTEK", RED);
    
    /* �������ʵ����Ϣ */
    printf("\n");
    printf("********************************\r\n");
    printf("STM32\r\n");
    printf("ATK-MW579\r\n");
    //printf("ATOM@ALIENTEK\r\n");
    printf("********************************\r\n");
    printf("\r\n");
}

void bluetooth(void)
{
    uint8_t ret;
    uint8_t key;
    uint8_t *recv_dat;
    
    uint16_t adcx;
    
    uint8_t start_hour, start_min, start_sec, start_ampm;
    uint8_t hour, min, sec, ampm;
    uint8_t year, month, date, week;
    uint8_t tbuf[40];
    
    uint8_t send_flag=0;
    
    float temp;
    float voltage;
    
    uint8_t id = 1;
    uint8_t flag = 0, t = 0;
    uint8_t dir = 1;
    uint16_t set_speed = 100;
    
    uint8_t start_t;
    
    char buf[32];
    

    /* ATK-MW579��ʼ�� */
    ret = atk_mw579_init(ATK_MW579_UART_BAUDRATE_115200);
    if (ret != 0)
    {
        printf("ATK-MW579 init failed!\r\n");
        while (1)
        {
            LED0_TOGGLE();
            delay_ms(200);
        }
    }
    
    /* ����ATK-MW579 */
    atk_mw579_enter_config_mode();
    ret  = atk_mw579_set_name(DEMO_BLE_NAME);
    ret += atk_mw579_set_hello(DEMO_BLE_HELLO);
    ret += atk_mw579_set_tpl(ATK_MW579_TPL_P0DBM);
    ret += atk_mw579_set_uart(ATK_MW579_UART_BAUDRATE_115200, ATK_MW579_UART_DATA_8, ATK_MW579_UART_PARI_NONE, ATK_MW579_UART_STOP_1);
    ret += atk_mw579_set_adptim(DEMO_BLE_ADPTIM);
    ret += atk_mw579_set_linkpassen(ATK_MW579_LINKPASSEN_OFF);
    ret += atk_mw579_set_leden(ATK_MW579_LEDEN_ON);
    ret += atk_mw579_set_slavesleepen(ATK_MW579_SLAVESLEEPEN_ON);
    ret += atk_mw579_set_maxput(ATK_MW579_MAXPUT_OFF);
    ret += atk_mw579_set_mode(ATK_MW579_MODE_S);
    if (ret != 0)
    {
        printf("ATK-MW579 config failed!\r\n");
        while (1)
        {
            LED0_TOGGLE();
            delay_ms(200);
        }
    }
    
    /* ���¿�ʼ�������� */
    printf("Connection Success\r\n");
    atk_mw579_uart_rx_restart();
    
    while (1)
    {

        rtc_get_time(&hour, &min, &sec, &ampm);
        rtc_get_date(&year, &month, &date, &week);
        sprintf((char *)tbuf, "Time:%02d:%02d:%02d", hour, min, sec);
        lcd_show_string(30, 150, 210, 16, 16, (char*)tbuf, RED);
        
        adcx = adc_get_result_average(ADC_ADCX_CHY, 10);                /* ��ȡADCͨ����ת��ֵ��10��ȡƽ�� */
        lcd_show_xnum(134, 110, adcx, 5, 16, 0, BLUE);                  /* ��ʾADC�������ƽ��ֵ */
 
        temp = (float)adcx * (3.3 / 4096);                              /* ��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ������3.1111 */
        voltage = (float)adcx * (3.3 / 4096);
        adcx = temp;                                                    /* ��ֵ�������ָ�adcx��������ΪadcxΪu16���� */
        lcd_show_xnum(134, 130, adcx, 1, 16, 0, BLUE);                  /* ��ʾ��ѹֵ���������֣�3.1111�Ļ������������ʾ3 */
        
        temp -= adcx;                                                   /* ���Ѿ���ʾ����������ȥ��������С�����֣�����3.1111 - 3 = 0.1111 */
        temp *= 1000;                                                   /* С�����ֳ���1000�����磺0.1111��ת��Ϊ111.1���൱�ڱ�����λС�� */
        lcd_show_xnum(150, 130, temp, 3, 16, 0X80, BLUE);               /* ��ʾС�����֣�ǰ��ת��Ϊ��������ʾ����������ʾ�ľ���111 */
        
        if (send_flag)
        {
            atk_mw579_uart_printf("adc:%f\r\n",voltage);
        }

        key = key_scan(0);
        
        switch (key)
        {
            case KEY0_PRES:
            {
                /* ͸���������������豸 */
                atk_mw579_uart_printf("adc:%f\r\n",voltage);
                break;
            }
            case KEY1_PRES:
            {
                /* ͨ�����ڻ���ATK-MW579 */
                atk_mw579_wakeup_by_uart();
                break;
            }
            default:
            {
                break;
            }
        }
        
        /* ͸�������������豸������ */
//        stepper_pwmt_speed(set_speed,ATIM_TIMX_PWM_CH1);
        recv_dat = atk_mw579_uart_rx_get_frame();
        if (recv_dat != NULL)
        {
            printf("%s\r\n", recv_dat);
            
            const char *start = "power";
            if (strncmp((const char*)recv_dat, start, strlen(start)) == 0) {
                flag = !flag;
                if(flag)
                {
//                    stepper_stop(id);
                    send_flag = !send_flag;
                    stepper_pwmt_speed(set_speed+900,ATIM_TIMX_PWM_CH1);
                    stepper_star(id, dir);
                    
                    start_hour = hour;
                    start_min = min;
                    start_sec = sec;
                    printf("start_hour:%d; start_min: %d; start_sec: %d\r\n", start_hour, start_min, start_sec);
                    flag = !flag;
                }
                else
                {
                    printf("sdhfksdjfksjd\r\n");
                    stepper_stop(id);
                }
            }
            
            
            const char *change = "change";
            if(strncmp((const char*)recv_dat, change, strlen(change)) == 0)
            {
                dir = !dir;
                send_flag = !send_flag;
                stepper_star(id, dir);
                stepper_pwmt_speed(set_speed+900,ATIM_TIMX_PWM_CH1);
                printf("hour:%d; min: %d; sec: %d\r\n",hour, min, sec);
                delay_ms(1000*((hour-start_hour)*60*60 + (min-start_min)*60 + (sec - start_sec)));
                stepper_stop(id);
                dir = !dir;
            }
            
            const char *up = "up";
            if(strncmp((const char*)recv_dat, up, strlen(change)) == 0)
            {
                stepper_star(id, 0);
                stepper_pwmt_speed(set_speed+900,ATIM_TIMX_PWM_CH1);
            }
            
            const char *down = "down";
            if(strncmp((const char*)recv_dat, down, strlen(change)) == 0)
            {
                stepper_star(id, 1);
                stepper_pwmt_speed(set_speed+900,ATIM_TIMX_PWM_CH1);
            }
            
            const char *stop = "stop";
            if(strncmp((const char*)recv_dat, stop, strlen(change)) == 0)
            {
                send_flag = 0;
                stepper_stop(id);
            }
            
            atk_mw579_uart_rx_restart();
        }
        
        
        if ((t % 20) == 0)
        {
            LED0_TOGGLE();  /* ÿ200ms,��תһ��LED0 */
        }
        delay_ms(100);
    }
}

int main(void)
{

    HAL_Init();                         /* ��ʼ��HAL�� */
    sys_stm32_clock_init(336, 8, 2, 7); /* ����ʱ��,168Mhz */
    delay_init(168);                    /* ��ʱ��ʼ�� */
    usart_init(115200);                 /* ���ڳ�ʼ��Ϊ115200 */
    usmart_dev.init(84);                    /* ��ʼ��USMART */
    led_init();                         /* ��ʼ��LED */
    key_init();                         /* ��ʼ������ */
    lcd_init();                         /* ��ʼ��LCD */
    show_mesg();                        /* ��ʾʵ����Ϣ */

    
    stepper_init(0xFFFF, 168 - 1);
    



    

    rtc_init();                             /* ��ʼ��RTC */
    rtc_set_wakeup(RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0);  /* ����WAKE UP�ж�, 1�����ж�һ�� */
    
    
    
    adc_init();                             /* ��ʼ��ADC */
    lcd_show_string(30, 67, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 87, 200, 16, 16, "ADC TEST", RED);
    //lcd_show_string(30, 136, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 107, 200, 16, 16, "ADC1_CH3_VAL:", BLUE);
    lcd_show_string(30, 127, 200, 16, 16, "ADC1_CH3_VOL:0.000V", BLUE); /* ���ڹ̶�λ����ʾС���� */
    

    
    bluetooth();



}



