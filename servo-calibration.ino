/* Servo Calibration

  Calibration of a continuous turn servo using an optical sensor (photo resistor)
  configured as an optical interrupter.

  When using multiple servos to drive wheels for a mobile bot, the (servos) need
  calibration to get the vehicle to drive in a straight line.  This is a small
  utility to collect the data and calculate the speed (revolutions per second) of
  a servo.

  Usage: On startup (reset), manually block and allow the light source to reach
  the sensor a few time.  Then, with the light NOT blocked, close the switch to
  drive the calibration pin low.  Timings will start with the following HIGH to
  LOW transition of the sensor pin.
 */
//
unsigned const int SERIAL_SPEED = 9600;
//300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200
unsigned const int SENSE_PIN = A0;
unsigned const int CALIB_PIN = 5;// Check for the end of calibration mode
unsigned const int MAX_ADC = 1023;// ADC == analog to digital conversion
unsigned const int MIN_ADC = 0;
const float DEAD_BAND_FACTOR = 0.25;// +/- 25% of the size of the complete range
unsigned const int FIXED_DEAD_BAND = 40;// +/- 40 should be plenty to prevent jitter/bounce

// (to be) calculated limits for values that will be used to switch between
// false (low value/voltage and true(high value/voltage.  The values will
// be the limits for the 'dead band', where no change in state will occur.
unsigned int threshold[2];

boolean adcState = HIGH;
unsigned long startTime;
unsigned long revolutionStart;
unsigned int revolutions = 0;

void setup() {
  pinMode(CALIB_PIN, INPUT);
  Serial.begin(SERIAL_SPEED);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  getActiveSenseRange();
  calibrateBooleanSense();
  startTime = synchronizeToRotation();
  revolutionStart = startTime;
}

void loop() {
  boolean senseState;
  unsigned long revolutionEnd, revolutionInterval, totalInterval;
  double revSpeed, averageSpeed;

  senseState = getAdcBoolean();
  if (adcState && !senseState) {
    // The ADC reading just went from HIGH to LOW
    // Increment revolution count, and calculate the speed
    revolutionEnd = millis();
    revolutions += 1;
    revolutionInterval = revolutionEnd - revolutionStart;
    totalInterval = revolutionEnd - startTime;
    // Single turn speed (rpm)
    revSpeed = 1000.0 / revolutionInterval; // revolutions/second
    averageSpeed = revolutions * 1000.0 / totalInterval;
    Serial.print("Revolution ");
    Serial.print(revolutions);
    Serial.print("; interval: ");
    Serial.print(revolutionInterval);
    Serial.print("; turn speed: ");
    Serial.print(revSpeed, 3);
    Serial.print("; average speed: ");
    Serial.print(averageSpeed, 3);
    Serial.println(" rev/second");
    revolutionStart = revolutionEnd;
  }
  adcState = senseState;
}

// Get multiple readings from the ADC pin, to determine the minumum and maximum
// range of values for the current environment.  For this to be effective, the
// user needs to vary the light reaching the sensor, to simulate the effect of
// the 'interruptor' attached to the wheel passing in front photo resistor.
// Typically this just means moving a dark card in front of and away from the
// sensor a few times, until the reported minimum and maximum values stop
// changing.  Then press the button to pull the calibration pin low.
void getActiveSenseRange() {
  unsigned int value;
  boolean newLimit = false;

  // Initialize to min/max values, so ANY reading will start adjusting them
  threshold[false] = MAX_ADC;
  threshold[true] = MIN_ADC;

  // Keep reading until the calibration line is pulled low
  while(digitalRead(CALIB_PIN)) {
    value = analogRead(SENSE_PIN);
    if (value < threshold[false]) {
      threshold[false] = value;
      newLimit = true;
    }
    if (value > threshold[true]) {
      threshold[true] = value;
      newLimit = true;
    }
    if (newLimit) {
      // Show the new range, so user can tell when readings quit changing
      Serial.print("new range [");
      Serial.print(threshold[false]);
      Serial.print(", ");
      Serial.print(threshold[true]);
      Serial.print("] at: ");
      Serial.println(millis());
    }
  }
}

// Calculation appropriate / reasonable values to the edges of the dead
// band where sensor reading values will not change the boolean state.
void calibrateBooleanSense() {
  unsigned int median, extent, halfBand;
  // Get the midpoint between the minimum and maximum values to use as
  // the base threshold between LOW and HIGH ADC values.
  median = (threshold[false] + threshold[true]) / 2;
  // Get the extent for the sense range.
  extent = threshold[true] - threshold[false];
  // Pick a dead-band size where the boolean state will not change.
  halfBand = min(extent * DEAD_BAND_FACTOR, FIXED_DEAD_BAND);

  // Set the threshold with the dead band range boundary limits
  threshold[false] = median - halfBand;
  threshold[true] = median + halfBand;
}

// Wait for the next HIGH to LOW boolean state change for the ADC value
// from the sensor.  This time this occurs will be the base time point
// for measuring rotation speed.
unsigned long synchronizeToRotation() {
  while(getAdcBoolean()) {}// Wait until the state goes low (light not blocked)
  while(!getAdcBoolean()) {}// Wait until the light is blocked
  return millis();// Reference time with the state switched to HIGH
}

// Determine the boolean state for the ADC value of the sensor
boolean getAdcBoolean() {
  unsigned int adc;
  adc = analogRead(SENSE_PIN);
  if (adcState) {
    // current state is ON/HIGH/true
    if (adc <= threshold[false]) {
      // sensor adc reading is at or below the lower end of the dead band
      return false;
    }
  } else {
    // current state is OFF/LOW/false
    if (adc >= threshold[true]) {
      // sensor adc reading is at or above the top end of the dead band
      return true;
    }
  }
  // sensor adc reading is in the dead band, or at the same end as previously
  return adcState;
}

unsigned int fixedPoint(float value) {
  // TODO return a formated string with inserted decimal point (3 decimal places)
  return (int)(value * 1000);
}
