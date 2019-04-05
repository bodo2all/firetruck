#include <DFMiniMp3.h>

//-------Pin Names-----------
#define LED_FRONT_BLUE_01 2 //Blue light front 01
#define LED_FRONT_BLUE_02 A3 //PWM Blue light front 02
#define LED_BLK_L         8 //Orange left blinker
#define LED_BLK_R         7 //Orange right blinker
#define LED_GYROS_B       5 //PWM Blue light back gyro
#define LED_GYROS_F       6 //PWM Blue light front gyro
#define LED_GYROS_L       9 //PWM Blue light left gyro
#define LED_GYROS_R      10 //PWM Blue light right gyro
#define LED_FLOOD         4 //PWM White flood light on mast/pole
#define LED_KBN          12 //White innner kabin lights
#define LED_FRONT_W      13 //white front driving lights
#define LED_FLOOD_BACK   A0 //White flood and Inner lights back
#define LED_REVERSE      A1 //White reverse gear lights
#define LED_BRAKE        A2 //Red brake lights
#define LED_FIRE1         3 //Orange candle/fire light
#define LED_FIRE2        11 //Orange candle/fire light

#define BTN_ADC_PORT      A4 //analog input
#define BTN_BLUE          1 // 1user choice button
#define BTN_GYRO          2 // 2user choice button
#define BTN_BLK           3 // 3user choice button
#define BTN_FLOOD         4 // 4user choice button
#define BTN_KBN           5 // 5user choice button
#define BTN_FRONT         6 // 6user choice button
#define BTN_FLOOD_BACK    7 // 7user choice button
#define BTN_REVERSE       8 // 8user choice button
#define BTN_BRAKE         9 // 9user choice button
#define BTN_FIRE         10 //10user choice button
#define BTN_AUDIO_MOTOR  11 //11user choice button
#define BTN_AUDIO_SIREN  12 //12user choice button
#define BTN_AUDIO_FIRE   13 //13user choice button
#define BTN_DEMO         14 //14user choice button

//-------SONG NAMES--------------
#define NONE   0 // no music
#define DIESEL 1 // engine noise file 0001_diesel_iddle.mp3
#define SYRENE 5 // loud horn noise file 0005_feuerwehr_sirene.mp3

//-------USER SETTINGS-----------
#define PWM_GYRO         //comment this out when not using pwm pins
#define GYRO_CLOCKWISE   //comment this out to reverse Gyro rotation

#define DEFAULT_VOLUME          25 // from 0 to 30 loudness
#define LED_FB_INTERVAL       100 //ms
#define LED_BLK_INTERVAL      500 //ms
#define BTN_DEBOUNCE_INTERVAL  30 //ms

#ifdef PWM_GYRO
#define LED_GYROS_INTERVAL 35 //ms
#else
#define LED_GYROS_INTERVAL 140 //ms should be 4x above timing
#endif

int fire_led_state[2];
unsigned fire_led_pin[] = {LED_FIRE1, LED_FIRE2};

// the following array controls light fading and spread,
// imagine it's a 360beam with the middle facing forward
// and the sides facing backwards
// 0 is max light, 255 is no light
uint8_t led_gyros_state[] =
#ifdef PWM_GYRO
  { 255, 255, 255, 253, 200, 150, 80, 0, 80, 150, 200, 253, 255, 255, 255, 255 };
#else
  { LOW, HIGH, HIGH, HIGH };
#endif


unsigned btn_adc[] = {
  13,  18,  23,  28,  33,  42,  67, 
  99, 130, 140, 175, 232, 373, 575, 1000
  };//no buttons=adc_max=1024
  
typedef struct {
  bool blink_front_state;
  uint8_t blink_blinkers_state;
  bool blink_gyros_state;
  bool led_kbn_state;
  bool led_front_state;
  bool led_flood_state;
  bool led_flood_back_state;
  bool led_reverse_state;
  bool led_brake_state;
  bool blink_fire_state;
  uint8_t  soundtrack_no;
  uint8_t  soundtrack_vol;
} FireTruck_t; // associate all those properties with the concept of truck

#if 0
FireTruck_t FireTruck = {
  .blink_front_state    = true,
  .blink_blinkers_state = 3,//both left and right
  .blink_gyros_state    = true,
  .led_kbn_state   = LOW,
  .led_front_state = LOW,
  .led_flood_state = LOW,
  .led_flood_back_state = LOW,
  .led_reverse_state    = HIGH,
  .led_brake_state      = LOW, //brake, reverse and back blinkers are on the same supply so don't power at once.
  .blink_fire_state     = true,
  .soundtrack_no = DIESEL, //DIESEL,SYRENE
  .soundtrack_vol = DEFAULT_VOLUME
};
#else
FireTruck_t FireTruck = {
  .blink_front_state    = false,
  .blink_blinkers_state = 0,//both left and right
  .blink_gyros_state    = false,
  .led_kbn_state        = HIGH,
  .led_front_state      = HIGH,
  .led_flood_state      = HIGH,
  .led_flood_back_state = HIGH,
  .led_reverse_state    = HIGH,
  .led_brake_state      = HIGH,
  .blink_fire_state     = false,
  .soundtrack_no = NONE,
  .soundtrack_vol = DEFAULT_VOLUME
};
#endif

