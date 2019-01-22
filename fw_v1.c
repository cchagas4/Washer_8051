#include <8051.h>
#include <stdio.h>
#include <strings.h>

#define  EXEC_LED   P0_0
#define  LAVA_LED   P0_1
#define  ENXA_LED   P0_2
#define  CENT_LED   P0_3
#define  DETE_LED   P0_4
#define  AMAC_LED   P0_5
#define  BOMB_LED   P0_6
#define  MOTO_LED   P0_7

#define  DIP_8      P1_0
#define  DIP_7      P1_1
#define  DIP_6      P1_2
#define  DIP_5      P1_3
#define  DIP_4      P1_4
#define  DIP_3      P1_5
#define  DIP_2      P1_6
#define  DIP_1      P1_7

#define LCD         P2
#define LCD_RS      P2_0
#define LCD_RW      P2_1
#define LCD_EN      P2_2

#define LCD_INIT    0x28
#define LCD_CURSOR  0x0C
#define LCD_INC     0x06
#define LCD_CLEAR   0x01
#define LCD_HOME    0x80
#define LCD_2LINE   0xC0

#define START_BT    P3_2
#define INC_BT      P3_3
#define DEC_BT      P3_4
#define SELECT_BT   P3_5

#define DETERGENTE  0
#define AMACIANTE   1

void configuraTimer(void);

void lavar(int lavagemTempo);
void enxaguar(int enxagueNum);
void centrifugar(int centrifugaTempo);

int sensorCheio(void);
int sensorVazio(void);
int sensorTampa(void);

void esvaziaMaquina(void);
void encheMaquina(int produto);

void diplayInit(void);
void LCD_Command (char cmnd);
void LCD_Char (char char_data);
void LCD_String (char *str);
void LCD_String_xy (char row, char pos, char *str);

char verificaBotao(void);
void configuraCiclo(void);

/*
  bit7 -> Flag_Motor
  bit6 -> Flag_Bomba
  bit5 -> Flag_AguaEAmaciante
  bit4 -> Flag_BombaAtiva

  bit3 -> Flag_SensorNivel_Cheio
  bit2 -> Flag_SensorNivel_Vazio
  bit1 -> Flag_TampaAberta
  bit0 _> Flag_Lavagem
*/

__xdata int flags = 0x00;
__xdata int pwmCtrl = 0;
__xdata int pwmAtual = 0;

__xdata int tempoLavagem = 0;
__xdata int tempoEnxague = 0;
__xdata int tempoCentrifuga = 0;

//int timerCtrl = 0;
//char delayCount = 0;
//char delayCtrl = 0;
//int lcd = 0x90;

__xdata int ctrl = 0;
__xdata unsigned long int countSeconds = 0;

void delay(unsigned int count){
 int i,j;
 for(i=0;i<count;i++)
 for(j=0;j<112;j++);
}


void Timer0_ISR(void) __interrupt (1){
  switch (pwmAtual) {
    case 25:{
      if (pwmCtrl>0 && pwmCtrl<=10) {
        MOTO_LED = 0;
      }else if (pwmCtrl>11 && pwmCtrl<=40) {
        MOTO_LED = 1;
      }
    }break;
    case 30:{
      if (pwmCtrl>0 && pwmCtrl<=12) {
        MOTO_LED = 0;
      }else if (pwmCtrl>=13 && pwmCtrl<=40) {
        MOTO_LED = 1;
      }
    }break;
    case 40:{
      if (pwmCtrl>0 && pwmCtrl<=16) {
        MOTO_LED = 0;
      }else if (pwmCtrl>=17 && pwmCtrl<=40) {
        MOTO_LED = 1;
      }
    }
    case 50:{
      if (pwmCtrl>0 && pwmCtrl<=20) {
        MOTO_LED = 0;
      }else if (pwmCtrl>=21 && pwmCtrl<=40) {
        MOTO_LED = 1;
      }
    }break;
    case 75:{
      if (pwmCtrl>0 && pwmCtrl<=30) {
        MOTO_LED = 0;
      }else if (pwmCtrl>=31 && pwmCtrl<=40) {
        MOTO_LED = 1;
      }
    }break;
    case 100:{
      MOTO_LED = 0;
    }break;
    default:{
      MOTO_LED = 1;
    }break;
  }
  pwmCtrl = (pwmCtrl==40?0:pwmCtrl+1);

  ctrl++;
  if (ctrl == 2000) {
    ctrl = 0;
    countSeconds++;
  }
  TH0 = 0xFF;
  TL0 = 0xCD;
  return;
}

