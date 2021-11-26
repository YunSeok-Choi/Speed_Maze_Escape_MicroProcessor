#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"
#include "utils/uartstdio.h"

#define MAPSIZE 12

uint32_t SysClock, ADC[8], x, y;
volatile int time = 60 , data = 0 ;
volatile char level[2];
volatile int row = 1, col = 0;
volatile int chr = 0;

char map[12][12] = { { '1','1','1','1','1','1','1','1','1','1','1','1' },
                      { 'x','0','1','1','1','1','1','1','1','1','0','1' },
                      { '1','0','1','1','0','0','0','1','1','1','0','1' },
                      { '1','0','0','0','0','1','0','1','1','0','0','1' },
                      { '1','1','1','0','1','1','0','1','1','0','1','1' },
                      { '1','1','1','0','1','1','0','0','0','0','1','1' },
                      { '1','1','1','0','1','1','1','1','0','1','1','1' },
                      { '1','0','0','0','1','1','1','1','0','1','1','1' },
                      { '1','0','1','0','0','0','0','0','0','0','0','1' },
                      { '1','0','1','1','1','1','1','1','1','1','0','1' },
                      { '1','0','0','0','0','1','1','1','1','1','0','y' },
                      { '1','1','1','1','1','1','1','1','1','1','1','1' },
};

void Delay(uint32_t ui32Seconds){   // 딜레이 함수
    uint8_t ui8Loop;

    while(ui32Seconds--)
    {
        for(ui8Loop = 0; ui8Loop < 20; ui8Loop++)
        {
            while(SysTickValueGet() > 1000) {  }

            while(SysTickValueGet() < 1000) {  }
        }
    }
}


void UART0IntHandler(void)      // UART 핸들러
{
    unsigned long ulStatus;
    int i;

    ulStatus = UARTIntStatus(UART0_BASE, true);

    UARTIntClear(UART0_BASE, ulStatus);

    for (i = 0; i < 2; i++){
        while(UARTCharsAvail(UART0_BASE)){
            level[i] = UARTCharGetNonBlocking(UART0_BASE);
            UARTCharPutNonBlocking(UART0_BASE, level[i]);
        }
    }
    if(level[0]== '1'){ // 난이도 하
        time = 60;
        level[0] = '0';
    }else if(level[0] == '2'){  // 난이도 중
        time = 45;
        level[0] = '0';
    }else if(level[0] == '3'){  // 난이도 상
        time = 30;
        level[0] = '0';
    }else{                      //  1~3 입력이 아닐시 1단계로 시작
        TimerEnable(TIMER0_BASE, TIMER_A);
        ADCIntEnable(ADC0_BASE, 0);
    }
}

void Timer0IntHandler(void)             //TIMER 핸들러
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    time--;

   IntMasterDisable();

    if(time == 0){                          // 0초 될 시 종료
        UARTprintf("\033[2J \n\t\t\tFail.");
        return;
    }
   IntMasterEnable();
}

void ADC0IntHandler(){                  // ADC 핸들러

    ADCSequenceDataGet(ADC0_BASE,0,ADC);
    ADCSequenceDataGet(ADC0_BASE,1,ADC);

    x = (ADC[0]*25)/100;                    // 조이스틱 0 ~ 1000 사이의 x 값
    y = (ADC[1]*25)/100;                    // 조이스택 0 ~ 1000 사이의 y 값

    if(x > 400 && x < 600 && y > 400 && y < 600) data = 0;
    else if(x < 200) data = 1;      //좌
    else if(x > 800) data = 2;      //우
    else if(y < 200) data = 3;      //상
    else if(y > 800) data = 4;      //하


    print_Map(map);                       // 맵 출력
    move_Me(map);                        // 주인공 움직임

    Delay(1);

    ADCIntClear(ADC0_BASE,0);
}
void print_Map(char maze[][MAPSIZE]){           // 맵 출력
    int i, j;

    UARTprintf("\033[2J\r           Time: %d\n", time);     //현재 남은 시간

    for (i = 0; i < MAPSIZE; i++){
        for (j = 0; j < MAPSIZE; j++)
        {
            if (maze[i][j] == '1')
                UARTprintf("#");
            else if (maze[i][j] == 'x'){
                UARTprintf("M");
                if(j >= 0 && j <= 3){                      // 주인공이 도착지점에 가까워 질 수록 값 증가
                    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, false);
                    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWMGenPeriodGet(PWM0_BASE,PWM_GEN_0)*1/50);
                    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
                }
                else if(j >= 4 && j <= 7){
                    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, false);
                    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWMGenPeriodGet(PWM0_BASE,PWM_GEN_0)*25/50);
                    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
                }
                else if(j >= 8 && j <= 11){
                    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, false);
                    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWMGenPeriodGet(PWM0_BASE,PWM_GEN_0)*45/50);
                    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
                }
            }
            else if (maze[i][j] == '0')
                UARTprintf(" ");
            else
                UARTprintf("G");
        }
        UARTprintf("\n");
    }

}
void ConfigureUART(void)                        //UART 설정
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));


    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    UARTStdioConfig(0, 115200, 16000000);
}

