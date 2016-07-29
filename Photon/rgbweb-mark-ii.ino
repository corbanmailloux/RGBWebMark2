/*
 * RGBWeb - Mark II
 * Photon-based RGB LED control
 *
 * Copyright 2016 - Corban Mailloux
 * https://corb.co
 */
 
const int redPin = D0;
const int greenPin = D1;
const int bluePin = D2;

/*
 * Cross fade setup
 */
// Color arrays
int black[3]  = { 0, 0, 0 };
int white[3]  = { 100, 100, 100 };
int red[3]    = { 100, 0, 0 };
int green[3]  = { 0, 100, 0 };
int blue[3]   = { 0, 0, 100 };

// Set initial color
int redVal = black[0];
int grnVal = black[1]; 
int bluVal = black[2];

int wait = 10;      // 10ms internal crossFade delay; increase for slower fades
int hold = 0;       // Optional hold when a color is complete, before the next crossFade

// Initialize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

bool interruptFade = false;

void setup() {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    
    RGB.control(true);
    
    setColor(0, 0, 0);
    
    Particle.function("updateColor", updateColor);
    Particle.function("fadeColors", fadeColors);
}

int updateColor(String rgbString) {
    interruptFade = true;
    
    // Minimum string = "0,0,0"
    // Maximum string = "255,255,255"
    if (rgbString.length() < 5) {
        return 1;
    }
    
    byte comma1 = rgbString.indexOf(',');
    byte comma2 = rgbString.lastIndexOf(',');
    
    if (comma1 == comma2 || comma1 == -1 || comma2 == -1) {
        return 2;
    }
    
    byte rgbValues[3];
    
    rgbValues[0] = rgbString.substring(0, comma1).trim().toInt();
    rgbValues[1] = rgbString.substring(comma1 + 1, comma2).trim().toInt();
    rgbValues[2] = rgbString.substring(comma2 + 1).trim().toInt();
    
    setColor(rgbValues[0], rgbValues[1], rgbValues[2]);
    
    return 0;
}

void setColor(byte red, byte green, byte blue) {
    RGB.color(red, green, blue);
    
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
    
    redVal = red;
    grnVal = green;
    bluVal = blue;
}

int fadeColors(String alternateDelay) {
    interruptFade = false;
    
    if (alternateDelay.length() > 0) {
        wait = alternateDelay.trim().toInt();
    }
    
    crossFade(red);
    crossFade(green);
    crossFade(blue);
    crossFade(black);
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
void crossFade(int color[3]) {
    // Convert to 0-255
    int R = (color[0] * 255) / 100;
    int G = (color[1] * 255) / 100;
    int B = (color[2] * 255) / 100;
    
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