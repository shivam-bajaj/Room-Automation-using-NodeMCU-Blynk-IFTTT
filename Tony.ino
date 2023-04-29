
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Keypad.h>

char auth[] = "XXXXXXXXXX"; // enter auth pin
char ssid[] = ""; 
char pass[] = "";

#define DHTPIN 2   // dht 11 pin
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WidgetLCD lcd(V0);
WidgetTerminal terminal(V1);
const byte n_rows = 4;
const byte n_cols = 4;

char keys[n_rows][n_cols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
// keypad pins
byte colPins[n_rows] = {D3, D2, D1, D0};
byte rowPins[n_cols] = {D7, D6, D5, D4};


Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, n_rows, n_cols);

// light and fan relay are connected directy
int smokeA0 = A0,door_pin=D8;  //smoke and relay to em lock
String dh;// storing temp and humidty
String p="6848";
String pin;
boolean login=false,door=true;

void send_sensor(){

  int t = dht.readTemperature();
  int h = dht.readHumidity();
  int s = analogRead(smokeA0);
  dh="T "+String(t)+"°C || H "+String(h)+"%";
  lcd.print(0,0,dh);
  Blynk.virtualWrite(V2,s);        // Gauge

}

BLYNK_WRITE(V1)
{
// terminal code
if (login == false){
  if ( String("bajju")==param.asString()){
  terminal.clear();
  terminal.println("Enter 1 to see your door pin");
  terminal.println("Enter 2 to see local IP");
  terminal.println("Enter 3 to disbale/Enable door pin");
  terminal.println("Enter 4 to update the sensors");
  terminal.flush();
  login = true;
}
else{
  terminal.clear();
  terminal.println("wrong password");
  terminal.println("Enter password");
}
}

//  use if/ else instead of switch , "1" = param.asString();
else{
  int input = param.asInt();

  switch(input){
    case 1:
  terminal.println(p);
  terminal.flush();
  delay(3000);
  terminal.clear();
  login=false;
  terminal.println("Enter password");
  break;

   case 2:
   terminal.clear();
   terminal.println(WiFi.localIP());
   login=false;
   terminal.println("Enter password");
   break;

  case 3:
  if( door == true){
    door=false;
    terminal.println("Disable keypad lock");
    lcd.print(0,1," keypad disable ");
  }
  else{
    door=true;
    terminal.println("Enable keypad lock");
    lcd.print(0,1," Enter door pin ");
  }
  login=false;
  terminal.println("Enter password");
  break;

  case 4 :
  send_sensor();
  login=false;
  terminal.println("Sensor updated");
  terminal.println("Enter password");
  break;

  default:
  terminal.println("invalid key");
  terminal.println("Enter password");
  login=false;
  }
terminal.flush();

}
}

void setup()
{
  // Debug console
  Blynk.begin(auth, ssid, pass);
  Serial.begin(9600);
  pinMode(smokeA0 , INPUT);
  pinMode(door_pin,OUTPUT);
  dht.begin();
  terminal.clear();
  lcd.clear();

  if (Blynk.connect()== true ){
  terminal.println("Connected to bAjjU");
  terminal.println(WiFi.localIP());
  terminal.println("Login .... ");
  terminal.flush();
  Blynk.notify("Boss I am connected");
  if (door == true){
  lcd.print(0,1," Enter door pin ");
  }
  else{
    lcd.print(0,1,"keypad locked");
  }
  send_sensor();

  timer.setInterval(600000L,send_sensor);   // for 15 min.

  }

}


void loop()
{
  Blynk.run();
  timer.run();
  if( door == true){
    char key = myKeypad.getKey();
 // 'D' to submit and 'C' to reset
 if (key != NULL & key !='D' & key!='C'){
    pin=pin+key;
    lcd.clear();
    lcd.print(0,0,dh);
    lcd.print(0,1,pin);

    if ( pin.length() > 7){
      lcd.clear();
      lcd.print(0,0,dh);
      lcd.print(0,1," Enter door pin ");
      pin="";
    }
}
if( key == 'D'){
  Serial.println(pin);
  if (pin == p){
      lcd.clear();
      lcd.print(0,0,dh);
      lcd.print(0,1,"Door Unlock");
      pin="";
      digitalWrite(door_pin,HIGH);
      Blynk.setProperty(door_pin,"onLabel");
      delay(7000);
      digitalWrite(door_pin,LOW);
      Blynk.setProperty(door_pin,"offLabel");
      lcd.print(0,1," Enter door pin ");
  }
  else{
    lcd.clear();
    lcd.print(0,0,dh);
    lcd.print(0,1,"invalid pin    ");
    pin="";
  }
}

if( key == 'C'){
  pin="";
  lcd.clear();
  lcd.print(0,0,dh);
  lcd.print(0,1," Enter door pin ");

}



  }
}
