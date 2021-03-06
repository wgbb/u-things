#include <libc.h>  
#include "esp8266.h"
#include "systick.h"

#define true    (1)
#define false   (0)
#define macNVIC_PriorityGroup_x NVIC_PriorityGroup_2

static void                   ESP8266_GPIO_Config                 ( void );
static void                   ESP8266_USART_Config                ( void );
static void                   ESP8266_USART_NVIC_Configuration    ( void );



struct  STRUCT_USARTx_Fram strEsp8266_Fram_Record = { 0 };


static char * __itoa( int value, char *string, int radix );

void USART_printf ( USART_TypeDef * USARTx, char * Data, ... )
{
    const char *s;
    int d;   
    char buf[16];


    va_list ap;
    va_start(ap, Data);

    while ( * Data != 0 )     // 真真真真真真
    {				                          
        if ( * Data == 0x5c )  //'\'
        {									  
            switch ( *++Data )
            {
                case 'r':							          //真�
                    USART_SendData(USARTx, 0x0d);
                    Data ++;
                    break;

                case 'n':							          //真�
                    USART_SendData(USARTx, 0x0a);	
                    Data ++;
                    break;

                default:
                    Data ++;
                    break;
            }			 
        }

        else if ( * Data == '%')
        {									  //
            switch ( *++Data )
            {				
                case 's':										  //真�
                    s = va_arg(ap, const char *);

                    for ( ; *s; s++) 
                    {
                        USART_SendData(USARTx,*s);
                        while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
                    }

                    Data++;

                    break;

                case 'd':			
                    //真�
                    d = va_arg(ap, int);

                    __itoa(d, buf, 10);

                    for (s = buf; *s; s++) 
                    {
                        USART_SendData(USARTx,*s);
                        while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
                    }

                    Data++;

                    break;

                default:
                    Data++;

                    break;

            }		 
        }

        else USART_SendData(USARTx, *Data++);

        while ( USART_GetFlagStatus ( USARTx, USART_FLAG_TXE ) == RESET );

    }
}


/*
 * 真真itoa
 * 真  真真真真真真
 * 真  �-radix =10 真10真真真真0
 *         -value 真真真�
 *         -buf 真真真�
 *         -radix = 10
 * 真  真
 * 真  真
 * 真  真USART2_printf()真
 */
static char * __itoa( int value, char *string, int radix )
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;

    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */


/**
 * @brief  ESP8266兜兵晒痕方
 * @param  涙
 * @retval 涙
 */
void ESP8266_Init ( void )
{
    ESP8266_GPIO_Config (); 

    ESP8266_USART_Config (); 


    macESP8266_RST_HIGH_LEVEL();

    macESP8266_CH_DISABLE();


}


/**
 * @brief  兜兵晒ESP8266喘欺議GPIO哈重
 * @param  涙
 * @retval 涙
 */
static void ESP8266_GPIO_Config ( void )
{
    /*協吶匯倖GPIO_InitTypeDef窃侏議潤更悶*/
    GPIO_InitTypeDef GPIO_InitStructure;


    /* 塘崔 CH_PD 哈重*/
    macESP8266_CH_PD_APBxClock_FUN ( macESP8266_CH_PD_CLK, ENABLE ); 

    GPIO_InitStructure.GPIO_Pin = macESP8266_CH_PD_PIN;	

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

    GPIO_Init ( macESP8266_CH_PD_PORT, & GPIO_InitStructure );	 


    /* 塘崔 RST 哈重*/
    macESP8266_RST_APBxClock_FUN ( macESP8266_RST_CLK, ENABLE ); 

    GPIO_InitStructure.GPIO_Pin = macESP8266_RST_PIN;	

    GPIO_Init ( macESP8266_RST_PORT, & GPIO_InitStructure );	 


}


/**
 * @brief  兜兵晒ESP8266喘欺議 USART
 * @param  涙
 * @retval 涙
 */
