#include <Rtc_Pcf8563.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

#define WS2812_PIN  6
#define NUM_LEDS  2
#define LED_RED		11
#define LED_GREEN	9
#define LED_BLUE	10
#define LED_WHITE	8

#define MAX_BRIGHT	255

Rtc_Pcf8563 rtc;
Adafruit_NeoPixel pixels(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);

uint16_t cycle_from_clock(uint32_t minSinceMidnight, bool up);
void hue_to_rgb(uint16_t cycle, uint8_t *r, uint8_t *g, uint8_t *b);
void set_RGB();

uint16_t w = 0;
uint8_t tcnt = 0;
char inbuf[6];
uint8_t rdptr = 0;

void printTime()
{
	char timestr[32];
	tcnt = 0;
	sprintf(timestr, "%02d:%02d:%02d", rtc.getHour(), rtc.getMinute(), rtc.getSecond());
	Serial.println(timestr);
}

void timeToRGB()
{
	uint8_t r, g, b;
	uint16_t minSinceMN = rtc.getHour() * 60 + rtc.getMinute();

	uint16_t cycle = cycle_from_clock(minSinceMN, false);
	hue_to_rgb(cycle, &r, &g, &b);
	analogWrite(LED_RED, r);
	analogWrite(LED_GREEN, g);
	analogWrite(LED_BLUE, b);

  for(int i=0;i<NUM_LEDS;i++){
    pixels.setPixelColor(i, pixels.Color(r, g, b)); // Moderately bright green color.
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}

//The setup function is called once at startup of the sketch
void setup()
{
  pinMode(LED_WHITE, OUTPUT);
	analogWrite(LED_WHITE, 0);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	pinMode(WS2812_PIN, OUTPUT);
  pixels.begin();
  pixels.clear();
  timeToRGB();
  Serial.begin(115200);
}

// The loop function is called in an endless loop
void loop()
{

	if(w < MAX_BRIGHT)
		w++;

	analogWrite(LED_WHITE, w);

	tcnt++;
	if(tcnt == 50)
	{
		timeToRGB();
		printTime();
	}

	while(Serial.available() > 0)
	{
		int c = Serial.read();
		if(c >= '0' && c <= '9')
		{
			inbuf[rdptr] = c - '0';
			rdptr++;
			if(rdptr == 6)
			{
				rtc.setTime(inbuf[0] * 10 + inbuf[1], inbuf[2] * 10 + inbuf[3], inbuf[4] * 10 + inbuf[5]);
				printTime();
				rdptr = 0;
			}
		}
		else
			rdptr = 0;
	}

	delay(20);
}

void set_RGB()
{
	uint8_t r, g, b;
	uint16_t minSinceMN = rtc.getHour() * 60 + rtc.getMinute();
	uint16_t cycle = cycle_from_clock(minSinceMN, true);
	hue_to_rgb(cycle, &r, &g, &b);
	analogWrite(LED_RED, r);
	analogWrite(LED_GREEN, g);
	analogWrite(LED_BLUE, b);

}

void hue_to_rgb(uint16_t cycle, uint8_t *r, uint8_t *g, uint8_t *b)
{
	uint16_t lum = cycle % (MAX_BRIGHT + 1);
    if (cycle <= MAX_BRIGHT) {
        *r = MAX_BRIGHT - lum;
        *g = lum;
        *b = 0;
    }
    else if (cycle < 2*(MAX_BRIGHT+1)) {
        *r = 0;
        *g = MAX_BRIGHT - lum;
        *b = lum;
    }
    else {
        *r = lum;
        *g = 0;
        *b = MAX_BRIGHT - lum;
    }
}

#define MAX_CYCLE	(3*(MAX_BRIGHT+1)-1)

uint16_t cycle_from_clock(uint32_t minSinceMidnight, bool up)
{
	uint16_t cycle;
	uint16_t minSinceMN = minSinceMidnight * MAX_BRIGHT / 480;
	if (up) {
		cycle = (minSinceMN);
		if(cycle > MAX_CYCLE)
			cycle -= (MAX_CYCLE+1);
	} else {
		cycle = (MAX_CYCLE - minSinceMN);
		if(cycle > MAX_CYCLE)
			cycle += MAX_CYCLE+1;
	}
	return cycle;
}
