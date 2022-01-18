#include <Adafruit_NeoPixel.h>
#include <si5351mcu.h>    //Si5351mcu library
Si5351mcu Si;             //Si5351mcu Board
#define PULSE_PIN     13 
#define NOISE         50
#define ROWS          20  //num of row MAX=20
#define COLUMNS       14  //num of column
#define DATA_PIN      9   //led data pin
#define STROBE_PIN    5   //MSGEQ7 strobe pin
#define RESET_PIN     7   //MSGEQ7 reset pin
#define NUMPIXELS    ROWS * COLUMNS

#define PEAK_DELAY_PIN 66 //A12
#define PEAK_HORSE_PIN 55 //A1
#define VOLUME_PIN 2  //khong co pin arduino
#define COLOR_COLUMN_PIN 58 //A4
#define COLOR_PEAK_PIN 60 //A6
#define DELAY_PIN 62   //A8
#define BRIGHTNESS_PIN 64
#define AUTO_PIN 11   //D11

#define SIGNAL_PIN_0 54 //A0
#define SIGNAL_PIN_1 56 //A2
struct Point{
  char x, y;
  char  r,g,b;
  bool active;
};
struct TopPoint{
  int position;
  int peakpause;
};
Point spectrum[ROWS][COLUMNS];
TopPoint peakhold[COLUMNS];
unsigned long timer;
unsigned long timeauto = 0;

int spectrumValue[COLUMNS];
long int counter = 0;
int numberCaseEffect = 0;
int maxNumberEffect = 10;
int initcolor;

int long pwmpulse = 0;
bool toggle = false;
int long time_change = 0;
int effect = 0;
int peakhorse = 3;
int peakdelay = 70;
int brightness_led =255;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);

void topSinking(void);
void read_signal_music(void);
void clearspectrum(void);
void pulse_for_si5351(void);
void controll_reset_si5351(void);
bool check_auto(void);
void set_led_follow_music(void);
void collumn_giam(void);
void read_data_hand();
void flushMatrix();
uint8_t WheelR(int);
uint8_t WheelG(int);
uint8_t WheelB(int);

void setup() 
{
   Si.init(25000000L);
   Si.setFreq(0, 104570);
   Si.setFreq(1, 166280);
   Si.setPower(0, SIOUT_8mA);
   Si.setPower(1, SIOUT_8mA);
   Si.enable(0);
   Si.enable(1);
   pinMode      (STROBE_PIN,    OUTPUT);
   pinMode      (RESET_PIN,     OUTPUT);
   pinMode      (DATA_PIN,      OUTPUT);
   pinMode      (PULSE_PIN,     OUTPUT);

   //Pin từ biến trở điều chỉnh
//   pinMode      (SIGNAL_PIN_0,    INPUT);
//   pinMode      (SIGNAL_PIN_1,     INPUT);
//   pinMode      (PEAK_DELAY_PIN,      INPUT);
//   pinMode      (COLOR_COLUMN_PIN,     INPUT);
//   pinMode      (COLOR_PEAK_PIN,     INPUT);
//   pinMode      (DELAY_PIN ,     INPUT);
   pinMode      (AUTO_PIN ,     INPUT);

   //Khởi tạo si5351
   digitalWrite(PULSE_PIN, HIGH);
   delay(100);
   digitalWrite(PULSE_PIN, LOW);
   delay(100);
   digitalWrite(PULSE_PIN, HIGH);
   delay(100);
   digitalWrite(PULSE_PIN, LOW);
   delay(100);
   digitalWrite(PULSE_PIN, HIGH);
   delay(100);
   
   //pixels.setBrightness(20); //set Brightness
   pixels.setBrightness(255);
   pixels.begin();
   pixels.show();  
   digitalWrite (RESET_PIN,  LOW);
   digitalWrite (STROBE_PIN, LOW);
   delay        (1);  
   digitalWrite (RESET_PIN,  HIGH);
   delay        (1);
   digitalWrite (RESET_PIN,  LOW);
   digitalWrite (STROBE_PIN, HIGH);
   delay        (1);
}