static void ESP8266_USART_Config ( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;


    /* config USART clock */
    macESP8266_USART_APBxClock_FUN ( macESP8266_USART_CLK, ENABLE );
    macESP8266_USART_GPIO_APBxClock_FUN ( macESP8266_USART_GPIO_CLK, ENABLE );

    /* USART GPIO config */
    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =  macESP8266_USART_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(macESP8266_USART_TX_PORT, &GPIO_InitStructure);  

    /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = macESP8266_USART_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(macESP8266_USART_RX_PORT, &GPIO_InitStructure);

    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = macESP8266_USART_BAUD_RATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(macESP8266_USARTx, &USART_InitStructure);


    /* 嶄僅塘崔 */
    USART_ITConfig ( macESP8266_USARTx, USART_IT_RXNE, ENABLE ); //聞嬬堪笥俊辺嶄僅 
    USART_ITConfig ( macESP8266_USARTx, USART_IT_IDLE, ENABLE ); //聞嬬堪笥悳�濘嬾儘亢� 	

    ESP8266_USART_NVIC_Configuration ();


    USART_Cmd(macESP8266_USARTx, ENABLE);


}


/**
 * @brief  塘崔 ESP8266 USART 議 NVIC 嶄僅
 * @param  涙
 * @retval 涙
 */