void main(void){

  P0 = 0xF0;
  /*
  configuraTimer();
  EA = 1;
  */
  diplayInit();

  //configuraCiclo();

  /*
  lavar(200);
  enxaguar(2);
  centrifugar(200);

  P0 = 0x0FF;
  */

  while(1){

  }

}

void configuraTimer(void){
  //Usando somente o Timer 0, temos:
  IE = 0x02;
  TMOD = 0x01;
  //Com o micro de 12MHz temos um ciclo de máquina de 1us
  //Para 50us temos 65535-50000 = 65486
  // Portantop 0xFFCD em hexadecimal
  TH0 = 0xFF;
  TL0 = 0xCD;
  TR0= 1;
}

char verificaBotao(void){
  int n;
  while (1) {
    if (!INC_BT) {
      n = 1;
      break;
    }
    if (!DEC_BT) {
      n = 2;
      break;
    }
    if (SELECT_BT) {
      n = 3;
      break;
    }
  }
  return n;
}

void configuraCiclo(void){
  __xdata int select = 0;
  __xdata int ver;
  char msg[16];

  LCD_Command (LCD_CLEAR);	 // Limpa o display
  LCD_String("Config do tempo");
  LCD_Command(0xC0);
  LCD_String_xy(1,0,"de lavagem");
  while (!select) {
    ver = verificaBotao();
    if (ver == 1) {
      tempoLavagem++;
    } else if (ver == 2) {
      if (tempoLavagem>0) {
        tempoLavagem--;
      }
    } else if( ver == 3){
      select = 1;
    }
    //select = 0;
    memset(msg, '', 16);
    sprintf(msg, "%is", tempoLavagem);
    //LCD_String_xy(1, 10, msg);
  }
  delay(500);
  LCD_Command(0xC0);
  LCD_String_xy(1,0,"de enxague      ");
  while (!select) {
    ver = verificaBotao();
    if (ver == 1) {
      tempoEnxague++;
    } else if (ver == 2) {
      if (tempoEnxague>0) {
        tempoEnxague--;
      }
    } else if( ver == 3){
      select = 1;
    }
    //select = 0;
    memset(msg, 16);
    sprintf(msg, "%is", tempoEnxague);
    //LCD_String_xy(1, 10, msg);
  }
  delay(500);
  LCD_Command(0xC0);
  LCD_String_xy(1,0,"de centrif.     ");
  while (!select) {
    ver = verificaBotao();
    if (ver == 1) {
      tempoCentrifuga++;
    } else if (ver == 2) {
      if (tempoCentrifuga>0) {
        tempoCentrifuga--;
      }
    } else if( ver == 3){
      select = 1;
    }
    memset(msg, "", 16);
    sprintf(msg, "%is", tempoCentrifuga);
    //LCD_String_xy(1, 10, msg);
  }
}

void varreduraLED(){}

void varreduraDisplay(){}

void varreduraBotoes(){}

