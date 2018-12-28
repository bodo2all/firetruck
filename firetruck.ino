#include <DFMiniMp3.h>

//-------Pin Names-----------
#define LED_FRONT_BLUE_01 2 //Blue light front 01
#define LED_FRONT_BLUE_02 3 //Blue light front 02
#define LED_BLK_L         8 //Orange left blinker
#define LED_BLK_R         7 //Orange right blinker
#define LED_GYROS_B       5 //Blue light back gyro
#define LED_GYROS_F       6 //Blue light front gyro
#define LED_GYROS_L       9 //Blue light left gyro
#define LED_GYROS_R      10 //Blue light right gyro
#define LED_FLOOD        11 //White flood light on mast/pole
#define LED_KBN          12 //White innner kabin lights
#define LED_FRONT_W      13 //white front driving lights

#define BTN_USER          4 //user choice button

//-------SONG NAMES--------------
#define NONE   0 // no music
#define DIESEL 1 // engine noise file 0001_diesel_iddle.mp3
#define SYRENE 5 // loud horn noise file 0005_feuerwehr_sirene.mp3

//-------USER SETTINGS-----------
#define PWM_GYRO         //comment this out when not using pwm pins
#define GYRO_CLOCKWISE   //comment this out to reverse Gyro rotation

#define DEFAULT_VOLUME          5 // from 0 to 30 loudness
#define LED_FB_INTERVAL       100 //ms
#define LED_BLK_INTERVAL      500 //ms
#define BTN_DEBOUNCE_INTERVAL  50 //ms

#ifdef PWM_GYRO
#define LED_GYROS_INTERVAL 35 //ms
#else
#define LED_GYROS_INTERVAL 140 //ms should be 4x above timing
#endif

// controls light fading and spread,
// imagine it's a 360beam with the middle facing forward
// and the sides facing backwards
// 0 is max light, 255 is no light
uint8_t led_gyros_state[] =
#ifdef PWM_GYRO
  { 255, 255, 255, 253, 200, 150, 80, 0, 80, 150, 200, 253, 255, 255, 255, 255 };
#else
  { LOW, HIGH, HIGH, HIGH };
#endif

typedef struct {
  bool const blink_front_state;
  bool const blink_blinkers_state;
  bool const blink_gyros_state;
  bool const led_kbn_state;
  bool const led_front_state;
  bool const led_flood_state;
  uint8_t const soundtrack_no;
  uint8_t const soundtrack_vol;
} FireTruck_t; // associate all those properties with the concept of truck

uint8_t user_choice = 7;
FireTruck_t const FireTruck[] = { //there's now many firetruck combinations, choose one.
  {
  .blink_front_state = false,
  .blink_blinkers_state = false,
  .blink_gyros_state = false,
  .led_kbn_state = HIGH,
  .led_front_state = HIGH,
  .led_flood_state = HIGH,
  .soundtrack_no = NONE,
  .soundtrack_vol = DEFAULT_VOLUME
  },
  {
  .blink_front_state = false,
  .blink_blinkers_state = false,
  .blink_gyros_state = false,
  .led_kbn_state = HIGH,
  .led_front_state = LOW,
  .led_flood_state = HIGH,
  .soundtrack_no = DIESEL,
  .soundtrack_vol = DEFAULT_VOLUME
  },
  {
  .blink_front_state = false,
  .blink_blinkers_state = false,
  .blink_gyros_state = false,
  .led_kbn_state = LOW,
  .led_front_state = LOW,
  .led_flood_state = HIGH,
  .soundtrack_no = DIESEL,
  .soundtrack_vol = DEFAULT_VOLUME
  },
  {
  .blink_front_state = false,
  .blink_blinkers_state = false,
  .blink_gyros_state = false,
  .led_kbn_state = LOW,
  .led_front_state = LOW,
  .led_flood_state = LOW,
  .soundtrack_no = DIESEL,
  .soundtrack_vol = DEFAULT_VOLUME
  },
  {
  .blink_front_state = false,
  .blink_blinkers_state = true,
  .blink_gyros_state = false,
  .led_kbn_state = LOW,
  .led_front_state = LOW,
  .led_flood_state = LOW,
  .soundtrack_no = DIESEL,
  .soundtrack_vol = DEFAULT_VOLUME
  },
  {
  .blink_front_state = true,
  .blink_blinkers_state = true,
  .blink_gyros_state = false,
  .led_kbn_state = LOW,
  .led_front_state = LOW,
  .led_flood_state = LOW,
  .soundtrack_no = DIESEL,
  .soundtrack_vol = DEFAULT_VOLUME
  },
  {
  .blink_front_state = true,
  .blink_blinkers_state = true,
  .blink_gyros_state = true,
  .led_kbn_state = LOW,
  .led_front_state = LOW,
  .led_flood_state = LOW,
  .soundtrack_no = DIESEL,
  .soundtrack_vol = DEFAULT_VOLUME
  },
  {
  .blink_front_state = true,
  .blink_blinkers_state = true,
  .blink_gyros_state = true,
  .led_kbn_state = LOW,
  .led_front_state = LOW,
  .led_flood_state = LOW,
  .soundtrack_no = SYRENE,
  .soundtrack_vol = DEFAULT_VOLUME
  },
};

