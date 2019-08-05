/*
 *  linux/arch/arm/kernel/sys_arm.c
 *
 *  Copyright (C) People who wrote linux/arch/i386/kernel/sys_i386.c
 *  Copyright (C) 1995, 1996 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  This file contains various random system calls that
 *  have a non-standard calling sequence on the Linux/arm
 *  platform.
 */
#include <linux/export.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/stat.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/ipc.h>
#include <linux/uaccess.h>
#include <linux/slab.h>


#define DJR
#ifdef DJR
#include <asm/gpio.h>
#include <linux/delay.h>
#define GPIO_TO_PIN(bank, gpio)  (((bank) - 1) * 32 + (gpio))
#endif 

/*
 * Since loff_t is a 64 bit type we avoid a lot of ABI hassle
 * with a different argument ordering.
 */
asmlinkage long sys_arm_fadvise64_64(int fd, int advice,
				     loff_t offset, loff_t len)
{
	return sys_fadvise64_64(fd, offset, len, advice);
}



#ifdef DJR
/***********dsp****************/
#define US_4385 5
#define US  1
#define MS  13
#define uchar unsigned char
#define uint unsigned int
#define DELAY


void DelayUs(uint Us)
{
#ifdef DELAY
        udelay(Us);
#endif
}

#define DLY 1
void DelayNOP(uint n)
{
#ifdef DELAY
        while(n--);
#endif
}

struct gpio_resource {
    int gpio; //gpio num 
    char *name; //name 
};
#if 1
static struct gpio_resource AD2425_table [] = {
    [0] = {
        .gpio = GPIO_TO_PIN(1,4),
        .name = "2425_SDA"
    },
    [1] = {
        .gpio = GPIO_TO_PIN(1,11),
        .name = "2425_SCL"
    },
};
#endif

static struct gpio_resource ADAU1452_table [] = {
    [0] = {
        .gpio = GPIO_TO_PIN(1,3),
        .name = "1452_SDA"
    },
    [1] = {
        .gpio = GPIO_TO_PIN(1,8),
        .name = "1452_SCL"
    },
};

static struct gpio_resource Reset_table [] = {
    [0] = {
        .gpio = GPIO_TO_PIN(1,6),
        .name = "RST1"
    },
    [1] = {
        .gpio = GPIO_TO_PIN(3,29),
        .name = "RST2"
    },
    [2] = {
        .gpio = GPIO_TO_PIN(3,23),
        .name = "mp3-play"
    },
    [3] = {
        .gpio = GPIO_TO_PIN(3,22),
        .name = "mp3-Record"
    },
    [4] = {
        .gpio = GPIO_TO_PIN(3,18),
        .name = "usb-play"
    },
    [5] = {
        .gpio = GPIO_TO_PIN(3,20),
        .name = "usb-Record"
    },
	[6] = {
		.gpio = GPIO_TO_PIN(3,28),
        .name = "MUTE"
    },
};



