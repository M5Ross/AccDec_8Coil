// Production 17 Function DCC Decoder 
// Version 5.1  Geoff Bunza 2014,2015,2016
// NO LONGER REQUIRES modified software servo Lib
// Software restructuring mods added from Alex Shepherd and Franz-Peter
//   With sincere thanks

      /*********************************************
       *                                           *
       *  modified by M.Ross => AccDec8Coil        *
       *                                           *
       *********************************************/

// ******** UNLESS YOU WANT ALL CV'S RESET UPON EVERY POWER UP
// ******** AFTER THE INITIAL DECODER LOAD REMOVE THE "//" IN THE FOOLOWING LINE!!
#define DECODER_LOADED

// THIS ADDRESS IS THE START OF THE SWITCHES RANGE
// example: 1 => 1,2,3,4,5,6,7,8
//          15 => 15,16,17,18,19,20,21,22
//          123 => 123,124,125,126,127,128,129,130
#define Accessory_Address 1
                                         
// Meke inactive selected turnaut for defined time
// (milliseconds) to prevent coils over heating
#define TIME_DISACTIVE_OUTPUT 50  // x10

// Uncomment if you wont to set turnaut at power up to the last known position in CV
#define SET_LAST_SAVED_POSITION

// Comment if you wont continuously active outputs (WARNING: only for led/bulbe ligths, ABSOLUTALY NOT coils switches)
#define PULSING_OUTPUTS

// Uncomment if you wont to set specific address for each output
//#define USE_MULTIPLE_ADDRESS

// Uncomment to invert direction of all outputs by default
//#define INVERT_DIRECTION

// ******** REMOVE THE "//" IN THE FOOLOWING LINE TO SEND / RECEIVE
// ******** CONFIGURATION TO THE SERIAL MONITOR
#define SERIALCOM

// ******** REMOVE THE "//" IN THE FOOLOWING LINE TO SEND DEBUGGING
// ******** INFO TO THE SERIAL MONITOR
//#define DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////////
#include <NmraDcc.h>
#include <confCV.h>
//#include <SoftwareServo.h> 

#define HW_VERSION 10
#define SW_VERSION 21

#define DECODER_ID 2


int tim_delay = 500;
int numfpins = 8;

byte FApins [] = {3,5,7,9,11,A0,A2,A4};
byte FBpins [] = {4,6,8,10,12,A1,A3,A5};

unsigned long temp[8];

const int LED = 13;

NmraDcc  Dcc ;
confCV   CV28(Dcc) ;

int t;                                    // temp
#define SET_CV_Address  99                // THIS ADDRESS IS FOR SETTING CV'S Like a Loco

                                          // WHICH WILL EXTEND FOR 16 MORE SWITCH ADDRESSES
uint8_t CV_DECODER_MASTER_RESET =   120;  // THIS IS THE CV ADDRESS OF THE FULL RESET
#define CV_To_Store_SET_CV_Address	121
#define CV_Accessory_Address CV_ACCESSORY_DECODER_ADDRESS_LSB
#define CV_TIME_DISACTIVE_OUTPUT    27

#define CV_SINGLE_INV     70
#define CV_MULTI_ADDRESS  80

struct QUEUE
{
  int16_t inuse;
  int16_t current_position;
  int16_t increment;
  int16_t stop_value;
  int16_t start_value;
  int16_t centre_value;
  int16_t multi_address;
  int16_t single_invert;
};
QUEUE *ftn_queue = new QUEUE[numfpins];

struct CVPair
{
  uint16_t  CV;
  uint8_t   Value;
};