void loop() 
{    
  counter++;  
//  if(check_auto){
  clearspectrum();    // reset led về false
  //Đọc data khi ở chế độ hand
  read_data_hand();
//  if (millis() - pwmpulse > 3000){
//    toggle = !toggle;
//    digitalWrite(PULSE_PIN, toggle);
//    pwmpulse = millis();
//  }
  //Set độ sáng cho led
  pixels.setBrightness(brightness_led); 
  //Pulse sườn xuống cho chân reset si5351
  controll_reset_si5351();
        
  //Read signal music
  read_signal_music(); 
      
  if(check_auto() ==true){
    //nếu auto thì set led theo màu sắc
//    switch(numberCaseEffect){
//        case 0: {initcolor =245; break;}
//        case 1: {initcolor =170;break;}
//        case 2: {initcolor =150;break;}
//        case 3: {initcolor =200;break;}
//        case 4: {initcolor =10;break;}
//        case 5: {initcolor =40;break;}
//        case 6: {initcolor =170;break;}
//        case 7: {initcolor =30;break;}
//        case 8: {initcolor =70;break;}
//        case 9: {initcolor =130;break;}
//        case 10: {initcolor =100;break;}
//      }
     set_led_follow_music(170);
  }
  else {
     //Set led follow with music
//     switch(numberCaseEffect){
//        case 0: {initcolor =245;;break;}
//        case 1: {initcolor =220;;break;}
//        case 2: {initcolor =150;;break;}
//        case 3: {initcolor =200;;break;}
//        case 4: {initcolor =10;;break;}
//        case 5: {initcolor =40;;break;}
//        case 6: {initcolor =170;;break;}
//        case 7: {initcolor =30;;break;}
//        case 8: {initcolor =70;;break;}
//        case 9: {initcolor =130;;break;}
//        case 10: {initcolor =100;;break;}
//      }
     set_led_follow_music(170);
  }
  //Show led
  flushMatrix();
  if(counter % peakdelay ==0)topSinking(); // thời gian delay để giảm độ rơi của đỉnh
}//End Loop 
//=====================================================================================


//=====================================================================================