static void ESP8266_USART_NVIC_Configuration ( void )
{
    NVIC_InitTypeDef NVIC_InitStructure; 


    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig ( macNVIC_PriorityGroup_x );

    /* Enable the USART2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = macESP8266_USART_IRQ;	 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

}


/*
 * 痕方兆��ESP8266_Rst
 * 宙峰  �砦愼�WF-ESP8266庁翠
 * 補秘  �採�
 * 卦指  : 涙
 * 距喘  �艮� ESP8266_AT_Test 距喘
 */
void ESP8266_Rst ( void )
{
#if 0
    ESP8266_Cmd ( "AT+RST", "OK", "ready", 2500 );   	

#else
    macESP8266_RST_LOW_LEVEL();
    mdelay( 500 ); 
    macESP8266_RST_HIGH_LEVEL();

#endif

}


/*
 * 痕方兆��ESP8266_Cmd
 * 宙峰  �唆�WF-ESP8266庁翠窟僕AT峺綜
 * 補秘  ��cmd��棋窟僕議峺綜
 *         reply1��reply2��豚棋議�贄Γ�葎NULL燕音俶�贄Γ�曾宀葎賜貸辞購狼
 *         waittime��吉棋�贄Φ鎚閏�
 * 卦指  : 1��峺綜窟僕撹孔
 *         0��峺綜窟僕払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_Cmd ( char * cmd, char * reply1, char * reply2, u32 waittime )
{    
    strEsp8266_Fram_Record .InfBit .FramLength = 0;               //貫仟蝕兵俊辺仟議方象淫

    macESP8266_Usart ( "%s\r\n", cmd );

    if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //音俶勣俊辺方象
        return true;

    mdelay( waittime );                 //决扮

    strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ]  = '\0';

    macPC_Usart ( "%s", strEsp8266_Fram_Record .Data_RX_BUF );

    if ( ( reply1 != 0 ) && ( reply2 != 0 ) )
        return ( ( uint8_t ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) || 
                ( uint8_t ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) ); 

    else if ( reply1 != 0 )
        return ( ( uint8_t ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) );

    else
        return ( ( uint8_t ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) );

}


/*
 * 痕方兆��ESP8266_AT_Test
 * 宙峰  �唆�WF-ESP8266庁翠序佩AT霞編尼強
 * 補秘  �採�
 * 卦指  : 涙
 * 距喘  �艮四皺慎�喘
 */
//void ESP8266_AT_Test ( void )
//{
//	macESP8266_RST_HIGH_LEVEL();
//	
//	Delay_ms ( 1000 ); 
//	
//	while ( ! ESP8266_Cmd ( "AT", "OK", NULL, 200 ) ) ESP8266_Rst ();  	

//}
void ESP8266_AT_Test ( void )
{
    char count=0;

    macESP8266_RST_HIGH_LEVEL();
    mdelay ( 1000 );
    while ( count < 10 )
    {
        if( ESP8266_Cmd ( "AT", "OK", NULL, 500 ) ) return;
        ESP8266_Rst();
        ++ count;
    }
}


/*
 * 痕方兆��ESP8266_Net_Mode_Choose
 * 宙峰  �済《�WF-ESP8266庁翠議垢恬庁塀
 * 補秘  ��enumMode��垢恬庁塀
 * 卦指  : 1��僉夲撹孔
 *         0��僉夲払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_Net_Mode_Choose ( ENUM_Net_ModeTypeDef enumMode )
{
    switch ( enumMode )
    {
        case STA:
            return ESP8266_Cmd ( "AT+CWMODE=1", "OK", "no change", 2500 ); 

        case AP:
            return ESP8266_Cmd ( "AT+CWMODE=2", "OK", "no change", 2500 ); 

        case STA_AP:
            return ESP8266_Cmd ( "AT+CWMODE=3", "OK", "no change", 2500 ); 

        default:
            return false;
    }

}


/*
 * 痕方兆��ESP8266_JoinAP
 * 宙峰  ��WF-ESP8266庁翠銭俊翌何WiFi
 * 補秘  ��pSSID��WiFi兆各忖憲堪
 *       ��pPassWord��WiFi畜鷹忖憲堪
 * 卦指  : 1��銭俊撹孔
 *         0��銭俊払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_JoinAP ( char * pSSID, char * pPassWord )
{
    char cCmd [120];

    snprintf ( cCmd, sizeof(cCmd), "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );

    return ESP8266_Cmd ( cCmd, "OK", NULL, 5000 );

}


/*
 * 痕方兆��ESP8266_BuildAP
 * 宙峰  ��WF-ESP8266庁翠幹秀WiFi犯泣
 * 補秘  ��pSSID��WiFi兆各忖憲堪
 *       ��pPassWord��WiFi畜鷹忖憲堪
 *       ��enunPsdMode��WiFi紗畜圭塀旗催忖憲堪
 * 卦指  : 1��幹秀撹孔
 *         0��幹秀払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_BuildAP ( char * pSSID, char * pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode )
{
    char cCmd [120];

    snprintf ( cCmd, sizeof(cCmd), "AT+CWSAP=\"%s\",\"%s\",1,%d", pSSID, pPassWord, enunPsdMode );

    return ESP8266_Cmd ( cCmd, "OK", 0, 1000 );

}


/*
 * 痕方兆��ESP8266_Enable_MultipleId
 * 宙峰  ��WF-ESP8266庁翠尼強謹銭俊
 * 補秘  ��enumEnUnvarnishTx��塘崔頁倦謹銭俊
 * 卦指  : 1��塘崔撹孔
 *         0��塘崔払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_Enable_MultipleId ( FunctionalState enumEnUnvarnishTx )
{
    char cStr [20];

    snprintf ( cStr, sizeof(cStr), "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );

    return ESP8266_Cmd ( cStr, "OK", 0, 500 );

}


/*
 * 痕方兆��ESP8266_Link_Server
 * 宙峰  ��WF-ESP8266庁翠銭俊翌何捲暦匂
 * 補秘  ��enumE��利大亅咏
 *       ��ip��捲暦匂IP忖憲堪
 *       ��ComNum��捲暦匂極笥忖憲堪
 *       ��id��庁翠銭俊捲暦匂議ID
 * 卦指  : 1��銭俊撹孔
 *         0��銭俊払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_Link_Server ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
    char cStr [100] = { 0 }, cCmd [120];

    switch (  enumE )
    {
        case enumTCP:
            snprintf ( cStr, sizeof(cStr), "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
            break;

        case enumUDP:
            snprintf ( cStr, sizeof(cStr), "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
            break;

        default:
            break;
    }

    if ( id < 5 )
        snprintf ( cCmd, sizeof(cCmd), "AT+CIPSTART=%d,%s", id, cStr);

    else
        snprintf ( cCmd, sizeof(cCmd), "AT+CIPSTART=%s", cStr );

    return ESP8266_Cmd ( cCmd, "OK", "ALREAY CONNECT", 4000 );

}


/*
 * 痕方兆��ESP8266_StartOrShutServer
 * 宙峰  ��WF-ESP8266庁翠蝕尼賜購液捲暦匂庁塀
 * 補秘  ��enumMode��蝕尼/購液
 *       ��pPortNum��捲暦匂極笥催忖憲堪
 *       ��pTimeOver��捲暦匂階扮扮寂忖憲堪��汽了�挫�
 * 卦指  : 1��荷恬撹孔
 *         0��荷恬払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_StartOrShutServer ( FunctionalState enumMode, char * pPortNum, char * pTimeOver )
{
    char cCmd1 [120], cCmd2 [120];

    if ( enumMode )
    {
        snprintf ( cCmd1, sizeof(cCmd1), "AT+CIPSERVER=%d,%s", 1, pPortNum );

        snprintf ( cCmd2, sizeof(cCmd2), "AT+CIPSTO=%s", pTimeOver );

        return ( ESP8266_Cmd ( cCmd1, "OK", 0, 500 ) &&
                ESP8266_Cmd ( cCmd2, "OK", 0, 500 ) );
    }

    else
    {
        snprintf ( cCmd1, sizeof(cCmd1), "AT+CIPSERVER=%d,%s", 0, pPortNum );

        return ESP8266_Cmd ( cCmd1, "OK", 0, 500 );
    }

}


/*
 * 痕方兆��ESP8266_Get_LinkStatus
 * 宙峰  �沙馮� WF-ESP8266 議銭俊彜蓑��熟癖栽汽極笥扮聞喘
 * 補秘  �採�
 * 卦指  : 2��資誼ip
 *         3��秀羨銭俊
 *         3��払肇銭俊
 *         0��資函彜蓑払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_Get_LinkStatus ( void )
{
    if ( ESP8266_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
    {
        if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:2\r\n" ) )
            return 2;

        else if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:3\r\n" ) )
            return 3;

        else if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:4\r\n" ) )
            return 4;		

    }

    return 0;

}


/*
 * 痕方兆��ESP8266_Get_IdLinkStatus
 * 宙峰  �沙馮� WF-ESP8266 議極笥��Id��銭俊彜蓑��熟癖栽謹極笥扮聞喘
 * 補秘  �採�
 * 卦指  : 極笥��Id��議銭俊彜蓑��詰5了葎嗤丼了��蛍艶斤哘Id5~0��蝶了飛崔1燕乎Id秀羨阻銭俊��飛瓜賠0燕乎Id隆秀羨銭俊
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_Get_IdLinkStatus ( void )
{
    uint8_t ucIdLinkStatus = 0x00;


    if ( ESP8266_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
    {
        if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:0," ) )
            ucIdLinkStatus |= 0x01;
        else 
            ucIdLinkStatus &= ~ 0x01;

        if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:1," ) )
            ucIdLinkStatus |= 0x02;
        else 
            ucIdLinkStatus &= ~ 0x02;

        if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:2," ) )
            ucIdLinkStatus |= 0x04;
        else 
            ucIdLinkStatus &= ~ 0x04;

        if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:3," ) )
            ucIdLinkStatus |= 0x08;
        else 
            ucIdLinkStatus &= ~ 0x08;

        if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:4," ) )
            ucIdLinkStatus |= 0x10;
        else 
            ucIdLinkStatus &= ~ 0x10;	

    }

    return ucIdLinkStatus;

}


/*
 * 痕方兆��ESP8266_Inquire_ApIp
 * 宙峰  �沙馮� F-ESP8266 議 AP IP
 * 補秘  ��pApIp��贋慧 AP IP 議方怏議遍仇峽
 *         ucArrayLength��贋慧 AP IP 議方怏議海業
 * 卦指  : 0��資函払移
 *         1��資函撹孔
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_Inquire_ApIp ( char * pApIp, uint8_t ucArrayLength )
{
    unsigned char uc;

    char * pCh;


    ESP8266_Cmd ( "AT+CIFSR", "OK", 0, 500 );

    pCh = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "APIP,\"" );

    if ( pCh )
        pCh += 6;

    else
        return 0;

    for ( uc = 0; uc < ucArrayLength; uc ++ )
    {
        pApIp [ uc ] = * ( pCh + uc);

        if ( pApIp [ uc ] == '\"' )
        {
            pApIp [ uc ] = '\0';
            break;
        }

    }

    return 1;

}


/*
 * 痕方兆��ESP8266_UnvarnishSend
 * 宙峰  �催籌�WF-ESP8266庁翠序秘邑勧窟僕
 * 補秘  �採�
 * 卦指  : 1��塘崔撹孔
 *         0��塘崔払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_UnvarnishSend ( void )
{
    if ( ! ESP8266_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 ) )
        return false;

    return 
        ESP8266_Cmd ( "AT+CIPSEND", "OK", ">", 500 );

}


/*
 * 痕方兆��ESP8266_ExitUnvarnishSend
 * 宙峰  �催籌�WF-ESP8266庁翠曜竃邑勧庁塀
 * 補秘  �採�
 * 卦指  : 涙
 * 距喘  �艮四皺慎�喘
 */
void ESP8266_ExitUnvarnishSend ( void )
{
    Delay_ms ( 1000 );

    macESP8266_Usart ( "+++" );

    Delay_ms ( 500 ); 

}


/*
 * 痕方兆��ESP8266_SendString
 * 宙峰  ��WF-ESP8266庁翠窟僕忖憲堪
 * 補秘  ��enumEnUnvarnishTx��蕗苧頁倦厮聞嬬阻邑勧庁塀
 *       ��pStr��勣窟僕議忖憲堪
 *       ��ulStrLength��勣窟僕議忖憲堪議忖准方
 *       ��ucId��陳倖ID窟僕議忖憲堪
 * 卦指  : 1��窟僕撹孔
 *         0��窟僕払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_SendString ( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
    char cStr [20];
    uint8_t bRet = false;


    if ( enumEnUnvarnishTx )
    {
        macESP8266_Usart ( "%s", pStr );

        bRet = true;

    }

    else
    {
        if ( ucId < 5 )
            snprintf ( cStr, sizeof(cStr), "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2 );

        else
            snprintf ( cStr, sizeof(cStr), "AT+CIPSEND=%d", ulStrLength + 2 );

        ESP8266_Cmd ( cStr, "> ", 0, 100 );

        bRet = ESP8266_Cmd ( pStr, "SEND OK", 0, 500 );
    }

    return bRet;

}


/*
 * 痕方兆��ESP8266_ReceiveString
 * 宙峰  ��WF-ESP8266庁翠俊辺忖憲堪
 * 補秘  ��enumEnUnvarnishTx��蕗苧頁倦厮聞嬬阻邑勧庁塀
 * 卦指  : 俊辺欺議忖憲堪遍仇峽
 * 距喘  �艮四皺慎�喘
 */
char * ESP8266_ReceiveString ( FunctionalState enumEnUnvarnishTx )
{
    char * pRecStr = 0;


    strEsp8266_Fram_Record .InfBit .FramLength = 0;
    strEsp8266_Fram_Record .InfBit .FramFinishFlag = 0;

    while ( ! strEsp8266_Fram_Record .InfBit .FramFinishFlag );
    strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ] = '\0';

    if ( enumEnUnvarnishTx )
        pRecStr = strEsp8266_Fram_Record .Data_RX_BUF;

    else 
    {
        if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+IPD" ) )
            pRecStr = strEsp8266_Fram_Record .Data_RX_BUF;

    }

    return pRecStr;

}


