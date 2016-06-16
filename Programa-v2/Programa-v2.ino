const int LMPIN= A0;
const int DHTPIN= 2;
const int BUTTONPIN= 3;
int mode= 0;
int holder= 0;
int counter= 0;
int namecounter= 0;
int lmv[10];

#include "DHT.h"

#include <SFE_BMP180.h>
#include <Wire.h>

#include <LiquidCrystal_I2C.h>

DHT dht(DHTPIN, DHT22);

SFE_BMP180 pressure;
double baseline;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup(){
  pinMode(DHTPIN, INPUT);
  pinMode(BUTTONPIN, INPUT);
  
  dht.begin();

  if (pressure.begin()){
    Serial.println("BMP180 init success");
    baseline = getPressure();
  }

  lcd.begin();
  lcd.backlight();

  Serial.begin(9600);
}
 
void loop(){                     // run over and over again
  delay(50);
  read_button();
  //Serial.print(mode);
  switch(mode){
    case 0: display_names(); break;
    case 1: read_lm(true); break;
    case 2: read_lm(false); break;
    case 3: read_dht(); break;
    case 4: read_bmp(); break;
  }
}

void display_names(){

  const char estacao[]= "    Estacao          Insper     ";
  const char insper[]="     Insper     ";
  const char meteorologica[]="  Meteorologica ";
  const char names[]= "  Meteorologica Alexandre Young   Sabrina Simao   Paulo Tozzo   ";
  const int limit= strlen(names);
  String temp= "";
  String temp2="";
  int i= 0;
  if(namecounter < 30){
    
    lcd.setCursor(0,0);
    lcd.print(estacao);
    lcd.setCursor(0,1);
    lcd.print(meteorologica);
  }else{
    while (i!=16){
      if ( ( (namecounter-30) < 16 ) or ( (limit*3+30-namecounter) < 16)    ){
          temp2+= estacao[(i+(namecounter-30) )%32];
        }
      
      temp+=names[ (i+((namecounter-30)/3))%limit];
      i+= 1;
    }

    lcd.setCursor(0,0);
    lcd.print(temp2);
    lcd.setCursor(0,1);
    lcd.print(temp);
  }

  namecounter+= 1;
  namecounter%= limit*3+30;
}

void read_dht(){
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  //conversão start
  const float dhti= 1.9386022720053262;
  h= h*0.97385071 - dhti;
  //conversão end

  String out= "Humidade: ";
  out+= h;
  out+= "%";
  lcd.setCursor(0,0);
  lcd.print(out);
  lcd.setCursor(0,1);
  lcd.print("                ");
}

void read_lm(bool celsius){

  long voltage= analogRead(LMPIN);  
  
  voltage= (voltage*5000)/1024; 
  
  Serial.print("\nmilivolts: "); Serial.print(voltage);


  lmv[counter]= voltage;
  double media= 0.0;
  int i= 0;
  while( i != 10 ){
    media+= lmv[i];
    i+=1;
  }
  media/= 10;

  //conversão - start
  const float lmi= 0.4874947868695475;
  media= media/9.2310178 + lmi;
  //end

  String out= "";
  if(celsius){
    int temp= int(media);
    int temp2= int(media*10)%10;
  
    out+= temp;
    out+= ".";
    out+= temp2;
    out+= " Celsius";
  }else{

    //converter media para fahrenheit
    media= (media*9 +160)/5;

    int temp= int(media);
    int temp2= int(media*10)%10;
    
    out+= temp;
    out+= ".";
    out+= temp2;
    out+= " Fahrenheit";
  }
    out+="                ";
    lcd.setCursor(0,0);
    lcd.print("Temperatura:    ");
    lcd.setCursor(0,1);
    lcd.print(out);
    
  counter+= 1;
  counter%= 10;
}

void read_button(){

  int push= digitalRead(BUTTONPIN);

  if ( holder != push){

    holder= push;
    if (holder == 0){
      mode+= 1;
      mode%= 5;
    }
  }
}

void read_bmp(){
  double a,P;
  
  // Get a new pressure reading:

  P = getPressure();

  // Show the relative altitude difference between
  // the new reading and the baseline reading:

  a = pressure.altitude(P,baseline);
  
  Serial.print("relative altitude: ");
  if (a >= 0.0) Serial.print(" "); // add a space for positive numbers
  Serial.print(a,1);
  Serial.print(" meters, ");
  if (a >= 0.0) Serial.print(" "); // add a space for positive numbers
  Serial.print(a*3.28084,0);
  Serial.println(" feet");

  String out= "Pressao: ";
  out+= P;
  out+=" ";
  lcd.setCursor(0,0);
  lcd.print(out);
  lcd.setCursor(0,1);
  lcd.print("milibar         ");
  //lcd.print("                ");
}

double getPressure(){
  char status;
  double T,P,p0,a;

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}