uint8_t psound_track = NONE;

// implement a notification class,
// its member methods will get called in the .loop()
#if 1
class Mp3Notify
{
  public:
    static void OnError(uint16_t errorCode)
    {
     ;
    }
    static void OnPlayFinished(uint16_t track)
    {
      ;
    }
    static void OnCardOnline(uint16_t code)
    {
      ;
    }
    static void OnCardInserted(uint16_t code)
    {
      ;
    }
    static void OnCardRemoved(uint16_t code)
    {
      ;
    }
};
#else
class Mp3Notify
{
  public:
    static void OnError(uint16_t errorCode)
    {
      Serial.println();
      Serial.print("Com Error ");
      Serial.println(errorCode); // see DfMp3_Error for code meaning
    }
    static void OnPlayFinished(uint16_t track)
    {
      Serial.print("Play finished for #");
      Serial.println(track);
    }
    static void OnCardOnline(uint16_t code)
    {
      Serial.println("Card online ");
    }
    static void OnCardInserted(uint16_t code)
    {
      Serial.println("Card inserted ");
    }
    static void OnCardRemoved(uint16_t code)
    {
      Serial.println("Card removed ");
    }
};
#endif

//mp3 player object, talks to the hardware serial port "Serial"
// notifies you via the above prints of erros and events
DFMiniMp3<HardwareSerial, Mp3Notify> DFPlayer(Serial);



// put your setup code here, to run once:
void setup() {

  // led pins should be output, we drive them
  pinMode(LED_FRONT_BLUE_01, OUTPUT);
  pinMode(LED_FRONT_BLUE_02, OUTPUT);
  pinMode(LED_BLK_L,   OUTPUT);
  pinMode(LED_BLK_R,   OUTPUT);
  pinMode(LED_GYROS_L, OUTPUT);
  pinMode(LED_GYROS_R, OUTPUT);
  pinMode(LED_GYROS_F, OUTPUT);
  pinMode(LED_GYROS_B, OUTPUT);
  pinMode(LED_FRONT_W, OUTPUT);
  pinMode(LED_KBN,     OUTPUT);
  pinMode(LED_FLOOD,   OUTPUT);
  pinMode(LED_FLOOD_BACK,   OUTPUT);
  pinMode(LED_REVERSE,   OUTPUT);
  pinMode(LED_BRAKE,   OUTPUT);
  pinMode(LED_FIRE1,   OUTPUT);
  pinMode(LED_FIRE2,   OUTPUT);
  
  // button pins should be input, user drives them to GND, when iddle it's pulled to 5V
  pinMode(BTN_ADC_PORT,    INPUT_PULLUP); // when not pressed the pin state is high

  // all project leds are turned off by pin high (and turned on by pin low)
  digitalWrite(LED_FRONT_BLUE_01, HIGH);
  digitalWrite(LED_FRONT_BLUE_02, HIGH);
  digitalWrite(LED_BLK_L,   HIGH);
  digitalWrite(LED_BLK_R,   HIGH);
  digitalWrite(LED_GYROS_L, HIGH);
  digitalWrite(LED_GYROS_R, HIGH);
  digitalWrite(LED_GYROS_F, HIGH);
  digitalWrite(LED_GYROS_B, HIGH);
  digitalWrite(LED_FRONT_W, HIGH);
  digitalWrite(LED_KBN,     HIGH);
  digitalWrite(LED_FLOOD,   HIGH);
  pinMode(LED_FLOOD_BACK,   HIGH);
  pinMode(LED_REVERSE,      HIGH);
  pinMode(LED_BRAKE,        HIGH);
  pinMode(LED_FIRE1,        HIGH);
  pinMode(LED_FIRE2,        HIGH);

  fire_led_state[0] = random(20, 201);
  fire_led_state[1] = random(20, 201);

  //Serial.begin(9600);//not needed, dfplayer does it
  DFPlayer.begin();
}