CVPair FactoryDefaultCVs [] =
{
  {CV_ACCESSORY_DECODER_ADDRESS_LSB, Accessory_Address},
  {CV_ACCESSORY_DECODER_ADDRESS_MSB, 0},
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_MSB, 0},
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_LSB, 0},
  {CV_DECODER_MASTER_RESET, 0},
  {CV_To_Store_SET_CV_Address, SET_CV_Address},
  {CV_To_Store_SET_CV_Address+1, 0},
  {CV_TIME_DISACTIVE_OUTPUT, TIME_DISACTIVE_OUTPUT}, // tempo disattivazione output dopo un comando per evitare sovraccarico
  {CONF_CV, CONF_CV_DEFAULT},
  {31, 15},  // durata impulso 1
  {34, 0},   // posizione all'accensione
  {36, 15},  // durata impulso 2
  {39, 0},   // posizione all'accensione
  {41, 15},  // durata impulso 3
  {44, 0},   // posizione all'accensione
  {46, 15},  // durata impulso 4
  {49, 0},   // posizione all'accensione
  {51, 15},  // durata impulso 5
  {54, 0},   // posizione all'accensione
  {56, 15},  // durata impulso 6
  {59, 0},   // posizione all'accensione
  {61, 15},  // durata impulso 7
  {64, 0},   // posizione all'accensione
  {66, 15},  // durata impulso 8
  {69, 0},   // posizione all'accensione

  {CV_SINGLE_INV, 0},
  {CV_SINGLE_INV+1, 0},
  {CV_SINGLE_INV+2, 0},
  {CV_SINGLE_INV+3, 0},
  {CV_SINGLE_INV+4, 0},
  {CV_SINGLE_INV+5, 0},
  {CV_SINGLE_INV+6, 0},
  {CV_SINGLE_INV+7, 0},

  {CV_MULTI_ADDRESS, Accessory_Address},
  {CV_MULTI_ADDRESS+1, 0},
  {CV_MULTI_ADDRESS+2, Accessory_Address+1},
  {CV_MULTI_ADDRESS+3, 0},
  {CV_MULTI_ADDRESS+4, Accessory_Address+2},
  {CV_MULTI_ADDRESS+5, 0},
  {CV_MULTI_ADDRESS+6, Accessory_Address+3},
  {CV_MULTI_ADDRESS+7, 0},
  {CV_MULTI_ADDRESS+8, Accessory_Address+4},
  {CV_MULTI_ADDRESS+9, 0},
  {CV_MULTI_ADDRESS+10, Accessory_Address+5},
  {CV_MULTI_ADDRESS+11, 0},
  {CV_MULTI_ADDRESS+12, Accessory_Address+6},
  {CV_MULTI_ADDRESS+13, 0},
  {CV_MULTI_ADDRESS+14, Accessory_Address+7},
  {CV_MULTI_ADDRESS+15, 0},
  /*
  {50, 4}, //F4 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {51, 25},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {52, 28},    //  Start Position Fx=0
  {53, 140},    //  End Position   Fx=1
  {54, 40},    //  Current Position
  {55, 4}, //F5 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {56, 25},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {57, 40},    //  Start Position Fx=0
  {58, 120},    //  End Position   Fx=1
  {59, 40},    //  Current Position
  {60, 4}, //F6 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {61, 25},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {62, 40},    //  Start Position Fx=0
  {63, 120},    //  End Position   Fx=1
  {64, 40},    //  Current Position
  {65, 4}, //F7 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {66, 25},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {67, 40},   //  Start Position Fx=0
  {68, 120},  //  End Position   Fx=1
  {69, 40},    //  Current Position
  {70, 2}, //F8 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {71, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {72, 28},   //  Start Position Fx=0
  {73, 140},  //  End Position   Fx=1
  {74, 28},    //  Current Position
  {75, 2}, //F9 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {76, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {77, 28},   //  Start Position Fx=0
  {78, 140},  //  End Position   Fx=1
  {79, 28},    //  Current Position
  {80, 2}, //F10 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {81, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {82, 28},   //  Start Position Fx=0
  {83, 140},  //  End Position   Fx=1
  {84, 28},    //  Current Position
  {85, 2}, //F11 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {86, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {87, 28},   //  Start Position Fx=0
  {88, 140},  //  End Position   Fx=1
  {89, 28},    //  Current Position
  {90, 2}, //F12 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {91, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {92, 28},   //  Start Position Fx=0
  {93, 140},  //  End Position   Fx=1
  {94, 28},    //  Current Position
  {95, 2}, //F13 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {96, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {97, 28},   //  Start Position Fx=0
  {98, 140},  //  End Position   Fx=1
  {99, 28},    //  Current Position
  {100, 2}, //F14 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {101, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {102, 28},   //  Start Position Fx=0
  {103, 140},  //  End Position   Fx=1
  {104, 28},    //  Current Position
  {105, 1}, //F15 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {106, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {107, 1},   //  Start Position Fx=0
  {108, 10},  //  End Position   Fx=1
  {109, 1},    //  Current Position
  {110, 0}, //F16 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {111, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {112, 1},   //  Start Position Fx=0
  {113, 10},  //  End Position   Fx=1
  {114, 1},    //  Current Position
//FUTURE USE
  {115, 0}, //F17 Config 0=On/Off,1=Blink,2=Servo,3=DBL LED Blink,4=Pulsed,5=fade
  {116, 1},    // Rate  Blink=Eate,PWM=Rate,Servo=Rate
  {117, 28},   //  Start Position Fx=0
  {118, 50},  //  End Position   Fx=1
  {119, 28},    //  Current Position
  */
};

uint8_t FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs)/sizeof(CVPair);

