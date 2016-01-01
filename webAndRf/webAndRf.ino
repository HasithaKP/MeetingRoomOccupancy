#include <SoftwareSerial.h>
#include <VirtualWire.h>

SoftwareSerial esp(10,11);

const int transmit_pin = 3;

# define SSID "99XT_Guest"
# define Pass "ext@99xt"
# define IP "184.106.153.149"

# define chipEnable 9
# define rf A4
# define wifi A3
# define motionLight A2

# define mic A4
int noiseDifference = 100;
int soundInc = 0;

float temp;
# define tempPin A3

# define pir 2
volatile int motionCount = 0;
bool state1 = false;
bool state2 = false;

# define ldr A1
int light =0;

unsigned int previousTime =0;
int timeout = 60000;

void setup() {

 pinMode(mic,INPUT);
 
  pinMode(pir,INPUT);
  digitalWrite(2,HIGH);
  attachInterrupt(0,motion,FALLING);
 
 pinMode(ldr,INPUT); 
 pinMode(tempPin,INPUT);
 pinMode(chipEnable,OUTPUT);
 
 digitalWrite(chipEnable,LOW);
 Serial.begin(9600);
 
 vw_set_tx_pin(transmit_pin);
 vw_setup(2000);  
 
 esp.begin(9600);
 setESP();
}

void loop() {

 unsigned int currentTime = millis();
 if((currentTime-previousTime)<timeout){
  
    lightLevel();
   temperatureReading();
   soundLevel();
  }

 if((currentTime-previousTime)>timeout){

    previousTime = currentTime;
    rfTransmission();  
    updateESP();
  }
}

void lightLevel(){
  light = analogRead(ldr);
  }

void temperatureReading(){
  
  temp = analogRead(tempPin);
  temp = temp * 0.48828125;
  }
void motion(){
  
  motionCount++;
  digitalWrite(motionLight,!(digitalRead(motionLight)));
  }

void soundLevel(){
  
  int i=  analogRead(mic);
  delay(50);
  if(i<524-noiseDifference || i>524+noiseDifference){
  //Serial.println(i);
  soundInc++;
 }
  
  }
void setESP(){
  esp.println("AT");
  readEsp(1000, "Send AT:");
  esp.println("AT+CIOBAUD=9600");
  readEsp(1000,"Set baudrate");
  esp.println("AT+CWMODE=3");
  readEsp(1000,"Set mode");
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=Pass;
  cmd+="\"";
  esp.println(cmd);
  readEsp(3000,"Connect to WiFi");
  esp.println("AT+CIFSR");
  readEsp(3000,"Check IP");
  esp.println("AT+CIPMUX=1");
  readEsp(500,"Set multiple connections");
  }
void updateESP(){
  
  digitalWrite(wifi,HIGH);
  digitalWrite(chipEnable,HIGH);
  delay(500);
  Serial.println(light);
  Serial.println(temp);
  Serial.println(motionCount);
  Serial.println(soundInc);
  

  String cmd = "AT+CIPSTART=4,\"TCP\",\"";
  cmd += "184.106.153.149";
  cmd += "\",80";
  esp.println(cmd);
  readEsp(2000,"Connect to thinkspeak");
  String str1 = "GET /update?key=UATXNCJ6374OSGZO&field1=" + String(light) + "&field2="+String(temp)+ "&field3="+String(motionCount)+"&field4="+String(soundInc)+"\r\n"+"\r\n";
  String cmd1 = "AT+CIPSEND=4,";
  cmd1 += str1.length();
  esp.println(cmd1);

  if (esp.find(">"))
  {
    esp.println(str1);
    readEsp(2000,"Updating to thinkspeak");
    }
  else Serial.println ("Couldn't find");
  
  digitalWrite(wifi,LOW);
  light = 0;
  temp = 0;
  motionCount = 0;
  soundInc = 0;  
  digitalWrite(chipEnable,LOW);
}

  bool readEsp(int time1,String cmnds){
  String catchResponce;
  Serial.println(cmnds);
  delay(time1);
  if (esp.available())
  {
    while(esp.available())
    {
      catchResponce=esp.readString();
     
      }
      Serial.print ("Responce: ");
      Serial.println(catchResponce);
    }
    
  }
void rfTransmission(){
  digitalWrite(rf, HIGH);
  for(int i=0;i<5;i++){
    String msg = "light="+String(light)+"&temperature="+String(temp)+"&motions="+String(motionCount)+"&sounds="+String(soundInc);
    char sender[msg.length()];
    //int msgLength = msg.length();
    //char char_array[msgLength];
    msg.toCharArray(sender,msg.length());
    vw_send((uint8_t *)sender, strlen(sender));
    vw_wait_tx(); // Wait until the whole message is gone  
  }
  Serial.println("Message sent complete");
  digitalWrite(rf, LOW);
}