/***********************************第一片ADAU1452*******************************/
asmlinkage void sys_ADAU1452_IICStart(void)
{
        gpio_set_value(ADAU1452_table[0].gpio, 1);	DelayNOP(DLY);
        gpio_set_value(ADAU1452_table[1].gpio, 1);	DelayNOP(DLY);
        gpio_set_value(ADAU1452_table[0].gpio, 0);	DelayNOP(DLY);
        gpio_set_value(ADAU1452_table[1].gpio, 0);	DelayNOP(DLY);
//        printk("ADAU1452_IICStart\n");

}
void ADAU1452_IICStart1(void)
{
        gpio_set_value(ADAU1452_table[0].gpio, 1);	DelayNOP(DLY);
        gpio_set_value(ADAU1452_table[1].gpio, 1);	DelayNOP(DLY);
        gpio_set_value(ADAU1452_table[0].gpio, 0);	DelayNOP(DLY);
        gpio_set_value(ADAU1452_table[1].gpio, 0);	DelayNOP(DLY);
//    printk("ADAU1452_IICStart1\n");

}
uint ADAU1452_IICACK(void)
{
    uint ACK = 0;

    gpio_set_value(ADAU1452_table[1].gpio, 0);	DelayNOP(DLY);
    gpio_set_value(ADAU1452_table[1].gpio, 1);	DelayNOP(DLY);

    if( gpio_get_value(ADAU1452_table[0].gpio) == 1 )
        {
        ACK = 1;
    }
    DelayNOP(DLY);
    gpio_set_value(ADAU1452_table[1].gpio, 0);
//	printk("ADAU1452_IICACK\n");
    return ACK;
}
void ADAU1452_IICWriteChar(uchar da)
{
        uint i = 8;

        gpio_set_value(ADAU1452_table[1].gpio, 0);
        DelayNOP(DLY);
        while(i--)
        {
                if(da&0x80)
                        gpio_set_value(ADAU1452_table[0].gpio, 1);
                else
                        gpio_set_value(ADAU1452_table[0].gpio, 0);
                DelayNOP(DLY);
                da <<= 1;
                gpio_set_value(ADAU1452_table[1].gpio, 1);	DelayNOP(DLY);
                gpio_set_value(ADAU1452_table[1].gpio, 0);	DelayNOP(DLY);
        }
        gpio_set_value(ADAU1452_table[0].gpio, 0);	DelayNOP(DLY);
//	printk("ADAU1452_IICWriteChar\n");
}
asmlinkage void sys_ADAU1452_IICWrite(uchar da)
{
    uint ACK;
XX:
    ADAU1452_IICWriteChar(da);
    ACK = ADAU1452_IICACK();
    if(ACK)
        {
        DelayUs(US);
        ADAU1452_IICStart1();
        goto XX;
    }
//    printk("ADAU1452_IICWrite\n");
}
asmlinkage void sys_ADAU1452_IICStop(void)
{
    gpio_set_value(ADAU1452_table[0].gpio, 0);	DelayNOP(DLY);
    gpio_set_value(ADAU1452_table[1].gpio, 1);	DelayNOP(DLY);
    gpio_set_value(ADAU1452_table[0].gpio, 1);	DelayNOP(DLY);
    gpio_set_value(ADAU1452_table[1].gpio, 0);	DelayNOP(DLY);
//    printk("ADAU1452_IICStop\n");
}

asmlinkage uchar sys_ADAU1452_IICReadChar(void)
{
    uchar i = 8;
    uchar da= 0;

    DelayNOP(3*DLY);
    gpio_set_value(ADAU1452_table[0].gpio, 1);
    gpio_set_value(ADAU1452_table[1].gpio, 0);	DelayNOP(DLY);
    gpio_direction_input(ADAU1452_table[0].gpio);
    while(i--)
        {
        da <<= 1;
        gpio_set_value(ADAU1452_table[1].gpio, 1);	DelayNOP(4*DLY);
        if(gpio_get_value(ADAU1452_table[0].gpio)==1) da=da|0x01;
                DelayNOP(DLY);
        gpio_set_value(ADAU1452_table[1].gpio, 0);	DelayNOP(DLY);
    }
    gpio_direction_output(ADAU1452_table[0].gpio, 0);
    gpio_set_value(ADAU1452_table[0].gpio, 0);	DelayNOP(2*DLY);
    gpio_set_value(ADAU1452_table[1].gpio, 1);	DelayNOP(2*DLY);
    gpio_set_value(ADAU1452_table[1].gpio, 0);	DelayNOP(2*DLY);
//    printk("ADAU1452_IICReadChar\n");

    return da;
}

/*****************************************Function Select GPIO****************/
asmlinkage void sys_Function_Select_gpio(int fun,int val)
{
    switch(fun)
    {
        case 1: gpio_set_value(Reset_table[0].gpio, val); break;
        case 2: gpio_set_value(Reset_table[1].gpio, val); break;
        case 3: gpio_set_value(Reset_table[2].gpio, val); break;
        case 4: gpio_set_value(Reset_table[3].gpio, val); break;
        case 5: gpio_set_value(Reset_table[4].gpio, val); break;
        case 6: gpio_set_value(Reset_table[5].gpio, val); break;
		case 7: gpio_set_value(Reset_table[6].gpio, val); break;
    }
//        printk("sys_Function_Select_gpio\n");
}