/*
 * 痕方兆��ESP8266_CWLIF
 * 宙峰  �魂蚪�厮俊秘譜姥議IP
 * 補秘  ��pStaIp��贋慧厮俊秘譜姥議IP
 * 卦指  : 1��嗤俊秘譜姥
 *         0��涙俊秘譜姥
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_CWLIF ( char * pStaIp )
{
    uint8_t uc, ucLen;

    char * pCh, * pCh1;


    ESP8266_Cmd ( "AT+CWLIF", "OK", 0, 100 );

    pCh = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "," );

    if ( pCh )
    {
        pCh1 = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "AT+CWLIF\r\r\n" ) + 11;
        ucLen = pCh - pCh1;
    }

    else
        return 0;

    for ( uc = 0; uc < ucLen; uc ++ )
        pStaIp [ uc ] = * ( pCh1 + uc);

    pStaIp [ ucLen ] = '\0';

    return 1;

}


/*
 * 痕方兆��ESP8266_CIPAP
 * 宙峰  �塞蕚団�翠議 AP IP
 * 補秘  ��pApIp��庁翠議 AP IP
 * 卦指  : 1��譜崔撹孔
 *         0��譜崔払移
 * 距喘  �艮四皺慎�喘
 */
uint8_t ESP8266_CIPAP ( char * pApIp )
{
    char cCmd [ 30 ];


    snprintf ( cCmd, sizeof(cCmd), "AT+CIPAP=\"%s\"", pApIp );

    if ( ESP8266_Cmd ( cCmd, "OK", 0, 5000 ) )
        return 1;

    else 
        return 0;

}

