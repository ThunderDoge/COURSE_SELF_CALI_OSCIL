#ifndef _KEY_H
#define _KEY_H

#define KEY0_PORT GPIOE
#define KEY1_PORT GPIOE
#define KEY2_PORT GPIOE
#define WK_UP_PORT GPIOA

#define KEY0_PIN GPIO_Pin_4
#define KEY1_PIN GPIO_Pin_3
#define KEY2_PIN GPIO_Pin_2
#define WK_UP_PIN GPIO_Pin_0

typedef enum{
    KEY0=1,
    KEY1,
    KEY2,
    WKUP
}Key;

#define KEY0_PRES	1
#define KEY1_PRES   2
#define KEY2_PRES	3
#define WKUP_PRES	4

void KEY_Init(void);  //IO初始化
u8 KEY_Scan(u8);    //按键扫描函数
#endif 
