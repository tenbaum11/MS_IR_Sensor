// Example sketch showing how to control ir devices
// An IR LED must be connected to Arduino PWM pin 3.
// An optional ir receiver can be connected to PWM pin 8. 
// All receied ir signals will be sent to gateway device stored in VAR_1.
// When binary light on is clicked - sketch will send volume up ir command
// When binary light off is clicked - sketch will send volume down ir command
//   202;22;1;0;32;551502015    
#include <MySensor.h>
#include <SPI.h>
#include <IRLib.h>



#define N0_IRrx 1
#ifdef N0_IRrx==1
 #define IRrxflag
 int RECV_PIN = 5;
 IRrecv irRecv(RECV_PIN);
 IRdecode irDecoderNEC;
 unsigned int irBuffer[RAWBUF];
#else
 
#endif

//#define N0_RCrx 1
#ifdef N0_RCrx
  #include <RCSwitch.h>
  RCSwitch RCsend = RCSwitch();
  unsigned int pulsetime = 140;  // RCremote pulse out
  byte etekState = 0;
  byte etekID = 4;
#else
#endif

//#define N0_RHrx 1
#ifdef N0_RHrx
  #include <RHDatagram.h>
  #include <RH_ASK.h>
  RH_ASK RHdriver(1200,2, 88);  //(speed, RXpin, TXpin, ptt;IN, inverted)
  RHDatagram RHmanager(RHdriver, 2);
#else
#endif

#define DEBUG 1
#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
#endif
//DEBUG_PRINT ("I think I'm here");",`





#define CHILD_1  3  // childId
#define CHILD_2  4  // childId
#define CID_IR  22  // childId
#define CID_IR2  23  // childId
#define CID_EtekA   12
#define CID_EtekB   13
#define CID_EtekC   14
#define CID_EtekD   15
#define CID_EtekE   16

IRsend irsend;
IRsendNEC irSendNEC;


const byte RX433_PIN = 2; 
const byte TX433_PIN = 4; 
const byte TXPWR_PIN = 6; 

MySensor gw;
MyMessage msg(CHILD_1, V_VAR3);
MyMessage msg2(CHILD_2, V_LIGHT);
//V_IR_SEND	32	Send out an IR-command
//V_IR_RECEIVE	33

// *****************************************
// ETEK CODES
// *****************************************
#ifdef N0_RCrx
const int ELEMENT_CNT = 20;
typedef struct {
   char CodeArr[16];
   byte etekStateArr;
   byte etekIDArr;
} CODES;

const CODES elementArr [ELEMENT_CNT] PROGMEM = {
  {"F0FF0FFF0101",1,0}, {"F0FF0FFF1001",1,1}, {"F0FF0FF10001",1,2}, {"F0FFFF1F0001",1,3}, {"F0FFF1FF0001",1,4},
  {"F0FFFFFF0101",1,0}, {"F0FFFFFF1001",1,1}, {"F0FFFFF10001",1,2}, {"F0FF0F1F0001",1,3}, {"F0FF01FF0001",1,4},
  {"F0FF0FFF0110",0,0}, {"F0FF0FFF1010",0,1}, {"F0FF0FF10010",0,2}, {"F0FFFF1F0010",0,3}, {"F0FFF1FF0010",0,4},
  {"F0FFFFFF0110",0,0}, {"F0FFFFFF1010",0,1}, {"F0FFFFF10010",0,2}, {"F0FFFF1F0010",0,3}, {"F0FF01FF0010",0,4},
};
#else
#endif

//CODES Item1;
//memcpy_P (&Item1, &elementArr[j+10], sizeof Item1);
//RCsend.sendTriState(Item1.CodeArr); 
long Code2;





void setup()  
{  

  gw.begin(incomingMessage, 212, false);
  gw.sendSketchInfo("IR Sensor", "1.0");
  gw.present(CHILD_1, S_LIGHT);
  gw.present(CHILD_2, S_LIGHT);
  gw.present(CID_IR, S_IR);
  //gw.present(CID_IR2, S_IR);
  if(N0_IRrx==1){
    irRecv.enableIRIn(); // Start the ir receiver
    irDecoderNEC.UseExtnBuf(irBuffer);
  }
}


void loop() 
{
    gw.process();
    if(N0_IRrx==1){
   
        if (irRecv.GetResults(&irDecoderNEC)) {
        Serial.println(F("Rcvd REMOTE IR"));
        irDecoderNEC.decode();
        irDecoderNEC.DumpResults();
            
        char buffer[10];
        sprintf(buffer, "%08lx", irDecoderNEC.value);
        // Send ir result to gw
        gw.send(msg.set(buffer));
        int test=1;
        irRecv.resume();    
        }

  }
  


}  // end main loop



void incomingMessage(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  
 if (message.type==V_IR_SEND) {
   //Serial.println("wprlomg"); 
   Serial.println(F("Rcvd SEND IR"));
   Serial.print(F("TYPE:"));
   Serial.println(message.type);
   Serial.print(F("DATA:"));
   Serial.println(message.data);
   //202;3;1;0;32;551502015
      Code2 = message.getULong();  // <----- here
     //irSendNEC.send(Code); delay(2500);
      irsend.send(NEC, Code2, 32); delay(500);
     if(N0_IRrx==1){ irRecv.enableIRIn(); }
  }
  else if (message.type==V_IR_RECEIVE) {
   //Serial.println("wprlomg"); 
   //202;3;1;0;32;551502015
     long Code = message.getULong();  // <----- here
     //delay(100);
     //irSendNEC.send(Code); delay(2500);
     irsend.send(NEC, Code, 32); delay(500);
     
     if(N0_IRrx==1){ irRecv.enableIRIn(); }
  }
  else if (message.type==V_LIGHT) {
     int incomingRelayStatus = message.getInt();
     if (incomingRelayStatus == 1) {
       
       irSendNEC.send(0x20DF40BF); delay(1000);
       //Serial.println("volup");
       //    202;3;1;0;2;    
       // 202;3;1;0;2;1 //  
       //    202;3;1;0;32;551520375      
       // 202;3;1;0;32;551520375
      //irsend.send(NEC, 0x20df40bf, 32); // Vol up yamaha ysp-900
     } else {
       irSendNEC.send(0x20DFC03F); delay(1000); 
       // 202;3;1;0;2;0  // 551,534,655
      //irsend.send(NEC, 0x1EE1F807, 32); // Vol down yamaha ysp-900
     }
     // Start receiving ir again...
    if(N0_IRrx==1){ irRecv.enableIRIn(); }
  }
  else{
   Serial.println(F("unrecognized command")); 
  }
}
    