//=====================================================================================
//topSinking đỉnh rơi
void topSinking()
{
  for(int j = 0; j < ROWS; j++)
  {
    if(peakhold[j].position > 0 && peakhold[j].peakpause <= 0) peakhold[j].position--;
    else if(peakhold[j].peakpause > 0) peakhold[j].peakpause--;       
  } 
}
//=====================================================================================
//=====================================================================================
void clearspectrum()
{
  for(int i = 0; i < ROWS; i++)
  {
    for(int j = 0; j < COLUMNS; j++)
    {
    spectrum[i][j].active = false;  
    } 
  }
}
//=====================================================================================
//=====================================================================================
void flushMatrix()
{
  for(int j = 0; j < COLUMNS; j++)
  {
    if( j % 2 != 0)
    {
      for(int i = 0; i < ROWS; i++)
      {
        if(spectrum[ROWS - 1 - i][j].active)
        {
          pixels.setPixelColor(j * ROWS + i, pixels.Color(
          spectrum[ROWS - 1 - i][j].r, 
          spectrum[ROWS - 1 - i][j].g, 
          spectrum[ROWS - 1 - i][j].b));         
        }
        else
        {
          pixels.setPixelColor( j * ROWS + i, 0, 0, 0);  
        } 
      }
    }
    else
    {
      for(int i = 0; i < ROWS; i++)
      {
        if(spectrum[i][j].active)
        {
          pixels.setPixelColor(j * ROWS + i, pixels.Color(
          spectrum[i][j].r, 
          spectrum[i][j].g, 
          spectrum[i][j].b));     
        }
        else
        {
          pixels.setPixelColor( j * ROWS + i, 0, 0, 0);  
        }
      }      
    } 
  }
  pixels.show();
}
//=====================================================================================
//=====================================================================================
void controll_reset_si5351(){
  digitalWrite(RESET_PIN, HIGH);
  delayMicroseconds(3000);
  digitalWrite(RESET_PIN, LOW);
}
//======================================================================================
//======================================================================================
void read_signal_music(){
  for(int i=0; i < COLUMNS; i++){ 
    digitalWrite(STROBE_PIN, LOW);
    delayMicroseconds(1000);
    spectrumValue[i] = analogRead(SIGNAL_PIN_1);
    if(spectrumValue[i] < 160)spectrumValue[i] = 0;
    //Ghim giá trị từ 0-1023
    spectrumValue[i] = constrain(spectrumValue[i], 0, 1023);
    spectrumValue[i] = map(spectrumValue[i], 0, 1023, 1, ROWS);
    i++;
    spectrumValue[i] = analogRead(SIGNAL_PIN_0);
    if(spectrumValue[i] < 160)spectrumValue[i] = 0;
    spectrumValue[i] = constrain(spectrumValue[i], 0, 1023);
    spectrumValue[i] = map(spectrumValue[i], 0, 1023, 1, ROWS);
    digitalWrite(STROBE_PIN, HIGH);  
  }
}
//======================================================================================
//======================================================================================
void set_led_follow_music(int color){
  for(int j = COLUMNS; j > 0; j--){
      for(int i = 0; i < spectrumValue[j]; i++){ 
        spectrum[i][COLUMNS - 1 - 14 + j].active = 1;     //Led được set sáng chờ show
        spectrum[i][COLUMNS - 1 - 14 + j].r = WheelR(color - 17*i);           //COLUMN Color red
        spectrum[i][COLUMNS - 1 - 14 + j].g = WheelG(color - 17*i);         //COLUMN Color green
        spectrum[i][COLUMNS - 1 - 14 + j].b = WheelB(color - 17*i);            //COLUMN Color blue
      }
      if(spectrumValue[j] - 1 > peakhold[j].position)
      {
        spectrum[spectrumValue[j] - 1][COLUMNS - 1 - j].r = 0; 
        spectrum[spectrumValue[j] - 1][COLUMNS - 1 - j].g = 0; 
        spectrum[spectrumValue[j] - 1][COLUMNS - 1 - j].b = 0;
        peakhold[j].position = spectrumValue[j] - 1;
        peakhold[j].peakpause = peakhorse; //set thời gian chờ rơi của đỉnh
      }
      else
      {
        spectrum[peakhold[j].position][COLUMNS - 1 - 14 + j].active = 1;
        spectrum[peakhold[j].position][COLUMNS - 1 - 14 + j].r = 170;  //Peak Color red
        spectrum[peakhold[j].position][COLUMNS - 1 - 14 + j].g = 150;  //Peak Color green
        spectrum[peakhold[j].position][COLUMNS - 1 - 14 + j].b = 0;   //Peak Color blue
      }
  }  
}
//=====================================================================================
//=====================================================================================
bool check_auto(){
  return digitalRead(AUTO_PIN);
}
//=====================================================================================
//=====================================================================================
void read_data_hand(){
  //Đọc độ sáng
  brightness_led =20; //map(analogRead(BRIGHTNESS_PIN), 0, 1023, 20, 255);

  //Đọc độ trễ đỉnh
  peakhorse = 1;//map(analogRead(PEAK_HORSE_PIN), 0,1023, 1,3);
  
  //Đọc độ trễ khi rơi đỉnh
  peakdelay = 2;//map(analogRead(PEAK_DELAY_PIN), 0,1023, 1,5);

  //Đọc peakcolor
  //Đọc .....
}
uint8_t WheelR(int WheelPos) {
  if(WheelPos<0){
    WheelPos = 255+ WheelPos;
  }
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return 255 - WheelPos * 3;
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return 0;
  }
  WheelPos -= 170;
  return WheelPos * 3;
}
uint8_t WheelG(int WheelPos) {
  if(WheelPos<0){
    WheelPos = 255+ WheelPos;
  }
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return 0;
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return WheelPos * 3;
  }
  WheelPos -= 170;
  return  255 - WheelPos * 3;
}
uint8_t WheelB(int WheelPos) {
  if(WheelPos < 0){
    WheelPos = 255+ WheelPos;
  }
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return WheelPos * 3;
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return 255 - WheelPos * 3;
  }
  WheelPos -= 170;
  return 0;
}