void lavar(int lavagemTempo){
  while (1) {
    LCD_Command (LCD_CLEAR);	 // Limpa o display
  	LCD_String("Tampa Aberta!");
  	LCD_Command(0xC0);
  	LCD_String_xy(1,0,"Lavagem");
    //Desliga os LEDs Indicativos
    //Liga os Leds Em Funcionamento e lavagem
    P0 = 0xFC;
    if(!sensorTampa()) { //Se a tampa estiver fechada
      LCD_Command (LCD_CLEAR);	 // Limpa o display
    	LCD_String("Agua+Detergente");
    	LCD_Command(0xC0);
    	LCD_String_xy(1,0,"Lavagem");
      //Enche a máquina com água e detergente
      encheMaquina(DETERGENTE);

      LCD_Command (LCD_CLEAR);	 // Limpa o display
    	LCD_String("Em Funcionamento");
    	LCD_Command(0xC0);
      char msg[16];
      sprintf(msg, "");
    	LCD_String_xy(1,0,"Lavagem    00:00");

      //Lava 10 segundos girando à 30% e 10 min parado
      for (int b = 0; b < 4; b++) {

        //MOTO_LED = 0;
        pwmCtrl = 0;
        pwmAtual = 30;
        //wait 10 segundos
        delay(lavagemTempo);

        //MOTO_LED = 1;
        pwmAtual = 0;
        delay(lavagemTempo);
      }
      //Executa o processo de lavagem
      LAVA_LED = 1;
      break;
    }
  }
}

void enxaguar(int enxagueNum) {
  while (1) {

    LCD_Command (LCD_CLEAR);	 // Limpa o display
  	LCD_String("Tampa Aberta!");
  	LCD_Command(0xC0);
  	LCD_String_xy(1,0,"Enxague");

    //Desliga os LEDs Indicativos
    //Liga os Leds Em Funcionamento e Enxague
    P0 = 0xFA;
    if (!sensorTampa()) { //Se a tampa estiver fechada
      LCD_Command (LCD_CLEAR);	 // Limpa o display
    	LCD_String("Retirando Agua...");
    	LCD_Command(0xC0);
    	LCD_String_xy(1,0,"Enxague");

      for (int e = 0; e < enxagueNum; e++) {
        //esvaziar agua da maquina
        esvaziaMaquina();

        LCD_Command (LCD_CLEAR);	 // Limpa o display
      	LCD_String("Agua + Amaciante");
      	LCD_Command(0xC0);
      	LCD_String_xy(1,0,"Enxague");

        //Enche a máquina com água e amaciante
        encheMaquina(AMACIANTE);

        LCD_Command (LCD_CLEAR);	 // Limpa o display
        LCD_String("Em Funcionamento");
        LCD_Command(0xC0);
        char msg[16];
        sprintf(msg, "");
        LCD_String_xy(1,0,"Enxague    00:00");

        //Enxagua 10 segundos girando à 50% e 10 min parado
        pwmCtrl = 0;
        pwmAtual = 40;
        //wait 10 segundos

        for (int a = 0; a < 5; a++){
          delay(50);
        }
        //wait10segundos
        //MOTO_LED = 1;
        pwmAtual = 0;
        delay(50);
      }
      ENXA_LED = 1;
      break;
    }
  }
}

void centrifugar(int centrifugaTempo  ) {
  while (1) {

    LCD_Command (LCD_CLEAR);	 // Limpa o display
  	LCD_String("Tampa Aberta!");
  	LCD_Command(0xC0);
  	LCD_String_xy(1,0,"Centrifuga");

    //Desliga os LEDs Indicativos
    //Liga os Leds Em Funcionamento e Enxague
    P0 = 0xF6;
    if (!sensorTampa()) {
      LCD_Command (LCD_CLEAR);	 // Limpa o display
      LCD_String("Retirando Agua...");
      LCD_Command(0xC0);
      LCD_String_xy(1,0,"Enxague");

      //Esvazia a máquina de lavar
      esvaziaMaquina();

      LCD_Command (LCD_CLEAR);	 // Limpa o display
      LCD_String("Em Funcionamento");
      LCD_Command(0xC0);
      char msg[16];
      sprintf(msg, "");
      LCD_String_xy(1,0,"Centrifuga 00:00");

      //TODO Liga o motor gradativamente 25%, 50%, 75% e 100%
      int tempoBase = centrifugaTempo/4;
      pwmCtrl=0;
      pwmAtual = 25;
      delay(200);
      pwmAtual = 50;
      delay(200);
      pwmAtual = 75;
      delay(200);
      pwmAtual = 100;
      delay(200);

      //Durante o tempo pre estipulado
      //MOTO_LED = 0;
      pwmAtual = 0;
      //wait 10 segundos

      //Desliga o motor
      MOTO_LED = 1;
      CENT_LED = 1;
      break;
    }
  }
  //Liga os Leds Em Funcionamento e Centrifuga
  EXEC_LED = 0;
  P0_3 = 0;
}

