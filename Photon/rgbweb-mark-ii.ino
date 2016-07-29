// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_DHT/Adafruit_DHT.h"

/*
 * RGBWeb - Mark II
 * Photon-based RGB LED control
 *
 * Copyright 2016 - Corban Mailloux
 * https://corb.co
 */
 
// DHT parameters
#define DHTPIN 5
#define DHTTYPE DHT11
// Variables
float temperature;
float humidity;

// DHT sensor
DHT dht(DHTPIN, DHTTYPE);
 
 
const int redPin = D0;
const int greenPin = D1;
const int bluePin = D2;

const int default_wait = 10;  // 10ms internal crossFade delay; increase for slower fades
const int hold = 0;           // Optional hold when a color is complete, before the next crossFade

/*
 * Cross fade setup
 */
// Color arrays
int black[3]  = { 0, 0, 0 };
int white[3]  = { 255, 255, 255 };
int red[3]    = { 255, 0, 0 };
int green[3]  = { 0, 255, 0 };
int blue[3]   = { 0, 0, 255 };

// Set initial color
int redVal = black[0];
int grnVal = black[1]; 
int bluVal = black[2];

// Initialize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

bool interruptFade = false;

String rgbColor;

void setup() {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    
    RGB.control(true);
    
    setColor(0, 0, 0);
    
    // Start DHT sensor
    dht.begin();
    
    Particle.function("updateColor", updateColor);
    Particle.variable("color", rgbColor);
    
    Particle.function("fadeColors", fadeColors);
    Particle.function("temperature", getTemp);
}

// void loop() {
//     temperature = dht.getTempFarenheit();
//     humidity = dht.getHumidity();
    
//     // Publish data
//     Spark.publish("temperature", String(temperature) + " °F");
//     delay(2000);
//     Spark.publish("humidity", String(humidity) + "%");
//     delay(2000);
// }

int getTemp(String UNUSED) {
    temperature = dht.getTempFarenheit();
    return int(temperature);
}

// Set the color, optionally fading to the next color.
// Input format = "RED,GREEN,BLUE[,FADE]"
// Where "RED," "GREEN," and "BLUE" are 0 - 255
// and "FADE" (optional) is the transition time in milliseconds
int updateColor(String rgbString) {
    interruptFade = true;
    
    // Minimum string = "0,0,0"
    // Maximum string = "255,255,255[,9999...]"
    if (rgbString.length() < 5) {
        return 1;
    }
    
    byte comma1 = rgbString.indexOf(',');
    byte comma2 = rgbString.indexOf(',', comma1 + 1); // Find the second comma
    
    byte comma3 = rgbString.lastIndexOf(','); // Optional last comma
    
    if (comma1 == -1 || comma2 == -1) {
        return 2;
    }
    
    int rgbValues[3];
    
    rgbValues[0] = rgbString.substring(0, comma1).trim().toInt();
    rgbValues[1] = rgbString.substring(comma1 + 1, comma2).trim().toInt();
    
    if (comma3 == -1 || comma2 == comma3) { // Normal mode, with no transition
        rgbValues[2] = rgbString.substring(comma2 + 1).trim().toInt();
        
        setColor(rgbValues[0], rgbValues[1], rgbValues[2]);
        return 3;
    }
    else {
        rgbValues[2] = rgbString.substring(comma2 + 1, comma3).trim().toInt();
        int wait = rgbString.substring(comma3 + 1).trim().toInt();
        
        interruptFade = false;
        crossFade(rgbValues, wait);
        return 4;
    }
    
    return 0;
}

void setColor(int red, int green, int blue) {
    RGB.color(red, green, blue);
    
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
    
    redVal = red;
    grnVal = green;
    bluVal = blue;
    
    rgbColor = String(red) + "," + String(green) + "," + String(blue);
}

int fadeColors(String alternateDelay) {
    interruptFade = false;
    
    int wait = default_wait;
    
    if (alternateDelay.length() > 0) {
        wait = alternateDelay.trim().toInt();
    }
    
    crossFade(red, wait);
    crossFade(green, wait);
    crossFade(blue, wait);
    crossFade(black, wait);
}


// From https://www.arduino.cc/en/Tutorial/ColorCrossfader
/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
* 
* The program works like this:
* Imagine a crossfade that moves the red LED from 0-10, 
*   the green from 0-5, and the blue from 10 to 7, in
*   ten steps.
*   We'd want to count the 10 steps and increase or 
*   decrease color values in evenly stepped increments.
*   Imagine a + indicates raising a value by 1, and a -
*   equals lowering it. Our 10 step fade would look like:
* 
*   1 2 3 4 5 6 7 8 9 10
* R + + + + + + + + + +
* G   +   +   +   +   +
* B     -     -     -
* 
* The red rises from 0 to 10 in ten steps, the green from 
* 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
* 
* In the real program, the color percentages are converted to 
* 0-255 values, and there are 1020 steps (255*4).
* 
* To figure out how big a step there should be between one up- or
* down-tick of one of the LED values, we call calculateStep(), 
* which calculates the absolute gap between the start and end values, 
* and then divides that gap by 1020 to determine the size of the step  
* between adjustments in the value.
*/
int calculateStep(int prevValue, int endValue) {
    int step = endValue - prevValue; // What's the overall gap?
    if (step) {                      // If its non-zero, 
        step = 1020/step;            //   divide by 1020
    }
    
    return step;
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1. 
*  (R, G, and B are each calculated separately.)
*/
int calculateVal(int step, int val, int i) {
    if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
        if (step > 0) {              //   increment the value if step is positive...
            val += 1;           
        } 
        else if (step < 0) {         //   ...or decrement it if step is negative
            val -= 1;
        } 
    }
    
    // Defensive driving: make sure val stays in the range 0-255
    if (val > 255) {
        val = 255;
    } 
    else if (val < 0) {
        val = 0;
    }
    
    return val;
}

/* crossFade() converts the percentage colors to a 
*  0-255 range, then loops 1020 times, checking to see if  
*  the value needs to be updated each time, then writing
*  the color values to the correct pins.
*/
void crossFade(int color[3], int wait) {
    // Convert to 0-255
    // int R = (color[0] * 255) / 100;
    // int G = (color[1] * 255) / 100;
    // int B = (color[2] * 255) / 100;
    
    int R = color[0];
    int G = color[1];
    int B = color[2];
    
    // Spark.publish("crossFade", String(R) + "," + String(G) + "," + String(B));
    
    int stepR = calculateStep(prevR, R);
    int stepG = calculateStep(prevG, G); 
    int stepB = calculateStep(prevB, B);

    for (int i = 0; i <= 1020; i++) {
        if (interruptFade) {
          break;
        }
        
        redVal = calculateVal(stepR, redVal, i);
        grnVal = calculateVal(stepG, grnVal, i);
        bluVal = calculateVal(stepB, bluVal, i);
        
        setColor(redVal, grnVal, bluVal); // Write current values to LED pins
        
        delay(wait); // Pause for 'wait' milliseconds before resuming the loop
    }
    
    // Update current values for next loop
    prevR = redVal; 
    prevG = grnVal; 
    prevB = bluVal;
    delay(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}
