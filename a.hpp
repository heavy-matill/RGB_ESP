#ifndef RAINBOWANIMATOR_H_
#define RAINBOWANIMATOR_H_

//#include <stdint.h>
//#include <stddef.h>
#include <FastLED.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
    #warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#ifndef DATA_PIN
    #define DATA_PIN    3
#endif
//#define CLK_PIN   4
#ifndef LED_TYPE
    #define LED_TYPE    WS2811
#endif
#ifndef COLOR_ORDER
    #define COLOR_ORDER GRB
#endif
#ifndef NUM_LEDS
    #define NUM_LEDS    64
#endif

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

class RainbowAnimator
{
  private:
    uint8_t hue;
    typedef void (*SimplePatternList[])();
    SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

  public:
    uint8_t brightness = 128;
    uint8_t speed = 16;
    uint8_t currentPattern = 0;
    
    // constructor? destructor?
    
    
    void animate() {        
        EVERY_N_MILLISECONDS( 20 ) { hue++; } // slowly cycle the "base color" through the rainbow
    }
    
    void nextPattern() {
      // add one to the current pattern number, and wrap around at the end
      gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
    }

    void rainbow() {
      // FastLED's built-in rainbow generator
      fill_rainbow( leds, NUM_LEDS, gHue, 7);
    }

    void rainbowWithGlitter() {
      // built-in FastLED rainbow, plus some random sparkly glitter
      rainbow();
      addGlitter(80);
    }

    void addGlitter( fract8 chanceOfGlitter) {
      if( random8() < chanceOfGlitter) {
        leds[ random16(NUM_LEDS) ] += CRGB::White;
      }
    }

    void confetti() {
      // random colored speckles that blink in and fade smoothly
      fadeToBlackBy( leds, NUM_LEDS, 10);
      int pos = random16(NUM_LEDS);
      leds[pos] += CHSV( gHue + random8(64), 200, 255);
    }

    void sinelon() {
      // a colored dot sweeping back and forth, with fading trails
      fadeToBlackBy( leds, NUM_LEDS, 20);
      int pos = beatsin16( 13, 0, NUM_LEDS-1 );
      leds[pos] += CHSV( gHue, 255, 192);
    }

    void bpm() {
      // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
      uint8_t BeatsPerMinute = 62;
      CRGBPalette16 palette = PartyColors_p;
      uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
      for( int i = 0; i < NUM_LEDS; i++) { //9948
        leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
      }
    }

    void juggle() {
      // eight colored dots, weaving in and out of sync with each other
      fadeToBlackBy( leds, NUM_LEDS, 20);
      byte dothue = 0;
      for( int i = 0; i < 8; i++) {
        leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
        dothue += 32;
      }
    }
};

#endif // RAINBOWANIMATOR_H_