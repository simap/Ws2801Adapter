//
// Created by Ben Hencke on 2/10/17.
//

#ifndef WS2801ADAPTER_HPP
#define WS2801ADAPTER_HPP

#include <Arduino.h>
#include <SPI.h>
#include <functional>

//borrowed from Adafruit
// Color-order flag for LED pixels (optional extra parameter to constructor):
// Bits 0,1 = R index (0-2), bits 2,3 = G index, bits 4,5 = B index
#define WS2801_RGB (0 | (1 << 2) | (2 << 4))
#define WS2801_RBG (0 | (2 << 2) | (1 << 4))
#define WS2801_GRB (1 | (0 << 2) | (2 << 4))
#define WS2801_GBR (2 | (0 << 2) | (1 << 4))
#define WS2801_BRG (1 | (2 << 2) | (0 << 4))
#define WS2801_BGR (2 | (1 << 2) | (0 << 4))

typedef std::function<void(uint16_t index, uint8_t rgb[])> Ws2801PixelFunction;

class Ws2801Adapter {
public:
    Ws2801Adapter(uint8_t o = WS2801_RGB) {
        setColorOrder(o);
    }

    ~Ws2801Adapter() {
        end();
    }

    void begin(uint32_t spiFrequency = 2000000L) {
        SPI.begin();
        SPI.setFrequency(spiFrequency);
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);
        //borrowed from SPI.cpp, set registers for a 24bit transfer buffer
        uint16_t bits = 24;
        const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
        bits--;
        SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
        timer = micros();
    }

    void end() {
        SPI.end();
    }

    void setSpiFrequency(uint32_t spiFrequency) {
        SPI.setFrequency(spiFrequency);
    }

    void setColorOrder(uint8_t o) {
        rOffset = (uint8_t) ((o & 3));
        gOffset = (uint8_t) (((o >> 2) & 3));
        bOffset = (uint8_t) (((o >> 4) & 3));
    }

    void show(uint16_t numPixels, Ws2801PixelFunction cb) {
        int curPixel;
        union {
            uint32_t frame;
            uint8_t b[4];
        } buf, rgb;

        //wait for any previous latch
        while (micros() - timer < 700) //2801 needs > 500us
            yield();

        //pixels, sourced from callback
        for (curPixel = 0; curPixel < numPixels; curPixel++) {
            rgb.frame = 0; //default to black
            cb(curPixel,rgb.b);

            //swap around rgb values based on mapping
            buf.b[rOffset] = rgb.b[0];
            buf.b[gOffset] = rgb.b[1];
            buf.b[bOffset] = rgb.b[2];
            write24(buf.frame);
        }

        timer = micros();
    }

private:
    inline void write24(uint32_t v) {
        while(SPI1CMD & SPIBUSY) {}
        SPI1W0 = v;
        SPI1CMD |= SPIBUSY;
    }
    unsigned long timer;
    uint8_t
            rOffset,                                // Index of red in 3-byte pixel
            gOffset,                                // Index of green byte
            bOffset;                                // Index of blue byte
};


#endif //WS2801ADAPTER_HPP