int sensorVazio(){
  if(DIP_1 == 0 | DIP_2 == 0){
    return 1;
  }
  return 0;
}

int sensorCheio(){
  if(DIP_4 == 0 | DIP_5 == 0){
    return 1;
  }
  return 0;
}

int sensorTampa(){
  if(DIP_7 == 0 | DIP_8 == 0){
    return 1; //Tampa Aberta
  }
  return 0;   //Tampa Fechada
}

void esvaziaMaquina(void){
  // TODO esvaziar agua da maquina
  BOMB_LED = 0;
  while (sensorVazio()){}
  BOMB_LED = 1;
}

void encheMaquina(int produto){
  switch (produto) {
    case DETERGENTE:{ DETE_LED = 0; }break;
    case AMACIANTE:{  AMAC_LED = 0; }break;
  }
  while (1) {
    if(sensorCheio()){
      switch (produto) {
        case DETERGENTE:{ DETE_LED = 1; }break;
        case AMACIANTE:{  AMAC_LED = 1; }break;
      }
      break;
    }
  }
}

void diplayInit(void){
  delay(20);
  LCD_Command (0x02);	       // Modo LCD 4bit
	LCD_Command (LCD_INIT);	   // Inicializa o Display 16X2 no modo 4bit
	LCD_Command (LCD_CURSOR);	 // Display ON Cursor OFF
	LCD_Command (LCD_INC);	   // Auto-Incremento do Cursor
	LCD_Command (LCD_CLEAR);	 // Limpa o display
	LCD_Command (LCD_HOME);	   // Leva o cursor para o inicio
}

void LCD_Command (char cmnd){
	LCD = (LCD & 0x0F) | (cmnd & 0xF0);/* Send upper nibble */
	LCD_RS = 0;			/* Command reg. */
	LCD_RW = 0;			/* Write operation */
	LCD_EN = 1;
	delay(1);
	LCD_EN = 0;
	delay(2);

	LCD = (LCD & 0x0F) | (cmnd << 4);/* Send lower nibble */
	LCD_EN = 1;			/* Enable pulse */
	delay(1);
	LCD_EN = 0;
	delay(5);
}

/* LCD data write function */
void LCD_Char (char char_data){
	LCD = (LCD & 0x0F) | (char_data & 0xF0);/* Send upper nibble */
	LCD_RS = 1;  			/*Data reg.*/
	LCD_RW = 0;  			/*Write operation*/
	LCD_EN = 1;
	delay(1);
	LCD_EN = 0;
	delay(2);

	LCD = (LCD & 0x0F) | (char_data << 4);/* Send lower nibble */
	LCD_EN = 1;  			/* Enable pulse */
	delay(1);
	LCD_EN = 0;
	delay(5);

}

/* Send string to LCD function */
void LCD_String (char *str){
	int i;
	for(i= 0;str[i]!= 0;i++)  /* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);  /* Call LCD data write */
	}
}

/* Send string to LCD function */
void LCD_String_xy (char row, char pos, char *str){
	if (row == 0)
	LCD_Command((pos & 0x0F)|0x80);
	else if (row == 1)
	LCD_Command((pos & 0x0F)|0xC0);
	LCD_String(str);  	/* Call LCD string function */
}