void ConfigureTIME(void){                       //TIMER 설정

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1);

    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);

    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
}

void ConfigurePWM(void){                        //PWM 설정

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    GPIOPinConfigure(GPIO_PB6_M0PWM0);

    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);

    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, (SysCtlClockGet()/4/250));

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWMGenPeriodGet(PWM0_BASE,PWM_GEN_0)*1/50);

    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, false);

}

void ConfigureADC(void){                        //ADC 설정

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_3);

    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_FULL, 0);

    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_ALWAYS , 0);

    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH1 | ADC_CTL_IE);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, 0);

}

int block(char maze[][MAPSIZE], int i, int j)       // 가려는 방향이 벽이라면
{
    if (maze[i][j] == '1' || maze[i][j] == 'y') //미로가 벽일 경우, 종점일 경우
        return 1;
    else
        return 0;
}
int finish(char maze[][MAPSIZE], int i, int j)      // 종점일 경우
{

    if (maze[i][j] == 'y') //종점일 경우
        return 1;
    else
        return 0;
}

void move_Me(char maze[][MAPSIZE])                  //주인공을 움직이는 함수
{
    int i = row; // 1 주인공의 초기 위치
    int j = col; // 0

    if(data == 3){ // 상
        i--;
        if (!(block(maze, i, j))) // 벽이 아닐 경우 움직일 수 있음
        {
            maze[row][j] = '0'; //이전 블록에 0을 삽입
            maze[i][j] = 'x'; //방향을 옮긴 뒤 x를 삽입
            row -= 1;
        }
        else if (finish(maze, i, j)) //종점일 경우
        {
            maze[row][j] = '0';
            maze[i][j] = 'x';
            UARTprintf("\033[2J \t\tComplete!!!\n");
            Delay(1);
            IntMasterDisable();
        }
    }
    else if(data == 4){ //하
        i++;
        if (!(block(maze, i, j)))
        {
            maze[row][j] = '0';
            maze[i][j] = 'x';
            row += 1;
        }
        else if (finish(maze, i, j))
        {
            maze[row][j] = '0';
            maze[i][j] = 'x';
            UARTprintf("\033[2J \t\tComplete!!!\n");
            Delay(1);
            IntMasterDisable();
        }
    }
    else if(data == 1){ //좌
        j--;
        if (!(block(maze, i, j)))
        {
            maze[i][col] = '0';
            maze[i][j] = 'x';
            col -= 1;
        }
        else if (finish(maze, i, j))
        {
            maze[i][col] = '0';
            maze[i][j] = 'x';
            UARTprintf("\033[2J \t\tComplete!!!\n");
            Delay(1);
            IntMasterDisable();
        }
    }
    else if(data == 2){ //우
        j++;
        if (!(block(maze, i, j)))
        {
            maze[i][col] = '0';
            maze[i][j] = 'x';
            col += 1;
        }
        else if (finish(maze, i, j))
        {
            maze[i][col] = '0';
            maze[i][j] = 'x';
            UARTprintf("\033[2J \t\tComplete!!!\n");
            Delay(1);
            IntMasterDisable();
        }
    }

}
int main(void){

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    SysTickPeriodSet(SysCtlClockGet()/100);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    ConfigureUART();
    ConfigureTIME();
    ConfigureADC();
    ConfigurePWM();

    UARTprintf("\033[2J\t\t Welcome to Speed Maze escape \n");
    UARTprintf("\t\t Insert Level(1 ~ 3) : ");

    IntMasterEnable();

    IntEnable(INT_TIMER0A);
    IntEnable(INT_UART0);
    IntEnable(INT_ADC0SS0);

    SysTickEnable();
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);

    while(1)    // 게임이 시작되기 전에 반짝이다 시작 시 멈춤. 끝나면 다시 반짝 거림.
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x04);
        Delay(1);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x00);
        Delay(1);

    }
}