void loop() {
  // put your main code here, to run repeatedly:
  read_btns();

  if (FireTruck.blink_front_state) {
    blink_front_blue(); //checks if it's time to do something
  } else {
    digitalWrite(LED_FRONT_BLUE_01, HIGH);
    digitalWrite(LED_FRONT_BLUE_02, HIGH);
  }
  
  if (FireTruck.blink_blinkers_state) {
    blink_blinkers();
  } else {
    digitalWrite(LED_BLK_L,   HIGH);
    digitalWrite(LED_BLK_R,   HIGH);
  }
  
  if (FireTruck.blink_gyros_state) {
    blink_gyros();
  } else {
    digitalWrite(LED_GYROS_L, HIGH);
    digitalWrite(LED_GYROS_R, HIGH);
    digitalWrite(LED_GYROS_F, HIGH);
    digitalWrite(LED_GYROS_B, HIGH);
  }
  
  digitalWrite(LED_FRONT_W, FireTruck.led_front_state);
  digitalWrite(LED_KBN,     FireTruck.led_kbn_state);
  digitalWrite(LED_FLOOD,   FireTruck.led_flood_state);
  digitalWrite(LED_FLOOD_BACK, FireTruck.led_flood_back_state);
  digitalWrite(LED_REVERSE,    FireTruck.led_reverse_state);
  digitalWrite(LED_BRAKE,      FireTruck.led_brake_state);
  
  if(FireTruck.blink_fire_state) {
    blink_fire();
  } else {
    digitalWrite(LED_FIRE1, HIGH);
    digitalWrite(LED_FIRE2, HIGH);
  }
  
  if (psound_track != FireTruck.soundtrack_no) {
    if (FireTruck.soundtrack_no == NONE) {
      DFPlayer.stop();
    } else {
      DFPlayer.reset();
      DFPlayer.setVolume(FireTruck.soundtrack_vol);
      DFPlayer.loopGlobalTrack(FireTruck.soundtrack_no);
    }
    psound_track = FireTruck.soundtrack_no;
  }

  //DFPlayer.loop(); // reads and prints out errors
  delay(5);//low power, all timings are > 35ms so we're still realtime.
}



void blink_front_blue() {
  static bool led_fb_01_state = HIGH;
  static bool led_fb_02_state = LOW;
  static unsigned long led_fb_previous_millis = 0;

  unsigned long led_fb_current_millis = millis();

  if (led_fb_current_millis - led_fb_previous_millis >= LED_FB_INTERVAL) {
    // save the last time you blinked the LED
    led_fb_previous_millis = led_fb_current_millis;

    // if the LED is off turn it on and vice-versa:
    led_fb_01_state = !led_fb_01_state; // ! means not so !LOW means notLOW so HIGH
    led_fb_02_state = !led_fb_02_state;

    // set the LED with the ledState of the variable:
    digitalWrite(LED_FRONT_BLUE_01, led_fb_01_state);
    digitalWrite(LED_FRONT_BLUE_02, led_fb_02_state);
  }
}

void blink_blinkers() {
  static bool led_blk_state = LOW;
  static unsigned long led_blk_previous_millis = 0;

  unsigned long led_blk_current_millis = millis();

  if (led_blk_current_millis - led_blk_previous_millis >= LED_BLK_INTERVAL) {
    // save the last time you blinked the LED
    led_blk_previous_millis = led_blk_current_millis;

    // if the LED is off turn it on and vice-versa:
    led_blk_state = !led_blk_state;

    // set the LED with the ledState of the variable:
    if(FireTruck.blink_blinkers_state & 0x01) { // for 1 and 3 turn on left
      digitalWrite(LED_BLK_L, led_blk_state);
    }
    if(FireTruck.blink_blinkers_state & 0x02) {// for 2 and 3 turn on right
      digitalWrite(LED_BLK_R, led_blk_state);
    }
  }
}


void blink_gyros() {
  static unsigned long led_gyros_previous_millis = 0;
  unsigned long led_gyros_current_millis = millis();
  uint8_t tmp;
  uint8_t i;

  if (led_gyros_current_millis - led_gyros_previous_millis >= LED_GYROS_INTERVAL) {

#ifdef PWM_GYRO
    analogWrite(LED_GYROS_L, led_gyros_state[0]); // imagine a circle with 0 being 12oclock
    analogWrite(LED_GYROS_F, led_gyros_state[4]); // 4 being  3oclock
    analogWrite(LED_GYROS_R, led_gyros_state[8]); // 8 being  6oclock
    analogWrite(LED_GYROS_B, led_gyros_state[12]);// 12 being 9oclock
#else
    // set the LED with the ledState of the variable:
    digitalWrite(LED_GYROS_L, led_gyros_state[0]);
    digitalWrite(LED_GYROS_F, led_gyros_state[1]);
    digitalWrite(LED_GYROS_R, led_gyros_state[2]);
    digitalWrite(LED_GYROS_B, led_gyros_state[3]);
#endif


#ifdef GYRO_CLOCKWISE
    //rotate values right
    tmp = led_gyros_state[sizeof(led_gyros_state) - 1]; //store last element
    for ( i = sizeof(led_gyros_state) - 1; i > 0 ; i-- ) {
      led_gyros_state[i] = led_gyros_state[i - 1];
    }
    led_gyros_state[0] = tmp;
#else
    //rotate values left
    tmp = led_gyros_state[0];
    for ( i = 0; i < sizeof(led_gyros_state) - 1; i++ ) {
      led_gyros_state[i] = led_gyros_state[i + 1];
    }
    led_gyros_state[sizeof(led_gyros_state) - 1] = tmp;
#endif

    // save the last time you blinked the LED
    led_gyros_previous_millis = led_gyros_current_millis;
  }
}