/*****************************************GPIO_init***************************/
asmlinkage int sys_ADAU1452_gpio_init(void)
{

    int i=0,err;
    for(i = 0; i < ARRAY_SIZE(ADAU1452_table); i++) 
    {
        err = gpio_request(ADAU1452_table[i].gpio,ADAU1452_table[i].name); 
        if(err)
			printk("gpio request fail ADAU1452_table[%d]=%s\n",i,ADAU1452_table[i].name);
        
        gpio_direction_output(ADAU1452_table[i].gpio, 1);
//        s3c_gpio_setpull(ADAU1452_table[i].gpio,S3C_GPIO_PULL_NONE);
    }
//        printk("ADAU1452_gpio_init\n");

/*
    for(i = 0; i < ARRAY_SIZE(ADAU1452_table2); i++) 
    {
        err = gpio_request(ADAU1452_table2[i].gpio,ADAU1452_table2[i].name); 
        if(err)
			printk("gpio request fail ADAU1452_table2[%d]=%s\n",i,ADAU1452_table2[i].name);
     
        gpio_direction_output(ADAU1452_table2[i].gpio, 1);
        s3c_gpio_setpull(ADAU1452_table2[i].gpio,S3C_GPIO_PULL_NONE);
        
    }
*/

#if 1
	for(i = 0; i < ARRAY_SIZE(AD2425_table); i++){
		err = gpio_request(AD2425_table[i].gpio,AD2425_table[i].name);
        	if(err)
			printk("gpio request fail AD2425_table[%d]=%s\n",i,AD2425_table[i].name);
		gpio_direction_output(AD2425_table[i].gpio, 1);
	}
#endif
    return 0;
}

asmlinkage int sys_Function_Select_gpio_init(void)
{

    int i=0;
    for(i = 0; i < ARRAY_SIZE(Reset_table); i++) 
    {
        gpio_request(Reset_table[i].gpio,Reset_table[i].name); 
        gpio_direction_output(Reset_table[i].gpio, 1);
    }

//    printk("Function_Select_gpio_init\n");

    return 0;
}


/*
*   AD2425 start
*/

#define Dly 5
#if 1
asmlinkage void sys_AD2425_IICStart(void)
{
	gpio_set_value(AD2425_table[0].gpio, 1);	DelayUs(Dly);
	gpio_set_value(AD2425_table[1].gpio, 1);	DelayUs(Dly);
	gpio_set_value(AD2425_table[0].gpio, 0);	DelayUs(Dly);
	gpio_set_value(AD2425_table[1].gpio, 0);	DelayUs(Dly);
	//        printk("sys_AD2425_IICStart\n");
}

void AD2425_IICStart1(void)
{
        gpio_set_value(AD2425_table[0].gpio, 1);        DelayUs(Dly);
        gpio_set_value(AD2425_table[1].gpio, 1);        DelayUs(Dly);
        gpio_set_value(AD2425_table[0].gpio, 0);        DelayUs(Dly);
        gpio_set_value(AD2425_table[1].gpio, 0);        DelayUs(Dly);
          //      printk("AD2425_IICStart1\n");
}

asmlinkage void sys_AD2425_IICStop(void)
{
	gpio_set_value(AD2425_table[0].gpio, 0);	DelayUs(Dly);
	gpio_set_value(AD2425_table[1].gpio, 1);	DelayUs(Dly);
	gpio_set_value(AD2425_table[0].gpio, 1);	DelayUs(Dly);
	gpio_set_value(AD2425_table[1].gpio, 0);	DelayUs(Dly);
	   // printk("sys_AD2425_IICStop\n");
}

uint AD2425_IICAck(void)
{
    uint ACK = 0;

    gpio_set_value(AD2425_table[1].gpio, 0);  DelayNOP(Dly);
    gpio_set_value(AD2425_table[1].gpio, 1);  DelayNOP(Dly);

    if( gpio_get_value(AD2425_table[0].gpio) == 1 ){
        ACK = 1;
    }
    DelayUs(Dly);
    gpio_set_value(AD2425_table[1].gpio, 0);
//      printk("ADAU1452_IICACK\n");
    return ACK;
}

