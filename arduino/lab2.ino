#include <arduinoFFT.h>
#include <SoftwareSerial.h>

#define DEBUG_PHOTO

#define BT_PIN_RX 11
#define BT_PIN_TX 12   // RX from BT -> pin 12 (TX on arduino)
#define BT_PIN_STATUS 13

#define SAMPLES 32              // Must be a power of 2
#define SAMPLING_FREQUENCY 1100 // Nyquist limit (2x of max frequency)

arduinoFFT FFT = arduinoFFT();
arduinoFFT FFT_photoDiode = arduinoFFT();
unsigned int sampling_period_us;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];   //phototransistor

double iReal[SAMPLES];    //photodiode
double iImag[SAMPLES];
const int analogPin_photoTransistor = A0;
const int analogPin_photoDiode = A2;
const int normalLED = 2;
const int alertLED = 4;
int offCounter = 0;
int onCounter = 0;

const int16_t SOM_MARKER = -32768;  // 0x8000
const int16_t EOM_MARKER = -32767;  // 0x8001
const int16_t ALERT      = 777;
const int16_t ACK        = 333;
bool pcConnected = false;
bool alreadySent = false;

unsigned long lastPrintTime = 0;
unsigned long printInterval = 500;

byte* sendingAsBytes = nullptr;

SoftwareSerial btSerial(BT_PIN_RX, BT_PIN_TX); // RX, TX

void setup() {
  analogReference(DEFAULT);
  Serial.begin(115200);
  btSerial.begin(9600);
  pinMode(normalLED, OUTPUT);
  pinMode(alertLED, OUTPUT);
  pinMode(BT_PIN_STATUS, INPUT);
  

  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

void loop() {
  // Acquire samples
  for (int i = 0; i < SAMPLES; i++) {
    microseconds = micros();
    vReal[i] = analogRead(analogPin_photoTransistor);
    vImag[i] = 0;

    iReal[i] = analogRead(analogPin_photoDiode);
    iImag[i] = 0;

    while (micros() < (microseconds + sampling_period_us)) {
      // Wait to satisfy the sampling frequency
    }
  }
  
  // Perform FFT
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  FFT_photoDiode.Windowing(iReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT_photoDiode.Compute(iReal, iImag, SAMPLES, FFT_FORWARD);
  FFT_photoDiode.ComplexToMagnitude(iReal, iImag, SAMPLES);

  // Find dominant frequency for transistor
  double dominantFrequency = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);


  // Manually search for specific frequency in photodiode
  double targetFrequency = 522.0;
  int targetIndex = round((double)SAMPLES * targetFrequency / SAMPLING_FREQUENCY);
  double targetAmplitude = iReal[targetIndex];
  double thresholdAmplitude = 60.0;

#ifdef DEBUG_FREQ
  // Check if the calculated frequency is within an expected range
  if (dominantFrequency >= 0 && dominantFrequency <= SAMPLING_FREQUENCY / 2) {
      Serial.print("Dominant Frequency: ");
      Serial.println(dominantFrequency);
  } else {
    Serial.println("Frequency out of bounds");
  }
#endif

  //Update connection status
  if(digitalRead(BT_PIN_STATUS) == LOW){
      pcConnected = false;
  }
  if(!pcConnected){
    if(digitalRead(BT_PIN_STATUS) == HIGH){
      pcConnected = true;
    } 
  }

  //Here, we check the phototransistor first, then check the photodiode only if ti fails. 
  //The photo transistor method does not have false positives. The goal of the diode is to handle the lamp
  //while the transistor handles range very well.
  // Handle LED based on the dominant frequency for phototransistor
  if (dominantFrequency >= 510 && dominantFrequency <= 550) {
    onCounter++;
    offCounter = 0;
    if (onCounter >= 5) {
      digitalWrite(normalLED, HIGH);
      digitalWrite(alertLED, LOW);
      alreadySent = false;
    }
    #ifdef DEBUG_PHOTO
      Serial.println("Phototransistor");
    #endif
  } else if (targetAmplitude >= thresholdAmplitude) { //check photodiode
    onCounter++;
    offCounter = 0;
    if (onCounter >= 3) {
      digitalWrite(normalLED, HIGH);
      digitalWrite(alertLED, LOW);
      alreadySent = false;
    }
    #ifdef DEBUG_PHOTO
      Serial.print("PhotoDiode: ");
      Serial.println(targetAmplitude);
    #endif
  } else {
    offCounter++;
    onCounter = 0;
    if (offCounter >= 3) {
      digitalWrite(alertLED, HIGH);
      digitalWrite(normalLED, LOW);
      // send the alert
      if (!alreadySent){
        send(true); 
      }
      alreadySent = true;
    }
      #ifdef DEBUG_PHOTO
      unsigned long currentTime = millis();
      if (currentTime - lastPrintTime >= printInterval) {
        Serial.print("PhotoDiode Not: ");
        Serial.println(targetAmplitude);
        lastPrintTime = currentTime; // Update the last print time
      }
      #endif
  }

  //send ack
  send(false);
}

void send(bool alertTriggered){
  if(alertTriggered){
    btSerial.write((byte*) &SOM_MARKER, sizeof(int16_t));
    btSerial.write((byte*) &ALERT, sizeof(int16_t));
    btSerial.write((byte*) &EOM_MARKER, sizeof(int16_t));
    #ifdef BT_DEBG
      Serial.println("Sent alert"); 
    #endif
  } else{
    btSerial.write((byte*) &SOM_MARKER, sizeof(int16_t));
    btSerial.write((byte*) &ACK, sizeof(int16_t));
    btSerial.write((byte*) &EOM_MARKER, sizeof(int16_t)); 
  }
}
