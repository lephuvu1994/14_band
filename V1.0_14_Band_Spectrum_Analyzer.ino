#include <Adafruit_NeoPixel.h>
#include <si5351mcu.h>    //Si5351mcu library
Si5351mcu Si;             //Si5351mcu Board
#define PULSE_PIN     13 
#define NOISE         50
#define ROWS          20  //num of row MAX=20
#define COLUMNS       14  //num of column
#define DATA_PIN      9   //led data pin
#define STROBE_PIN    6   //MSGEQ7 strobe pin
#define RESET_PIN     7   //MSGEQ7 reset pin
#define NUMPIXELS    ROWS * COLUMNS

#define PEAK_DELAY_PIN 0
#define PEAK_HORSE_PIN 1
#define VOLUME_PIN 2
#define COLOR_COLUMN_PIN 3
#define COLOR_PEAK_PIN 4
#define DELAY_PIN 5
#define BRIGHTNESS_PIN 6
#define AUTO_PIN 7

#define SIGNAL_PIN_0 0
#define SIGNAL_PIN_1 1
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

int spectrumValue[COLUMNS];
long int counter = 0;
int long pwmpulse = 0;
bool toggle = false;
int long time_change = 0;
int effect = 0;
int peakhorse = 1;
int peakdelay = 1;
int brightness_led =20;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);

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
   pinMode      (SIGNAL_PIN_0,    INPUT);
   pinMode      (SIGNAL_PIN_1,     INPUT);
   pinMode      (PEAK_DELAY_PIN,      INPUT);
   pinMode      (COLOR_COLUMN_PIN,     INPUT);
   pinMode      (COLOR_PEAK_PIN,     INPUT);
   pinMode      (DELAY_PIN ,     INPUT);
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
   pixels.setBrightness(brightness_led);
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
  clearspectrum();    // reset led về false
  
  //Đọc data khi ở chế độ hand
  read_data_hand()
  
  //Set độ sáng cho led
  pixels.setBrightness(brightness_led);
  
  //Tạo xung cấp cho chân Pulse pin của si5351
  pulse_for_si5351();
    
  //Pulse sườn xuống cho chân reset si5351
  controll_reset_si5351();
    
  //Read signal music
  read_signal_music(); 
  if(check_auto() ==true)
  {
    //nếu auto thì set led theo màu sắc
    set_led_follow_music(1,2,3,4,5,6);
  }
  else {
      //Set led follow with music
      set_led_follow_music(1,2,3,4,5,6);
  }
  //Show led
  flushMatrix();
  if(counter % peakdelay ==0)topSinking(); // thời gian delay để giảm độ rơi của đỉnh
}//End Loop 
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
void pulse_for_si5351(){
  if (millis() - pwmpulse > 3000){
    toggle = !toggle;
    digitalWrite(PULSE_PIN, toggle);
    pwmpulse = millis();
  }
}
//=====================================================================================
//=====================================================================================
void controll_reset_si5351(){
  digitalWrite(RESET_PIN, HIGH);
  delayMicroseconds(3000);
  digitalWrite(RESET_PIN, LOW)
}
//======================================================================================
//======================================================================================
void read_signal_music(){
  for(int i=0; i < COLUMNS; i++){ 
    digitalWrite(STROBE_PIN, LOW);
    delayMicroseconds(1000);
    spectrumValue[i] = analogRead(SIGNAL_PIN_0);
    if(spectrumValue[i] < 120)spectrumValue[i] = 0;
    //Ghim giá trị từ 0-1023
    spectrumValue[i] = constrain(spectrumValue[i], 0, 1023);
    spectrumValue[i] = map(spectrumValue[i], 0, 1023, 0, ROWS);i++;
    
    spectrumValue[i] = analogRead(SIGNAL_PIN_1);
    if(spectrumValue[i] < 120)spectrumValue[i] = 0;
    spectrumValue[i] = constrain(spectrumValue[i], 0, 1023);
    spectrumValue[i] = map(spectrumValue[i], 0, 1023, 0, ROWS);
    digitalWrite(STROBE_PIN, HIGH);  
  }
}
//======================================================================================
//======================================================================================
void set_led_follow_music(int r1, int g1, int b1, int r2, int g2, int b2){
  for(int j = 0; j < COLUMNS; j++){
      for(int i = 0; i < spectrumValue[j]; i++){ 
        spectrum[i][COLUMNS - 1 - j].active = 1;     //Led được set sáng chờ show
        spectrum[i][COLUMNS - 1 - j].r =rl; //i*32;           //COLUMN Color red
        spectrum[i][COLUMNS - 1 - j].g =gl; //255;         //COLUMN Color green
        spectrum[i][COLUMNS - 1 - j].b =b1; //j*16;           //COLUMN Color blue
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
        spectrum[peakhold[j].position][COLUMNS - 1 - j].active = 1;
        spectrum[peakhold[j].position][COLUMNS - 1 - j].r = r2;  //Peak Color red
        spectrum[peakhold[j].position][COLUMNS - 1 - j].g = g2;  //Peak Color green
        spectrum[peakhold[j].position][COLUMNS - 1 - j].b = b2;    //Peak Color blue
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
  brightness_led = map(analogRead(BRIGHTNESS_PIN), 0, 1023, 20, 255));

  //Đọc độ trễ đỉnh
  peakhorse = map(analogRead(PEAK_HORSE_PIN), 0,1023, 1,3);
  
  //Đọc độ trễ khi rơi đỉnh
  peakdelay = map(analogRead(PEAK_DELAY_PIN), 0,1023, 1,5);

  //Đọc peakcolor
  //Đọc .....
}