#if 0
uint AD2425_IICAck(void)
{
	uint ACK = 0;

	gpio_direction_input(AD2425_table[0].gpio);

	gpio_set_value(AD2425_table[1].gpio, 0);	DelayUs(Dly);
	gpio_set_value(AD2425_table[1].gpio, 1);	DelayUs(Dly);

	if( 1 == gpio_get_value(AD2425_table[0].gpio) ){
		ACK = 1;
	}
	DelayUs(Dly);
	gpio_set_value(AD2425_table[1].gpio, 0);
	
	gpio_direction_output(AD2425_table[0].gpio, 0);
		printk("AD2425_IICAck\n");
	return ACK;
}
#endif
void AD2425_IICWirteByte(uchar da)
{
	uint i = 8;

	gpio_set_value(AD2425_table[1].gpio, 0);
	DelayUs(Dly);
	while(i--){
		if(da&0x80)
			gpio_set_value(AD2425_table[0].gpio, 1);
		else
			gpio_set_value(AD2425_table[0].gpio, 0);
		DelayUs(Dly);
		
		gpio_set_value(AD2425_table[1].gpio, 1);	DelayUs(Dly);
		gpio_set_value(AD2425_table[1].gpio, 0);	DelayUs(Dly);
		
		da <<= 1;
	}
	gpio_set_value(AD2425_table[0].gpio, 0);	DelayUs(2*Dly);
	//	printk("AD2425_IICWirteByte\n");
}
asmlinkage void sys_AD2425_IICWrite(uchar da)
{
	uint ACK;
Loop:
	AD2425_IICWirteByte(da);
	ACK = AD2425_IICAck();
	
	if(ACK){
		DelayUs(US);
//		sys_AD2425_IICStart();
		AD2425_IICStart1();
		goto Loop;
	}
	   // printk("sys_AD2425_IICWrite\n");
}


asmlinkage uchar sys_AD2425_IICRead(void)
{
	uchar i = 8;
	uchar da= 0;

	gpio_set_value(AD2425_table[0].gpio, 1);
	gpio_set_value(AD2425_table[1].gpio, 0);
	DelayUs(Dly);
	gpio_direction_input(AD2425_table[0].gpio);
	
	while(i--){
		da <<= 1;
		gpio_set_value(AD2425_table[1].gpio, 1);	
		DelayUs(Dly);
		
		if(gpio_get_value(AD2425_table[0].gpio)==1) 
			da=da|0x01;
		
		DelayUs(Dly);
		
		gpio_set_value(AD2425_table[1].gpio, 0);	
		DelayUs(Dly);
	}
	
	gpio_direction_output(AD2425_table[0].gpio, 0);
	gpio_set_value(AD2425_table[0].gpio, 0);	DelayUs(Dly);
	gpio_set_value(AD2425_table[1].gpio, 1);	DelayUs(Dly);
	gpio_set_value(AD2425_table[1].gpio, 0);	DelayUs(Dly);
	  //  printk("sys_AD2425_IICRead\n");
	
	return da;
}
#endif


/*
*   AD2425 end
*/


static struct gpio_resource CS4385_table [] = {
    [0] = {
        .gpio = GPIO_TO_PIN(2, 10),
        .name = "4385_SDA"
    },
    [1] = {        
        .gpio = GPIO_TO_PIN(2, 11),
        .name = "4385_SCL"
    },
};

/*************************************CS4385********************************/
asmlinkage void sys_CS4385_IICStart(void)
{
#if 0
    gpio_set_value(CS4385_table[0].gpio, 1);
    DelayUs(US_4385);  
    gpio_set_value(CS4385_table[1].gpio, 1);
    DelayUs(US_4385);
    gpio_set_value(CS4385_table[0].gpio, 0);
    DelayUs(US_4385);
    gpio_set_value(CS4385_table[1].gpio, 0);
//    printk("CS4385_IICStart\n");
#endif 

}