#if defined(SERIALCOM)

  #include <DccSerialCom.h>
  
  DccSerialCom SerialCom(FactoryDefaultCVIndex, CV_MULTI_ADDRESS, CV_SINGLE_INV);

  void notifyExecuteFunction(uint8_t function, uint8_t state) {
    exec_function(function, state);
  }
  
  uint16_t notifyGetCVnum(uint16_t index) {
    return FactoryDefaultCVs[index].CV;
  }

  uint16_t notifyGetCVval(uint16_t CV) {
    return Dcc.getCV(CV);
  }

  void notifySetCV(uint16_t CV, uint16_t value) {
    Dcc.setCV(CV, value);
  }
  
#endif

void notifyCVResetFactoryDefault()
{
  // Make FactoryDefaultCVIndex non-zero and equal to num CV's to be reset 
  // to flag to the loop() function that a reset to Factory Defaults needs to be done
  FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs)/sizeof(CVPair);
};

void setup()   //******************************************************
{
#if defined(DEBUG) | defined(SERIALCOM)
  Serial.begin(9600);
  SerialCom.init(Serial);
#endif
  int i;

  // initialize the digital pins as outputs
  for (i=0; i < numfpins; i++) {
      pinMode(FApins[i], OUTPUT);
      digitalWrite(FApins[i], 0);
      pinMode(FBpins[i], OUTPUT);
      digitalWrite(FBpins[i], 0);
     }
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 1);
  
  // Setup which External Interrupt, the Pin it's associated with that we're using 
  Dcc.pin(0, 2, 0);
  // Call the main DCC Init function to enable the DCC Receiver
  Dcc.init( MAN_ID_DIY, 100, FLAGS_OUTPUT_ADDRESS_MODE | FLAGS_DCC_ACCESSORY_DECODER, CV_To_Store_SET_CV_Address);

  delay(1000);
  
  #if defined(DECODER_LOADED)
  if ( Dcc.getCV(CV_DECODER_MASTER_RESET)== CV_DECODER_MASTER_RESET || Dcc.getCV(CV_ACCESSORY_DECODER_ADDRESS_LSB)== 255) 
  #endif  
  
     {
       for (int j=0; j < sizeof(FactoryDefaultCVs)/sizeof(CVPair); j++ )
         Dcc.setCV( FactoryDefaultCVs[j].CV, FactoryDefaultCVs[j].Value);
     }

  CV28.init();
  
  tim_delay = Dcc.getCV( CV_TIME_DISACTIVE_OUTPUT ) * 10;
  
  for ( i=0; i < numfpins; i++) {
   // Simple Pulsed Output based on saved Rate =10*Rate in Milliseconds
		 {
		   ftn_queue[i].inuse = 0;
       
       CVrefresh(i);
       
       // limita a 1 se maggiore
       if(ftn_queue[i].current_position > 1) {
          ftn_queue[i].current_position = 1;
          Dcc.setCV( 34+(i*5), 1);
       }
       
       if(ftn_queue[i].current_position) {
         digitalWrite(FApins[i], 1);
         digitalWrite(FBpins[i], 0);
         //digitalWrite(Rpins[i], 1);
       }
       else {
         digitalWrite(FApins[i], 0);
         digitalWrite(FBpins[i], 1);
         //digitalWrite(Rpins[i], 0);
       }
       if(CV28.GetPulse()) {
         delay(ftn_queue[i].increment);
         digitalWrite(FApins[i], 0);
         digitalWrite(FBpins[i], 0);
       }
       delay(tim_delay);

       ftn_queue[i].start_value = 0;
       ftn_queue[i].stop_value = ftn_queue[i].current_position;
       
		 }
  }
  digitalWrite(LED, 0);
}

void loop()   //**********************************************************************
{
  // Serial comunication
  #if defined(SERIALCOM)
    SerialCom.process();
  #endif
  
  //MUST call the NmraDcc.process() method frequently 
  // from the Arduino loop() function for correct library operation
  Dcc.process();
  
  for (int i=0; i < numfpins; i++) {
    
    if (ftn_queue[i].start_value == 1) {
      if (ftn_queue[i].stop_value != ftn_queue[i].current_position) {
        post_execution(i, ftn_queue[i].stop_value);
      }
    }
    
    if (ftn_queue[i].inuse==1)  {
        unsigned long time = millis() - temp[i];
        if(time >= ftn_queue[i].increment && ftn_queue[i].centre_value) {
          ftn_queue[i].centre_value = 0;
          
          if(CV28.GetPulse()) {
            digitalWrite(FApins[i], 0);
            digitalWrite(FBpins[i], 0);
          }
          
          if(CV28.GetSave()) Dcc.setCV( 34+(i*5), ftn_queue[i].current_position);
          
          //ftn_queue[i].current_position = !ftn_queue[i].current_position;
          digitalWrite(LED, 0);
        }
        if(time >= ftn_queue[i].increment + tim_delay)
          ftn_queue[i].inuse = 0;
    }
  }
}


