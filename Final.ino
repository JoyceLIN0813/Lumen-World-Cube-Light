#include <Adafruit_NeoPixel.h>

// ------------------- 效果1：两个灯带控制 -------------------
#define LED1_PIN 13
#define NUM_LEDS1 10
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM_LEDS1, LED1_PIN, NEO_GRB + NEO_KHZ800);

#define LED2_PIN 12
#define NUM_LEDS2 10
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM_LEDS2, LED2_PIN, NEO_GRB + NEO_KHZ800);

const int touchPins1[5] = {2, 3, 4, 5, 6};
const int touchPins2[5] = {A1, A2, A3, A5, A4};

// 亮度数组，保存每个灯当前亮度（0-255）
uint8_t brightness1[NUM_LEDS1] = {0};
uint8_t brightness2[NUM_LEDS2] = {0};

// 亮度递减步长，每次循环降低多少亮度（可调节）
const uint8_t fadeStep = 50;

// ------------------- 效果2+3：42灯流水和水波纹 -------------------
#define LED_PIN 10
#define LED_COUNT 42
#define RING_COUNT 10
#define SENSOR_A_PIN 7
#define SENSOR_B_PIN A0
#define INTERVAL 110
#define BRIGHTNESS 50

Adafruit_NeoPixel leds(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// 效果2控制
struct Animation {
unsigned long startTime;
bool active;
};
Animation animation;
bool isAnimating = false;

// 效果3控制
int showType = 0;
bool lastTouchA = false;
bool lastTouchB = true;

uint32_t softBlueGreen = leds.Color(128, 100, 15);
uint32_t softBlueGreenDim = leds.Color(80, 60, 10);
uint32_t offColor = leds.Color(0, 0, 0);

// ------------------- 效果4：环形灯和按钮 -------------------
#define BUTTON_PIN 11
#define RING_LED_PIN 9
#define RING_LED_COUNT 16

Adafruit_NeoPixel ringLED = Adafruit_NeoPixel(RING_LED_COUNT, RING_LED_PIN, NEO_GRB + NEO_KHZ800);
bool ringOn = false;
bool lastButtonState = HIGH;

// ------------------- SETUP -------------------
void setup() {
Serial.begin(9600);

// 效果1
strip1.begin(); strip1.show();
strip2.begin(); strip2.show();
for (int i = 0; i < 5; i++) {
pinMode(touchPins1[i], INPUT);
pinMode(touchPins2[i], INPUT);
}

// 效果2+3
leds.begin();
leds.setBrightness(BRIGHTNESS);
leds.show();
pinMode(SENSOR_A_PIN, INPUT);
pinMode(SENSOR_B_PIN, INPUT);
animation.active = false;

// 效果4
pinMode(BUTTON_PIN, INPUT_PULLUP);
ringLED.begin();
ringLED.show();
}

// ------------------- LOOP -------------------
void loop() {
// 效果1 控制两个灯带，使用亮度渐变效果
for (int i = 0; i < 5; i++) {
bool touched1 = (digitalRead(touchPins1[i]) == LOW);
int ledIndex1 = i * 2;
int ledIndex2 = i * 2 + 1;

if (touched1) {
brightness1[ledIndex1] = 200;
brightness1[ledIndex2] = 200;
} else {
if (brightness1[ledIndex1] > fadeStep) brightness1[ledIndex1] -= fadeStep;
else brightness1[ledIndex1] = 0;

if (brightness1[ledIndex2] > fadeStep) brightness1[ledIndex2] -= fadeStep;
else brightness1[ledIndex2] = 0;
}

strip1.setPixelColor(ledIndex1, strip1.Color(brightness1[ledIndex1], brightness1[ledIndex1], brightness1[ledIndex1]));
strip1.setPixelColor(ledIndex2, strip1.Color(brightness1[ledIndex2], brightness1[ledIndex2], brightness1[ledIndex2]));

bool touched2 = (digitalRead(touchPins2[i]) == LOW);

if (touched2) {
brightness2[ledIndex1] = 200;
brightness2[ledIndex2] = 200;
} else {
if (brightness2[ledIndex1] > fadeStep) brightness2[ledIndex1] -= fadeStep;
else brightness2[ledIndex1] = 0;

if (brightness2[ledIndex2] > fadeStep) brightness2[ledIndex2] -= fadeStep;
else brightness2[ledIndex2] = 0;
}

strip2.setPixelColor(ledIndex1, strip2.Color(brightness2[ledIndex1], brightness2[ledIndex1], brightness2[ledIndex1]));
strip2.setPixelColor(ledIndex2, strip2.Color(brightness2[ledIndex2], brightness2[ledIndex2], brightness2[ledIndex2]));
}
strip1.show();
strip2.show();

// 效果2 控制流水灯
bool touchA = (digitalRead(SENSOR_A_PIN) == LOW);
if (!isAnimating && touchA && !lastTouchA) {
animation.startTime = millis();
animation.active = true;
isAnimating = true;
delay(200);
}
lastTouchA = touchA;

// 效果3 控制水波纹
bool touchB = (digitalRead(SENSOR_B_PIN) == LOW);
if (touchB && !lastTouchB) {
delay(50);
if (digitalRead(SENSOR_B_PIN) == LOW) {
showType++;
if (showType > 1) showType = 0;
startShow(showType);
}
}
lastTouchB = touchB;

// 执行流水灯动画
if (isAnimating) {
runFlowingLight();
}

// 效果4 储存式按钮控制环形灯
bool buttonState = digitalRead(BUTTON_PIN);
if (buttonState == LOW && lastButtonState == HIGH) {
ringOn = !ringOn;
delay(200); // 防抖
}
lastButtonState = buttonState;

if (ringOn) {
for (int i = 0; i < RING_LED_COUNT; i++) {
ringLED.setPixelColor(i, softBlueGreen);
}
} else {
ringLED.clear();
}
ringLED.show();

delay(100);
}

// ------------------- 动画函数 -------------------
void runFlowingLight() {
uint8_t brightness[LED_COUNT] = {0};
unsigned long now = millis();
if (!animation.active) return;

unsigned long elapsed = now - animation.startTime;
if (elapsed > INTERVAL * (LED_COUNT + 6)) {
animation.active = false;
isAnimating = false;
leds.clear();
leds.show();
return;
}

int litCount = elapsed / INTERVAL;
if (litCount > LED_COUNT) litCount = LED_COUNT;

for (int i = 0; i < litCount; i++) {
int ledIndex;
if (i < RING_COUNT) {
ledIndex = RING_COUNT - 1 - i;
} else {
int linearPos = i - RING_COUNT;
ledIndex = LED_COUNT - 1 - linearPos;
if (ledIndex < RING_COUNT) ledIndex = RING_COUNT;
}

unsigned long ledOnTime = animation.startTime + i * INTERVAL;
unsigned long ledElapsed = now - ledOnTime;

uint8_t b = 0;
if (ledElapsed < INTERVAL * 3) {
b = 255;
} else if (ledElapsed < INTERVAL * 6) {
float fade = 1.0 - (float)(ledElapsed - INTERVAL * 3) / (INTERVAL * 3);
b = (uint8_t)(fade * 255);
}

if (b > brightness[ledIndex]) {
brightness[ledIndex] = b;
}
}

for (int i = 0; i < LED_COUNT; i++) {
uint8_t b = brightness[i];
if (b == 0) {
leds.setPixelColor(i, 0);
} else {
leds.setPixelColor(i, leds.Color(b, b * 200 / 255, b * 30 / 255));
}
}
leds.show();
}



void startShow(int mode) {
  switch (mode) {
    case 0:
      colorAll(offColor, 0);
      break;
    case 1:
      for (int i = 0; i < 3; i++) {
        colorLast4(softBlueGreen, 0);

        colorInnerRing(softBlueGreenDim, 200);
        colorInnerRing(softBlueGreen, 0);

        colorMiddleRing(softBlueGreenDim, 200);
        colorInnerRing(softBlueGreenDim, 0);

        colorMiddleRing(softBlueGreen, 0);
        colorOuterRing(softBlueGreenDim, 200);

        colorInnerRing(offColor, 0);
        colorMiddleRing(softBlueGreenDim, 0);

        colorOuterRing(softBlueGreen, 200);

        colorInnerRing(offColor, 0);
        colorMiddleRing(offColor, 0);

        colorOuterRing(softBlueGreenDim, 200);
        colorOuterRing(offColor, 0);
      }
      break;
  }
}

void colorAll(uint32_t c, uint16_t wait) {
  for (uint16_t i = 0; i < leds.numPixels(); i++) {
    leds.setPixelColor(i, c);
  }
  leds.show();
  delay(wait);
}

void colorInnerRing(uint32_t c, uint16_t wait) {
  for (uint16_t i = 0; i < 12; i++) {
    leds.setPixelColor(i, c);
  }
  leds.show();
  delay(wait);
}

void colorMiddleRing(uint32_t c, uint16_t wait) {
  for (uint16_t i = 12; i < 25; i++) {
    leds.setPixelColor(i, c);
  }
  leds.show();
  delay(wait);
}

void colorOuterRing(uint32_t c, uint16_t wait) {
  for (uint16_t i = 25; i < 42; i++) {
    leds.setPixelColor(i, c);
  }
  leds.show();
  delay(wait);
}

void colorLast4(uint32_t c, uint16_t wait) {
  for (uint16_t i = 38; i < 42; i++) {
    leds.setPixelColor(i, c);
  }
  leds.show();
  delay(wait);
}