void CS4385_IICStart1(void)
{
#if 0
    gpio_set_value(CS4385_table[0].gpio, 1);
    DelayUs(US_4385);  
    gpio_set_value(CS4385_table[1].gpio, 1);
    DelayUs(US_4385);
    gpio_set_value(CS4385_table[0].gpio, 0);
    DelayUs(US_4385);
    gpio_set_value(CS4385_table[1].gpio, 0);
//    printk("CS4385_IICStart1\n");
#endif 
}

uint CS4385_IICACK(void)
{

    uint ACK = 0;
#if 0	
    gpio_set_value(CS4385_table[1].gpio, 0);
    DelayUs(US_4385);
    gpio_set_value(CS4385_table[1].gpio, 1);
    DelayUs(US_4385);

    if( gpio_get_value(CS4385_table[0].gpio) == 1 ){
        ACK = 1;
    }
    DelayUs(US_4385);
    gpio_set_value(CS4385_table[1].gpio, 0);
//    printk("CS4385_IICACK\n");
#endif 
    return ACK;
}

void CS4385_IICWriteChar(uchar da)
{
#if 0
    uint i = 8;

    gpio_set_value(CS4385_table[1].gpio, 0);
    DelayUs(US_4385);
    while(i--){

        if(da&0x80)

            gpio_set_value(CS4385_table[0].gpio, 1);
        else
            gpio_set_value(CS4385_table[0].gpio, 0);
        DelayUs(US_4385);
        da <<= 1;
        gpio_set_value(CS4385_table[1].gpio, 1);
        DelayUs(US_4385);
        gpio_set_value(CS4385_table[1].gpio, 0);
        DelayUs(US_4385);
    }
    gpio_set_value(CS4385_table[0].gpio, 0);
    DelayUs(US_4385);
//    printk("CS4385_IICWriteChar\n");
#endif 
}

asmlinkage void sys_CS4385_IICWrite(uchar da)
{
#if 0
    uint ACK;
XX:
    CS4385_IICWriteChar(da);
    ACK = CS4385_IICACK();
    if(ACK){
        DelayUs(US_4385);
        CS4385_IICStart1();
        goto XX;
    }
//    printk("CS4385_IICWrite\n");
#endif 
}


asmlinkage void sys_CS4385_IICStop(void)
{
#if 0
    gpio_set_value(CS4385_table[0].gpio, 0);
    DelayUs(US_4385);  
    gpio_set_value(CS4385_table[1].gpio, 1);
    DelayUs(US_4385);
    gpio_set_value(CS4385_table[0].gpio, 1);
    DelayUs(US_4385);
    gpio_set_value(CS4385_table[1].gpio, 0);
    DelayUs(US_4385);
//    printk("CS4385_IICStop\n");
#endif 
}

asmlinkage uchar sys_CS4385_IICReadChar(void)
{
	uchar da= 0;
#if 0
    uchar i = 8;
//    DelayUs(US_4385); 
    gpio_direction_input(CS4385_table[0].gpio);

    gpio_set_value(CS4385_table[0].gpio, 1);
    gpio_set_value(CS4385_table[1].gpio, 0);

    DelayUs(US_4385); 
    while(i--){         
        da <<= 1;   
        gpio_set_value(CS4385_table[1].gpio, 1);
        DelayUs(2*US_4385); 
        if(gpio_get_value(CS4385_table[0].gpio)==1) 
            da=da|0x01;
        DelayUs(US_4385);
        gpio_set_value(CS4385_table[1].gpio, 0);
        DelayUs(US_4385); 
    }

    gpio_direction_output(CS4385_table[0].gpio, 0);  

    gpio_set_value(CS4385_table[0].gpio, 0);
    DelayUs(2*US_4385);
    gpio_set_value(CS4385_table[1].gpio, 1);
    DelayUs(2*US_4385);
    gpio_set_value(CS4385_table[1].gpio, 0);
    DelayUs(2*US_4385);
//    printk("CS4385_IICReadChar\n");
#endif 
    return da;
}