void CVrefresh(uint8_t out) {
  if (CV28.GetMultiAdr()) {
    ftn_queue[out].multi_address = int (word(char (Dcc.getCV( CV_MULTI_ADDRESS+1+(out*2))), char (Dcc.getCV( CV_MULTI_ADDRESS+(out*2)))));
  }
  else {
    ftn_queue[out].multi_address = 0;
  }
  ftn_queue[out].single_invert = int (Dcc.getCV( CV_SINGLE_INV+out));
  ftn_queue[out].increment = 10 * int (char (Dcc.getCV( 31+(out*5))));
  ftn_queue[out].current_position = int (Dcc.getCV( 34+(out*5)));
}

// This function is called by the NmraDcc library
// when a DCC ACK needs to be sent
// Calling this function should cause an increased 60ma current drain
// on the power supply for 6ms to ACK a CV Read
/*void notifyCVAck(void) {
  //digitalWrite( 3, HIGH );
  digitalWrite( LED, HIGH );
  delay(6);
  //digitalWrite( 3, LOW );
  digitalWrite( LED, LOW );
}*/

void notifyCVChange( uint16_t CV, uint8_t Value) {
  digitalWrite( LED, HIGH );
  delay(20);
  digitalWrite( LED, LOW );
  tim_delay = Dcc.getCV( CV_TIME_DISACTIVE_OUTPUT ) * 10;
  CV28.init();
  for (uint8_t i=0; i < numfpins; i++) {
    CVrefresh(i);
  }
}

extern void notifyDccAccTurnoutOutput( uint16_t Addr, uint8_t Direction, uint8_t OutputPower ) {
  if (CV28.GetMultiAdr()) {
    for (uint8_t i=0; i < numfpins; i++) {
      if (ftn_queue[i].multi_address == Addr) {
        if (ftn_queue[i].single_invert) {
          exec_function(i, Direction ? 0 : 1);
        }
        else {
          exec_function(i, Direction ? 1 : 0);
        }
      }
    }
  }
  else {
    uint16_t Current_Decoder_Addr = Dcc.getAddr();
    if ( Addr >= Current_Decoder_Addr && Addr < Current_Decoder_Addr+numfpins) { //Controls Accessory_Address+8
      if (ftn_queue[Addr-Current_Decoder_Addr].single_invert) {
        exec_function(Addr-Current_Decoder_Addr, Direction ? 0 : 1);
      }
      else {
        exec_function(Addr-Current_Decoder_Addr, Direction ? 1 : 0);
      }
    }
  }
}

/*
extern void notifyDccAccState( uint16_t Addr, uint16_t BoardAddr, uint8_t OutputAddr, uint8_t State) {
  uint16_t Current_Decoder_Addr;
  uint8_t Bit_State;
  Current_Decoder_Addr = Dcc.getAddr();
  Bit_State = OutputAddr & 0x01;
  
  if ( Addr >= Current_Decoder_Addr && Addr < Current_Decoder_Addr+numfpins) { //Controls Accessory_Address+3
#ifdef DEBUG
	 Serial.print("Addr = ");
	 Serial.println(Addr);
   Serial.print("BoardAddr = ");
	 Serial.println(BoardAddr);
	 Serial.print("Bit_State = ");
	 Serial.println(Bit_State);
#endif
	exec_function(Addr-Current_Decoder_Addr, Bit_State );
  } 
}
*/

void exec_function (uint8_t function, uint8_t FuncState) {
      ftn_queue[function].start_value = 1;
      ftn_queue[function].stop_value = FuncState;
}

void post_execution(uint8_t function, uint8_t FuncState) {
  if (ftn_queue[function].inuse == 0) {  //First Turn On Detected
    digitalWrite(LED, 1);
    if(FuncState ^ CV28.GetInv()) {
      digitalWrite(FApins[function], 1);
      digitalWrite(FBpins[function], 0);
      //digitalWrite(Rpins[function], 1);
    }
    else {
      digitalWrite(FApins[function], 0);
      digitalWrite(FBpins[function], 1);
      //digitalWrite(Rpins[function], 0);
    }
    ftn_queue[function].current_position = FuncState;
    temp[function] = millis();
    ftn_queue[function].inuse = 1;
    ftn_queue[function].centre_value = 1;
  }
}
