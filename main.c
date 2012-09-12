/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Christmas tree
 *
 *****************************************************************************/

#include "boardVersion.h"
#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>
#include <string.h>
#include "pins.h"
#include "lcd.h"
#include "rgb.h"
#include "song.h"
#include "uart.h"
#include "../startup/config.h"

#if defined(BOARD_VERSION_LPC2104_1) || defined(BOARD_VERSION_LPC2103_2)
  #include "key.h"
#else
  #include "adc.h"
#endif

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD
#define STAT_INTERVAL (5*60*1000)
#define PROC1_STACK_SIZE 1400
#define PROC2_STACK_SIZE 50
#define PROC3_STACK_SIZE 600
#define PROC4_STACK_SIZE 10
#define PROC5_STACK_SIZE 1000
#define INIT_STACK_SIZE  800

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 proc2Stack[PROC2_STACK_SIZE];
static tU8 proc3Stack[PROC3_STACK_SIZE];
static tU8 proc4Stack[PROC4_STACK_SIZE];
static tU8 proc5Stack[PROC5_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
tU8 pid1;
tU8 pid2;
tU8 pid3;
tU8 pid4;
tU8 pid5;

tU32 users = 0;

volatile tU32 currentTime = 0;

static void proc1(void* arg);
static void proc2(void* arg);
static void proc3(void* arg);
static void proc4(void* arg);
static void proc5(void* arg);
static void initProc(void* arg);

void matrixControl(void);
void testI2C(void);

struct
{
	tU8  preBuffer[16];
	tU8  buffer[256];
	tU8  character;
	tU16 bufferIndex;
	tU8  newMessageFlag;
} rx;


tU8  display[] = "H'0'12:12 Fri 25-28B'h'0'00% 00/00 Wknd .'";

tU32 usersTimeout = 0;
#define USERS_TIMEOUT 200  //10 seconds
tU32 thRoom,thComp,thCore,targetDeg,targetOn,targetOff,degRoom,pcCore,targetCore;
tU8 periodCore =0 ,periodDemand = 0;
tU8 dayCore =0 ,dayDemand = 0;
	tU32 nextStatMs;





/*****************************************************************************
 *
 * Description:
 *    The first function to execute 
 *    Critical hardware initialisation.
 *    OS initialisation
 *    Call to first process
 ****************************************************************************/
int
main(void)
{
  tU8 error;
  tU8 pid;

  //immediately turn off buzzer (if connected)
  IODIR0 |= BUZZER;
  IOSET0  = BUZZER;
  
  //immediatley turn off all LEDs
  IOCLR = (LEDMATRIX_COL1 | LEDMATRIX_COL2 | LEDMATRIX_COL3 | LEDMATRIX_COL4 | LEDMATRIX_COL5 | LEDMATRIX_COL6) |
          (LEDMATRIX_ROW1 | LEDMATRIX_ROW2 | LEDMATRIX_ROW3 | LEDMATRIX_ROW4 | LEDMATRIX_ROW5 | LEDMATRIX_ROW6 | LEDMATRIX_ROW7 | LEDMATRIX_ROW8);
  IODIR |=(LEDMATRIX_COL1 | LEDMATRIX_COL2 | LEDMATRIX_COL3 | LEDMATRIX_COL4 | LEDMATRIX_COL5 | LEDMATRIX_COL6) |
          (LEDMATRIX_ROW1 | LEDMATRIX_ROW2 | LEDMATRIX_ROW3 | LEDMATRIX_ROW4 | LEDMATRIX_ROW5 | LEDMATRIX_ROW6 | LEDMATRIX_ROW7 | LEDMATRIX_ROW8);
  IOCLR = (LEDMATRIX_COL1 | LEDMATRIX_COL2 | LEDMATRIX_COL3 | LEDMATRIX_COL4 | LEDMATRIX_COL5 | LEDMATRIX_COL6) |
          (LEDMATRIX_ROW1 | LEDMATRIX_ROW2 | LEDMATRIX_ROW3 | LEDMATRIX_ROW4 | LEDMATRIX_ROW5 | LEDMATRIX_ROW6 | LEDMATRIX_ROW7 | LEDMATRIX_ROW8);

  //immediatley turn off RGB-LED
  IODIR0 |= (RGBLED_R | RGBLED_G | RGBLED_B);
  IOCLR0  = (RGBLED_R | RGBLED_G | RGBLED_B);
  
  //initialize uart #0: 57.6 kbps, 8N1, no FIFO
  initUart0(B57600((CRYSTAL_FREQUENCY * PLL_FACTOR) / VPBDIV_FACTOR), UART_8N1, UART_FIFO_OFF);

  osInit();
  osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
  osStartProcess(pid, &error);
  
  osStart();
  return 0;
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc1(void* arg)
{
	tU32 previousEventTime;
  tU8  error;
  const tU8 Day1[] = "MTWTFSS";
  const tU8 Day2[] = "ouehrau";
  const tU8 Day3[] = "neduitn";
  tU32 timeSec, timeSec10, timeMin, timeMin10, timeHour, timeHour10, timeDay, timeWeeks;
  rx.newMessageFlag = FALSE;
  osCreateProcess(proc5, proc5Stack, PROC5_STACK_SIZE, &pid5, 3, NULL, &error);
  osStartProcess(pid5, &error);
  outputToLcd("RCB'0'Init'");
  currentTime = ((5*24)+13)*3600*1000;
  nextStatMs = currentTime + STAT_INTERVAL;
      IODIR |= LEDMATRIX_COL1;    
      IOSET  = LEDMATRIX_ROW1;

      //initial setpoints
      targetDeg = 22;
      targetOn  = (targetDeg * 42) - 828 - 5;
      targetOff = (targetDeg * 42) - 828 + 5;
      targetCore = 550;


  //Keep display updated
  for (;;) {
    osSleep(100);
    if (display[6] == ':') {
      display[6] = ' ';
    } else {
     display[6] = ':';
    }
    outputToLcd(display);
    // "H'0'12:12 Fri 25-28B'h'0'00% 00/00 Wknd .'";
    //  0123456789012345678901234567890123456789012
    //  0000000000111111111122222222223333333333444

    timeSec   = currentTime/1000;
    timeSec10 = timeSec/10;
    timeSec   = timeSec - timeSec10*10;
    timeMin   = timeSec10/6;
    timeSec10 = timeSec10 - timeMin * 6;
    timeMin10 = timeMin/10;
    timeMin   = timeMin - timeMin10 * 10;
    timeHour  = timeMin10/6;
    timeMin10 = timeMin10 - timeHour * 6;
    timeDay   = timeHour / 24;
    timeHour  = timeHour - timeDay * 24;
    timeHour10 = timeHour / 10;
    timeHour = timeHour - timeHour10 * 10;
    timeWeeks = timeDay / 7;
    timeDay = timeDay - timeWeeks * 7;
    display[5]  = timeHour + '0';
    display[4]  = timeHour10 + '0';
    display[8]  = timeMin   + '0';
    display[7]  = timeMin10 + '0';
    display[10] = Day1[timeDay];
    display[11] = Day2[timeDay];
    display[12] = Day3[timeDay];
    if (thRoom < targetOn) {
      display[16] = '^';
      IOSET = LEDMATRIX_COL1;
      IOSET  = LEDMATRIX_ROW1;
      periodDemand = 1;
    }
    if (thRoom > targetOff) {
      IOCLR = LEDMATRIX_COL1;    
      if ((display[16] == '#') && (timeSec10 == 1)) {
	display[16] = '-';
	thComp += thComp/20;
      }
      if ((display[16] == '=') && (timeSec10 == 2)) {
	display[16] = '#';
	thComp += thComp/20;
      }
      if ((display[16] == '^') && (timeSec10 == 3)) {
	thComp += 68;
      display[16] = '=';
      }
    }
    display[17] = degRoom/10;
    display[18] = degRoom-display[17]*10 + '0';
    display[17] += '0';
    if ((thCore < targetCore) && (timeDay < 12)) {
      display[40] = '.';
      IOSET = LEDMATRIX_COL2;
      IOSET  = LEDMATRIX_ROW1;
      periodCore = 1;
    }
    if (thCore > targetCore+6)
      {
	display[40] = '*';
	IOCLR = LEDMATRIX_COL2;    
      }
    pcCore = thCore/7;
    display[25] = pcCore/10;
    display[26] = (pcCore-(display[25]*10)) + '0';
    display[25] += '0';

    if (currentTime > nextStatMs) {
      dayDemand  += periodDemand;
      dayCore    += periodCore;
      thComp += (periodDemand * 31);
      if (thComp > 0) {
	thComp -= ((thComp*2)/26);
      }
      if (thComp > 0) {
	thComp --;
      }      
      periodCore = 0;
      periodDemand = 0;
      nextStatMs += STAT_INTERVAL;
      display[29] = dayCore / 10;
      display[30] = dayCore - display[29]*10 + '0';
      display[29] += '0';
      display[32] = dayDemand / 10;
      display[33] = dayDemand - display[32]*10 + '0';
      display[32] += '0';
    }
  }
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc2(void* arg)
{
  	matrixControl();
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc3(void* arg)
{
  handleLcd();
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc4(void* arg)
{
   handleSong();
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
proc5(void* arg)
{
  tU8 i;
  tU8 command = 0;
  tU32 newTime,oldTime,boostOff=0;
  tU8 commandIndex;
  tU32 thRoom1=600,thRoom2=600,thRoom3=500,thRoom4=500;
  thComp = 0;
  rx.bufferIndex = 0;
  for(;;)
    {
      // Cal points: 
      // 22C 522 (178)
      // 23C 564 (138)
      thRoom4 = thRoom3;
      thRoom3 = thRoom2;
      thRoom2 = thRoom1;
      if (thComp > 2000) {
	thComp = 2000;
      }
      thRoom1 = getAnalogueInput(AIN0) + thComp/64;
      thRoom = (thRoom1+thRoom2+thRoom3+thRoom4)/4;
      if (thRoom < 700) {
	thRoom = 700-thRoom;
      } else {
	thRoom = 999;
      }
      degRoom = (thRoom+32)/8;
      // 12:00 192
      thCore = getAnalogueInput(AIN1);
      if (thCore < 700) {
	thCore = 700- thCore;
      } else {
	thCore = 998;
      }
      {
	printf("%c%c:%c%c Room %d (%d)Core %d On at   %d to   %d. Core %d Demand   %d\n",display[4],display[5],display[7],display[8],thRoom,thComp,thCore,targetOn,targetOff,dayCore,dayDemand);
	osSleep(4000);
	if (boostOff == 0) {
	} else {
	  printf ("%d %d\n",currentTime, boostOff);
	  if (currentTime > boostOff) {
	    printf ("boost ended\n");
	    targetOn  -= 18;
	    boostOff    = 0;
	    display[19] = ' ';
	    targetDeg = (targetOn + 32) / 8;
	    targetOff = targetOn + 4;
	    display[14] = targetDeg/10;
	    display[15] = targetDeg - display[14] * 10+ '0';
	    display[14] += '0';
	  }
	}
	//check if any charcters received
	while (uart0GetChar(&rx.character))
	  {
	    switch (command)
	      {
	      case 0: 
		switch (rx.character)
		  {
		    //check for new command
		  case 'd' : 
		  case 't' : 
		  case 's' : 
		  case 'c' : 
		    usersTimeout = USERS_TIMEOUT;
		    command = rx.character;
		    commandIndex=0;
		    break;
		  case 'b' :
		    if (boostOff == 0) {
		      targetOn += 18;
		      boostOff = currentTime + 60*60*1000;
		      display[19] = 'B';
		    } else {
		      targetOn  -= 18;
		      boostOff    = 0;
		      display[19] = ' ';
		    }
		    targetDeg = (targetOn + 32) / 8;
		    targetOff = targetOn + 4;
		    display[14] = targetDeg/10;
		    display[15] = targetDeg - display[14] * 10+ '0';
		    display[14] += '0';
		    break;
		  case '?' :
		    printf("d{n}    - Day. Mon is 0. Time to 00:00\n");
		    printf("t{nnnn} - Set time.\n");
		    printf("b       - Boost for 1hr\n");
		    printf("B       - Boost for 2hr\n");
		    printf("s{nn}   - Set current set-point\n");
		    printf("/       - display targets\n");
		  case '/' :
		    printf("Setpoint  - On %d, Off %d, %dC\n",targetOn,targetOff,targetDeg);
		    printf("Core Max  - %d\n",targetCore);
		    printf("BoostEnd  - %d. init timer\n",boostOff);
		    nextStatMs = currentTime + STAT_INTERVAL;
		    outputToLcd("RCB'0'Init'");
		    break;
		  }
		break;
	      case 'd' :
		// parsing time
		if ((rx.character >= '0') && (rx.character <= '9'))
		  {
		    currentTime = (rx.character - '0') * 24 *3600 *1000;
		  }
		command = 0;
		break;
	      case 't' :
		// parsing time
		if ((rx.character >= '0') && (rx.character <= '9'))
		  {
		    switch (commandIndex)
		      {
		      case 0:
			oldTime = currentTime/24/3600/1000;
			oldTime *=24;
			newTime = rx.character - '0';
			newTime *= 10;
			newTime += oldTime;
			break;
		      case 1:
			newTime += rx.character - '0';
			newTime *= 6;
			break;
		      case 2:
			newTime += rx.character - '0';
			newTime *= 10;
			break;
		      case 3:
			newTime += rx.character - '0';
			newTime *= 60000;
			currentTime = newTime;
			command = 0;
			printf ("Time Set\n");
			dayDemand = 0;
			dayCore = 0;
			break;
		      }
		    commandIndex++;
		  } else {
		  printf("ERROR - reseting parser\n");
		  command = 0;
		}
		break;
	      case 's' :
	      case 'c' :
		// parsing setpoint
		if ((rx.character >= '0') && (rx.character <= '9'))
		  {
		    switch (commandIndex)
		      {
		      case 0:
			newTime = rx.character - '0';
			break;
		      case 1:
			newTime *= 10;
			newTime += rx.character - '0';
			break;
		      case 2:
			newTime *= 10;
			newTime += rx.character - '0';
			break;
		      }
		    commandIndex++;
		  }
		if ((rx.character ==13) || (commandIndex ==3)) {
		  if (command == 'c') {
		    targetCore = newTime;
		    printf ("Core Max now %d,%d\n",newTime,targetCore);
		  } else {
		    targetOn = newTime - 2;
		    targetOff = newTime + 2;
		    targetDeg = (newTime + 32) / 8;
		    display[14] = targetDeg/10;
		    display[15] = targetDeg - display[14] * 10 + '0';
		    display[14] += '0';
		    printf ("Setpoint now %d,%d\n",newTime,targetDeg);
		  }
		  command = 0;
		}
		break;
	      }
	  }
      }
    }      
      // ignore this code for now
      for(;;)
	{
#if defined(BOARD_VERSION_LPC2104_1) || defined(BOARD_VERSION_LPC2103_2)
	  sampleKeys();
#endif

		osSleep(5);
		if (usersTimeout > 0)
		  usersTimeout--;

    //check if any charcters received
    while (uart0GetChar(&rx.character))
    {
    	//check if ping request
    	if (rx.character == 't')
    	{
	      uart0SendChar('T');
        usersTimeout = USERS_TIMEOUT;
      }

    	//check if ping request
    	else if (rx.character == 0xf3)
    	{
    		if (rx.newMessageFlag == FALSE)
	        uart0SendChar(0xf3);  //ACK = OK
	      else
	        uart0SendChar(0xf4);  //NACK
    	}

    	else
    	{
    		//only receive if last message has been processed
    		if (rx.newMessageFlag == FALSE)
    		{
      	  //check if start of message
      	  if ((rx.character == 0xf0) || (rx.character == 0xf1) || (rx.character == 0xf2))
      	  {
      		  rx.bufferIndex = 1;
    	  	  rx.buffer[0] = rx.character;
    	    }
    	    //fill received characters in buffer
    	    else
    	    {
    	  	  if (rx.bufferIndex < sizeof(rx.buffer))
    	  	    rx.buffer[rx.bufferIndex++] = rx.character;
    		
    	      //check if buffer complete
    	      if ((rx.bufferIndex > 1) && (rx.bufferIndex == (rx.buffer[1] + 2)))
    	      {
	            //signal that command has been accepted
	            uart0SendChar(rx.buffer[0]);
	          
  	          //handle received command
	            switch(rx.buffer[0])
	            {
        	      //0xf0 = number of users
	            	case 0xf0:
	            	users = 0;
	            	for(i=0; i<rx.buffer[1]; i++)
	          	  {
	          		  users *= 10;
  	          		users += rx.buffer[2+i] - '0';
	            	}
	            	break;

      	        //0xf1 = message
	            	case 0xf1:
                rx.newMessageFlag = TRUE;
	          	  break;

        	      //0xf2 = prio-message
	            	case 0xf2:
                rx.newMessageFlag = TRUE;
	            	break;

	            	default:
	            	break;
	            }
	    }
		}
      	}
    }
	}
    }
}

/*****************************************************************************
 *
 * Description:
 *    The entry function for the initialization process. 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void
initProc(void* arg)
{
  tU8 error;

  TIMER0_MR0 = 147459;        /* Peripheral clock rate / TICKS = tick rate */

  printf("\n\n\n\n\n*******************************************************\n");
  printf("*                                                     *\n");
  printf("* Welcome to heating control system                   *\n");

  printf("*******************************************************\n");

	//init buzzer
	//initSong();

  //init RGB-LED
  //initRGB();

#if defined(BOARD_VERSION_LPC2104_1) || defined(BOARD_VERSION_LPC2103_2)
  //Initialize keys
  initKeys();
#else
  //Initialize ADC
  initAdc();
#endif

  //Initialize LCD
  initLcd();

  //Test EEPROM via I2C
  testI2C();
  
  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 3, NULL, &error);
  osStartProcess(pid1, &error);
  //osCreateProcess(proc2, proc2Stack, PROC2_STACK_SIZE, &pid2, 3, NULL, &error);
  //osStartProcess(pid2, &error);
  osCreateProcess(proc3, proc3Stack, PROC3_STACK_SIZE, &pid3, 3, NULL, &error);
  osStartProcess(pid3, &error);
  //  osCreateProcess(proc4, proc4Stack, PROC4_STACK_SIZE, &pid4, 3, NULL, &error);
  //osStartProcess(pid4, &error);

  osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/
void
appTick(tU32 elapsedTime)
{
	currentTime += elapsedTime;
}