asmlinkage int sys_CS4385_gpio_init(void)
{
#if 0
    int i=0;
    for(i = 0; i < ARRAY_SIZE(CS4385_table); i++) 
    {
        gpio_request(CS4385_table[i].gpio,CS4385_table[i].name); 
        gpio_direction_output(CS4385_table[i].gpio, 1);
    }

//    printk("CS4385_gpio_init\n");
#endif 
	printk("sys_CS4385_gpio_init\n");
    return 0;
}



#if 0

static struct gpio_resource BD3201_table [] = {
    [0] = {
        .gpio = GPIO_TO_PIN(2, 7),
        .name = "DAT_3201"
    },
    [1] = {
        .gpio = GPIO_TO_PIN(2, 9),
        .name = "CLK_3201_1"
    },
    [2] = {

        .gpio = GPIO_TO_PIN(2, 14),
        .name = "CLK_3201_2"
    },
};


/*************************************BD3201********************************/
void BD3201CLK(uchar FxUnit)
{
    if(FxUnit==1)
    {
        if( gpio_get_value(BD3201_table[1].gpio) ) gpio_set_value(BD3201_table[1].gpio, 0);
        else gpio_set_value(BD3201_table[1].gpio, 1);
    }
    if(FxUnit==2)
    {
        if( gpio_get_value(BD3201_table[2].gpio) ) gpio_set_value(BD3201_table[2].gpio, 0);
        else gpio_set_value(BD3201_table[2].gpio, 1);
    }
//    printk("BD3201CLK\n");
}

void BD3201SDA(uchar FxUnit,uchar dat)
{
    if(FxUnit==1)gpio_set_value(BD3201_table[0].gpio, dat);
    if(FxUnit==2)gpio_set_value(BD3201_table[0].gpio, dat);
//    if(FxUnit==3)gpio_set_value(BD3201_table[0].gpio, dat);
//    if(FxUnit==4)gpio_set_value(BD3201_table[0].gpio, dat);
//    printk("BD3201SDA\n");
}

asmlinkage void sys_BD3201_Start_Bit(uchar FxUnit)
{
    BD3201CLK(FxUnit);      DelayUs(US);
    BD3201SDA(FxUnit,0);    DelayUs(US);
    BD3201SDA(FxUnit,1);    DelayUs(US);
    BD3201SDA(FxUnit,0);    DelayUs(US);
    BD3201CLK(FxUnit);      DelayUs(US);
    BD3201SDA(FxUnit,1);
    BD3201CLK(FxUnit);      DelayUs(US);
    BD3201SDA(FxUnit,0);
//    printk("BD3201_Start_Bit\n");
}

asmlinkage void sys_BD3201_Stop_Bit(uchar FxUnit)
{
    BD3201SDA(FxUnit,0);    DelayUs(US);
    BD3201SDA(FxUnit,1);    DelayUs(US);
    BD3201SDA(FxUnit,0);    DelayUs(US);
    BD3201SDA(FxUnit,1);
    BD3201CLK(FxUnit);      DelayUs(US);
    BD3201SDA(FxUnit,0);
//    printk("BD3201_Stop_Bit\n");

}

asmlinkage void sys_BD3201_Write_Char(uchar FxUnit,uchar dat)//写一个字节
{
    uchar i;
    for(i = 0; i < 8; i++)
    {
        if(dat&0x80)BD3201SDA(FxUnit,1);
        else BD3201SDA(FxUnit,0);    DelayUs(US);
        BD3201CLK(FxUnit);
        dat <<= 1;      DelayUs(US);
    }
//    printk("BD3201_Write_Char\n");
}






asmlinkage int sys_BD3201_gpio_init(void)
{

    int i=0;
    for(i = 0; i < ARRAY_SIZE(BD3201_table); i++) 
    {
        gpio_request(BD3201_table[i].gpio,BD3201_table[i].name); 
        gpio_direction_output(BD3201_table[i].gpio, 1);
    }

//    printk("BD3201_gpio_init\n");

    return 0;
}

#endif 

#endif