/**************************************************/

/********************************** 喘薩俶勣譜崔議歌方**********************************/
#define   macUser_ESP8266_BulitApSsid         "utos-wifi"      //勣秀羨議犯泣議兆各
#define   macUser_ESP8266_BulitApEcn           WPA2_PSK               //勣秀羨議犯泣議紗畜圭塀
#define   macUser_ESP8266_BulitApPwd           "wildfire"         //勣秀羨議犯泣議畜埒

#define   macUser_ESP8266_TcpServer_IP         "192.168.121.169"      //捲暦匂蝕尼議IP仇峽
#define   macUser_ESP8266_TcpServer_Port       "8080"             //捲暦匂蝕尼議極笥   

#define   macUser_ESP8266_TcpServer_OverTime   "1800"             //捲暦匂階扮扮寂�┻ノ撮挫襭�


/**
 * @brief  ESP8266 ��Sta Tcp Client��邑勧
 * @param  涙
 * @retval 涙
 */
void ESP8266_StaTcpClient_UnvarnishTest ( void )
{
    uint8_t ucId, ucLen;
    uint8_t ucLed1Status = 0, ucLed2Status = 0, ucLed3Status = 0, ucBuzzerStatus = 0;

    char cStr [ 100 ] = { 0 }, cCh;

    char * pCh, * pCh1;

    PRINT_EMG( "\r\n屎壓塘崔 ESP8266 ......\r\n" );

    macESP8266_CH_ENABLE();

    ESP8266_AT_Test ();

    ESP8266_Net_Mode_Choose ( AP );

    while ( ! ESP8266_CIPAP ( macUser_ESP8266_TcpServer_IP ) );

    while ( ! ESP8266_BuildAP ( macUser_ESP8266_BulitApSsid, macUser_ESP8266_BulitApPwd, macUser_ESP8266_BulitApEcn ) );	

    ESP8266_Enable_MultipleId ( ENABLE );
    while ( !	ESP8266_StartOrShutServer ( ENABLE, macUser_ESP8266_TcpServer_Port, macUser_ESP8266_TcpServer_OverTime ) );

    ESP8266_Inquire_ApIp ( cStr, 20 );
    PRINT_EMG ( "\r\n云庁翠WIFI葎%s��畜鷹蝕慧賜宀葎%s\r\nAP IP 葎��%s��蝕尼議極笥葎��%s\r\n返字利大廁返銭俊乎 IP 才極笥��恷謹辛銭俊5倖人薩極\r\n",
            macUser_ESP8266_BulitApSsid, macUser_ESP8266_BulitApPwd, cStr, macUser_ESP8266_TcpServer_Port );


    strEsp8266_Fram_Record.InfBit.FramLength = 0;
    strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;	

    while ( 1 )
    {		
        if ( strEsp8266_Fram_Record.InfBit.FramFinishFlag )
        {
            USART_ITConfig ( macESP8266_USARTx, USART_IT_RXNE, DISABLE ); //鋤喘堪笥俊辺嶄僅
            strEsp8266_Fram_Record.Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ]  = '\0';

            PRINT_EMG( "\r\nrecv [%s]\r\n", strEsp8266_Fram_Record.Data_RX_BUF );//

            if ( ( pCh = strstr ( strEsp8266_Fram_Record.Data_RX_BUF, "+IPD," ) ) != 0 ) 
            {
                ucId = * ( pCh + strlen ( "+IPD," ) ) - '0';
                ESP8266_SendString ( DISABLE, cStr, strlen ( cStr ), ( ENUM_ID_NO_TypeDef ) ucId );
            }

            strEsp8266_Fram_Record.InfBit.FramLength = 0;
            strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;	

            USART_ITConfig ( macESP8266_USARTx, USART_IT_RXNE, ENABLE ); //聞嬬堪笥俊辺嶄僅

        }

    }


}