void blink_fire() {
  static unsigned long led_fire_previous_millis = 0;
  unsigned long led_fire_current_millis = millis();
  
  if (led_fire_current_millis - led_fire_previous_millis >= (random(100))) {

   #define FIRE_BRIGHTNESS 50
   // analogWrite(LED_FIRE1, 255 - (FIRE_BRIGHTNESS + random(255-FIRE_BRIGHTNESS)));
   // analogWrite(LED_FIRE2, 255 - (FIRE_BRIGHTNESS + random(255-FIRE_BRIGHTNESS)));
   for (int i=0; i<2; i++){                  // for each led:
     analogWrite(fire_led_pin[i], fire_led_state[i]);     // set the pwm value of that pin determined previously
     fire_led_state[i] += random(-40, 41);;               // that range can be tweaked to change the intensity of the flickering
     if (fire_led_state[i] > (255 - FIRE_BRIGHTNESS)) {   // clamp the limits of the pwm values so it remains within
       fire_led_state[i] = (255 - FIRE_BRIGHTNESS);       // a pleasing range as well as the pwm range
     }
     if (fire_led_state[i] < 0) {
       fire_led_state[i] = 0;
     }
    }
    led_fire_previous_millis = led_fire_current_millis;
  }
}

unsigned map_btn_adc_to_id(unsigned val) {
  #if 1
  unsigned const correction = 3;//when supplied by batteries
  #else
   unsigned const correction = 0;//when supplied by USB
   #endif
  for(unsigned i=0; i < sizeof(btn_adc); i++ ) {
    if (val < (btn_adc[i] - correction)) {
      return i;
    }
  }
  return sizeof(btn_adc) - 1 ;
}

void read_btns() {
  static uint8_t btn_usr_state = true;
  static uint8_t btn_usr_pstate = true;
  static unsigned long btn_usr_previous_millis = 0; 


  uint8_t reading = map_btn_adc_to_id(analogRead(BTN_ADC_PORT));

  if (reading != btn_usr_pstate) {
    btn_usr_previous_millis = millis();
  }
  unsigned long btn_usr_current_millis = millis();
  if (btn_usr_current_millis - btn_usr_previous_millis > BTN_DEBOUNCE_INTERVAL) {
    if (btn_usr_state != reading) {
      btn_usr_state = reading;

      switch(btn_usr_pstate) {
        case BTN_BLUE: FireTruck.blink_front_state ^= 1;
        break;
        case BTN_GYRO: FireTruck.blink_gyros_state ^= 1;
        break;
        case BTN_BLK:
          FireTruck.blink_blinkers_state++;
          FireTruck.blink_blinkers_state %= 4;
          digitalWrite(LED_BLK_R, HIGH); // turn off all blikers
          digitalWrite(LED_BLK_L, HIGH); // so they don't stay
        break;
        case BTN_FLOOD: FireTruck.led_flood_state ^= 1;
        break;
        case BTN_KBN: FireTruck.led_kbn_state ^= 1;
        break;
        case BTN_FRONT: FireTruck.led_front_state ^= 1;
        break;
        case BTN_FLOOD_BACK: FireTruck.led_flood_back_state ^= 1;
        break;
        case BTN_REVERSE: FireTruck.led_reverse_state ^= 1;
        break;
        case BTN_BRAKE: FireTruck.led_brake_state ^= 1;
        break;
        case BTN_FIRE: FireTruck.blink_fire_state ^= 1;
        break;
        case BTN_AUDIO_MOTOR: FireTruck.soundtrack_no = DIESEL;
        break;
        case BTN_AUDIO_SIREN: FireTruck.soundtrack_no = SYRENE;
        break;
        case BTN_AUDIO_FIRE: FireTruck.soundtrack_no = NONE;
        break;
        case BTN_DEMO:
        break;
        default:
        break;//not_pressed, do nothing
      }
    }
  }
  btn_usr_pstate = reading;
}