uint8_t psound_track = NONE;

// implement a notification class,
// its member methods will get called in the .loop()
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
  // button pins should be input, user drives them to GND, when iddle it's pulled to 5V
  pinMode(BTN_USER,    INPUT_PULLUP); // when not pressed the pin state is high

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

  //Serial.begin(9600);//not needed, dfplayer does it
  DFPlayer.begin();
}



void loop() {
  // put your main code here, to run repeatedly:
  read_btns();

  if (FireTruck[user_choice].blink_front_state) {
    blink_front_blue(); //checks if it's time to do something
  } else {
    digitalWrite(LED_FRONT_BLUE_01, HIGH);
    digitalWrite(LED_FRONT_BLUE_02, HIGH);
  }
  
  if (FireTruck[user_choice].blink_blinkers_state) {
    blink_blinkers();
  } else {
    digitalWrite(LED_BLK_L,   HIGH);
    digitalWrite(LED_BLK_R,   HIGH);
  }
  
  if (FireTruck[user_choice].blink_gyros_state) {
    blink_gyros();
  } else {
    digitalWrite(LED_GYROS_L, HIGH);
    digitalWrite(LED_GYROS_R, HIGH);
    digitalWrite(LED_GYROS_F, HIGH);
    digitalWrite(LED_GYROS_B, HIGH);
  }
  
  digitalWrite(LED_FRONT_W, FireTruck[user_choice].led_front_state);
  digitalWrite(LED_KBN,     FireTruck[user_choice].led_kbn_state);
  digitalWrite(LED_FLOOD,   FireTruck[user_choice].led_flood_state);

  if (psound_track != FireTruck[user_choice].soundtrack_no) {
    if (FireTruck[user_choice].soundtrack_no == NONE) {
      DFPlayer.stop();
    } else {
      DFPlayer.reset();
      DFPlayer.setVolume(FireTruck[user_choice].soundtrack_vol);
      DFPlayer.loopGlobalTrack(FireTruck[user_choice].soundtrack_no);
    }
    psound_track = FireTruck[user_choice].soundtrack_no;
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
    digitalWrite(LED_BLK_L, led_blk_state);
    digitalWrite(LED_BLK_R, led_blk_state);
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

void read_btns() {
  static bool btn_usr_state = true;
  static bool btn_usr_pstate = true;
  static unsigned long btn_usr_previous_millis = 0; 

  uint8_t reading = digitalRead(BTN_USER);

  if (reading != btn_usr_pstate) {
    btn_usr_previous_millis = millis();
  }
  unsigned long btn_usr_current_millis = millis();
  if (btn_usr_current_millis - btn_usr_previous_millis > BTN_DEBOUNCE_INTERVAL) {
    if (btn_usr_state != reading) {
      btn_usr_state = reading;

      if (btn_usr_pstate == HIGH) {
        user_choice++;
         //user choice must remain in how many trucks there are in the list
        user_choice %= sizeof(FireTruck)/sizeof(FireTruck_t);
      }
    }
  }
  btn_usr_pstate = reading;
}
