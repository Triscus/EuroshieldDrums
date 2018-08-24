#include "Audio.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <elapsedMillis.h>

//uncomment here if you want the Serial Out or TFT
//#define TFT
#define SERIAL_OUT

#ifdef TFT
#include "ILI9341_t3.h"
//---------------DISPLAY-------------------
#define DC 10
#define CS 15
#define RST 255
#define MOSI 7
#define SCLK 14
#define MISO 12

ILI9341_t3 tft = ILI9341_t3(CS, DC, RST, MOSI, SCLK, MISO);
#endif // TFT

elapsedMillis timeElapsed;
elapsedMillis timeElapsed_Serial;
elapsedMillis drumUpperTriggered;
elapsedMillis drumLowerTriggered;

int displayRefreshInterval = 25;
int drumTrigger;

// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drumLower;      //xy=113,1089
AudioSynthSimpleDrum     drumUpper;      //xy=132,923
AudioInputI2S            audioInput;     //xy=194,1361
AudioFilterStateVariable filterLower;    //xy=284,1158
AudioFilterStateVariable filterUpper;        //xy=307,948
AudioMixer4              filterMixerUpper;         //xy=502,941
AudioMixer4              filterMixerLower;         //xy=504,1160
AudioAnalyzePeak         peakCVUpper;    //xy=508,1405
AudioAnalyzePeak         peakCVLower;    //xy=514,1466
AudioEffectFreeverb      reverbUpper;    //xy=713,1010
AudioEffectFreeverb      reverbLower;    //xy=718,1258
AudioMixer4              reverbMixerUpper;     //xy=913,1002
AudioMixer4              reverbMixerLower;     //xy=922,1199
AudioOutputI2S           audioOutput;    //xy=1214,1156
AudioConnection          patchCord1(drumLower, 0, filterLower, 0);
AudioConnection          patchCord2(drumUpper, 0, filterUpper, 0);
AudioConnection          patchCord3(audioInput, 0, peakCVUpper, 0);
AudioConnection          patchCord4(audioInput, 1, peakCVLower, 0);
AudioConnection          patchCord5(filterLower, 0, filterMixerLower, 0);
AudioConnection          patchCord6(filterLower, 1, filterMixerLower, 1);
AudioConnection          patchCord7(filterLower, 2, filterMixerLower, 2);
AudioConnection          patchCord8(filterUpper, 0, filterMixerUpper, 0);
AudioConnection          patchCord9(filterUpper, 1, filterMixerUpper, 1);
AudioConnection          patchCord10(filterUpper, 2, filterMixerUpper, 2);
AudioConnection          patchCord11(filterMixerUpper, reverbUpper);
AudioConnection          patchCord12(filterMixerUpper, 0, reverbMixerUpper, 0);
AudioConnection          patchCord13(filterMixerLower, 0, reverbMixerLower, 0);
AudioConnection          patchCord14(filterMixerLower, reverbLower);
AudioConnection          patchCord15(reverbUpper, 0, reverbMixerUpper, 1);
AudioConnection          patchCord16(reverbLower, 0, reverbMixerLower, 1);
AudioConnection          patchCord17(reverbMixerUpper, 0, audioOutput, 0);
AudioConnection          patchCord18(reverbMixerLower, 0, audioOutput, 1);
AudioControlSGTL5000     audioShield;    //xy=452,1366
										 // GUItool: end automatically generated code

// Arrays for Upper / Lower Drum Voices
AudioAnalyzePeak*         peak[2];
AudioSynthSimpleDrum* drums[2] = { &drumUpper,&drumLower };
AudioEffectFreeverb* reverb[2] = { &reverbUpper,&reverbLower };
AudioMixer4* reverbMixer[2] = { &reverbMixerUpper,&reverbMixerLower };
AudioMixer4* filterMixer[2] = { &filterMixerUpper,&filterMixerLower };
AudioFilterStateVariable* filter[2] = { &filterUpper,&filterLower };
// GUItool: end automatically generated code

//---------------POTS-------------------

#define POT_MIN_VALUE 1
#define POT_MAX_VALUE 1022

int upperPotInput = 20;
int lowerPotInput = 21;
float SHRT_MAX_AS_FLT = 32767.0;
float POT_MAX_AS_FLT = 1023.0;
int potInput[2] = { upperPotInput, lowerPotInput };
float potValue[2] = { 0.0,0.0 };
float potValueLast[2] = { 0.0,0.0 };
float smoothedPotValue[2] = { 0.0,0.0 };
bool potTracking[2] = { false,false };

//----------BUTTON-----------
int buttonInput = 2;
int lastButtonState = -1;
long lastButtonMS = 0;
int  mode = 0;
int  lastmode = 0;
#define debounceMS    100

//----------LEDS---------------
#define ledPinCount   4
int     ledPins[ledPinCount] = { 3, 4, 5, 6 };
uint8_t ledState = 1;

//---------------CV INPUT--------------

double peakCV[2] = { 0.0,0.0 };
double peakCVLastRawValue[2] = { 0.0,0.0 };
double maxPeakCV[2];

//-----DRUMS-----------
#define DRUM_COUNT 2
#define DRUM_UPPER 0
#define DRUM_LOWER 1
#define MIXER_CHANNEL_DRY 0
#define MIXER_CHANNEL_REVERB 1
#define MIN_DRUM_FREQ 60
#define MAX_DRUM_FREQ 5000
#define MIN_DRUM_LENGTH 10
#define MAX_DRUM_LENGTH 2000

int drumFreq[2] = { 100,500 };
int drumFreq_new[2];

int drumLength[2] = { 100,100 };
int drumLength_new[2];

double drumSecondMix[2] = { 1.0,1.0 };
double drumSecondMix_new[2];

double drumPitchMod[2] = { 0.5,0.5 };
double drumPitchMod_new[2];

double filterFreq[2] = { MAX_DRUM_FREQ,MAX_DRUM_FREQ };
double filterFreq_new[2];

double filterReso[2] = { 0.0,0.0 };
double filterReso_new[2];

double reverbLvl[2] = { 0.0,0.0 };
double reverbLvl_new[2] = { 0.0,0.0 };

double dryLvl[2] = { 1.0,1.0 };
double dryLvl_new[2];

double roomsize[2] = { 0.0,0.0 };
double roomsize_new[2];

double dampening[2] = { 0.0,0.0 };
double dampening_new[2];

double filterMixerGain[2][3] = {
	{1,0, 0},
	{1,0, 0}
};

//Array for holding the Mixer Config for Filter Modes
double mixerSettings[3][3] =
{
	{1.0,0.0,0.0},	// LowPass
	{0.0,1.0,0.0},	// BandPass
	{0.0,0.0,1.0}	// HighPass
};

uint8_t filterModeIndex[2] = { 0,0 };
uint8_t filterModeIndex_new[2];

//--------UI-----------
#define NUMBER_OF_MODES 10
char* modeText[NUMBER_OF_MODES] = { "Freq       ","Length     ","2nd        ","Pitch      ","Filter Mode","Filter Freq","Filter Reso","Reverb     " ,"Roomsize   ","Dampening  " };
int maxMem;
#define NUMBER_OF_FILTER_MODES 3
#define NUMBER_OF_FILTER_MODES_F 3.0
char* filterModeText[NUMBER_OF_FILTER_MODES] = { "Low Pass","Band Pass", "High Pass" };

void setup()
{
#ifdef TFT
	//-----DISPLAY-----
	tft.begin();
	tft.setRotation(2);
	tft.fillScreen(ILI9341_BLACK);
	//tft.setFont(Arial_10);
	tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
	tft.setTextSize(1);
	tft.println("Initializing...");
#endif

	Serial.begin(15200);

	//-----AUDIO-----
	AudioMemory(128);

	audioShield.enable();
	audioShield.inputSelect(AUDIO_INPUT_LINEIN);
	audioShield.volume(1.0);
	audioShield.adcHighPassFilterDisable();
	audioShield.lineInLevel(0, 0);

#ifdef TFT
	tft.fillScreen(ILI9341_BLACK);
	tft.drawFastHLine(0, 160, 240, ILI9341_WHITE);
	tft.setCursor(0, 0);
	tft.println(modeText[mode]);
#endif
	peak[DRUM_UPPER] = &peakCVUpper;
	peak[DRUM_LOWER] = &peakCVLower;

	//initial Configuration
	for (int i = 0; i < DRUM_COUNT; i++) {
		drums[i]->frequency(drumFreq[i]);
		drums[i]->length(drumLength[i]);
		drums[i]->secondMix(drumSecondMix[i]);
		drums[i]->pitchMod(drumPitchMod[i]);
		filter[i]->frequency(MAX_DRUM_FREQ);
		filter[i]->resonance(filterReso[i]);
		for (int x = 0; x < 3; x++)
		{
			filterMixer[i]->gain(x, filterMixerGain[i][x]);
		}
	}

	//Pin configuration
	for (int i = 0; i < ledPinCount; i++)
		pinMode(ledPins[i], OUTPUT);
	pinMode(buttonInput, INPUT);

	//Setting the LEDS to Frequency Mode
	digitalWrite(ledPins[3], LOW);
	digitalWrite(ledPins[2], LOW);
	digitalWrite(ledPins[1], LOW);
	digitalWrite(ledPins[0], HIGH);
}

void loop()
{
	int buttonState = digitalRead(buttonInput);

	if (buttonState != lastButtonState) {
		long ms = millis();
		if (ms - lastButtonMS > debounceMS) {
			if (buttonState == LOW) {
				mode = (mode + 1) % NUMBER_OF_MODES;
				maxMem = AudioMemoryUsage();

#ifdef TFT

				tft.setCursor(0, 0);
				tft.println(modeText[mode]);
				tft.print(maxMem);

#endif
				//calculating the LEDS
				if (((mode + 1) % 2) > 0) { digitalWrite(ledPins[0], HIGH); }
				else { digitalWrite(ledPins[0], LOW); }

				if (((mode + 1) % 4) > 1) { digitalWrite(ledPins[1], HIGH); }
				else { digitalWrite(ledPins[1], LOW); }

				if (((mode + 1) % 8) > 3) { digitalWrite(ledPins[2], HIGH); }
				else { digitalWrite(ledPins[2], LOW); }

				if (((mode + 1) % 16) > 7) { digitalWrite(ledPins[3], HIGH); }
				else { digitalWrite(ledPins[3], LOW); }
			}
		}
		lastButtonMS = ms;
		lastButtonState = buttonState;
	}

	//reset potTracking if the mode has changes
	if (lastmode != mode) {
		potTracking[0] = false;
		potTracking[1] = false;
		lastmode = mode;
	}

	for (int i = 0; i < DRUM_COUNT; i++) { //for each Drum Voice [i]
		//smooth the values a bit
		int bothValues;
		bothValues = potValueLast[i] + analogRead(potInput[i]);
		potValue[i] = bothValues / 2;

		switch (mode) {
		case 0: //Frequency

			//Map the Pot Value to Frequency

			drumFreq_new[i] = map(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, MIN_DRUM_FREQ, MAX_DRUM_FREQ);

			//if the actual pot value is near the set value, activate pot tracking
			if (drumFreq_new[i] < drumFreq[i] + 10 && drumFreq_new[i] > drumFreq[i] - 10)
			{
				potTracking[i] = true;
			}

			//set the parameter value
			if (potTracking[i] == true)
			{
				drumFreq[i] = drumFreq_new[i];
			}

			break;

		case 1: //Length
			drumLength_new[i] = map(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, MIN_DRUM_LENGTH, MAX_DRUM_LENGTH);

			if (drumLength_new[i] < drumLength[i] + 10 && drumLength_new[i] > drumLength[i] - 10)
			{
				potTracking[i] = true;
			}

			if (potTracking[i] == true)
			{
				drumLength[i] = drumLength_new[i];
			}

			break;

		case 2: //SecondMix
			drumSecondMix_new[i] = mapfloat(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, 0.0, 1.0);

			if (drumSecondMix_new[i] < drumSecondMix[i] + 0.01 && drumSecondMix_new[i] > drumSecondMix[i] - 0.01)
			{
				potTracking[i] = true;
			}

			if (potTracking[i] == true)
			{
				drumSecondMix[i] = drumSecondMix_new[i];
			}

			break;

		case 3: //Pitch Modulation

			drumPitchMod_new[i] = mapfloat(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, 0.0, 1.0);

			if (drumPitchMod_new[i] < drumPitchMod[i] + 0.01 && drumPitchMod_new[i] > drumPitchMod[i] - 0.01)
			{
				potTracking[i] = true;
			}

			if (potTracking[i] == true)
			{
				drumPitchMod[i] = drumPitchMod_new[i];
			}

			break;

		case 4: //FilterMode

			filterModeIndex_new[i] = (int)(NUMBER_OF_FILTER_MODES_F*potValue[i] / 1023);

			//filterFreq_new[i] = map(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, 0.0, MAX_DRUM_FREQ);

			if (filterModeIndex_new[i] == filterModeIndex[i])

			{
				potTracking[i] = true;
			}

			if (potTracking[i] == true)
			{
				filterModeIndex[i] = filterModeIndex_new[i];
			}

			break;
		case 5: //FilterFreq

			filterFreq_new[i] = map(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, 0.0, MAX_DRUM_FREQ);

			if (filterFreq_new[i] < filterFreq[i] + 3 && filterFreq_new[i] > filterFreq[i] - 3)
			{
				potTracking[i] = true;
			}

			if (potTracking[i] == true)
			{
				filterFreq[i] = filterFreq_new[i];
			}

			break;

		case 6: //Filter Resonance

			filterReso_new[i] = map(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, 0.0, 5.0);

			if (filterReso_new[i] < filterReso[i] + 0.01 && filterReso_new[i] > filterReso[i] - 0.01)
			{
				potTracking[i] = true;
			}

			if (potTracking[i] == true)
			{
				filterReso[i] = filterReso_new[i];
			}

			break;
		case 7: //Reverb - Level

			reverbLvl_new[i] = mapfloat(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, 0.0, 1.0);

			if (reverbLvl_new[i] < reverbLvl[i] + 0.01 && reverbLvl_new[i] > reverbLvl[i] - 0.01)
			{
				potTracking[i] = true;
			}

			if (potTracking[i] == true)
			{
				reverbLvl[i] = reverbLvl_new[i];
			}

			break;

		case 8: //Reverb -Roomsize

			roomsize_new[i] = mapfloat(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, 0.0, 1.0);

			if (roomsize_new[i] < roomsize[i] + 0.01 && roomsize_new[i] > roomsize[i] - 0.01)
			{
				potTracking[i] = true;
			}

			if (potTracking[i] == true)
			{
				roomsize[i] = roomsize_new[i];
			}

			break;

		case 9: //Reverb-Dampening

			dampening_new[i] = mapfloat(potValue[i], POT_MIN_VALUE, POT_MAX_VALUE, 0.0, 1.0);

			if (dampening_new[i] < dampening[i] + 0.01 && dampening_new[i] > dampening[i] - 0.01)
			{
				potTracking[i] = true;
			}

			if (potTracking[i] == true)
			{
				dampening[i] = dampening_new[i];
			}
			break;
		}

		potValueLast[i] = potValue[i];

		// set the parameters
		drums[i]->frequency(drumFreq[i]);
		drums[i]->length(drumLength[i]);
		drums[i]->secondMix(drumSecondMix[i]);
		drums[i]->pitchMod(drumPitchMod[i]);
		reverb[i]->roomsize(roomsize[i]);
		reverb[i]->damping(dampening[i]);
		dryLvl[i] = 1.0 - reverbLvl[i];
		reverbMixer[i]->gain(MIXER_CHANNEL_DRY, dryLvl[i]);
		reverbMixer[i]->gain(MIXER_CHANNEL_REVERB, reverbLvl[i]);
		filter[i]->frequency(filterFreq[i]);
		filter[i]->resonance(filterReso[i]);

		for (int channel = 0; channel < 3; channel++)
		{
			filterMixer[i]->gain(channel, mixerSettings[filterModeIndex[i]][channel]);
		}

		//read the CV Inputs / Trigger
		peakCV[i] = peakCVLastRawValue[i];
		if (peak[i]->available()) {
			peakCV[i] = peak[i]->read();

			if (maxPeakCV[i] < peakCV[i])

			{
				maxPeakCV[i] = peakCV[i];
			}
		}
		peakCVLastRawValue[i] = peakCV[i];

		//play the drums when the peak value is > 0.5 and the former drum hit has ended
		if (i == DRUM_UPPER)
		{
			if (peakCV[i] > 0.5 && drumUpperTriggered > drumLength[i])
			{
				drums[i]->noteOn();

				drumUpperTriggered = 0;
			}
		}
		else //lower drums
		{
			if (peakCV[i] > 0.5 && drumLowerTriggered > drumLength[i])
			{
				drums[i]->noteOn();
				drumLowerTriggered = 0;
			}
		}

		//-------------OUTPUT / DEBUG--------

#ifdef TFT

		if (timeElapsed > displayRefreshInterval)

		{
			if (i == DRUM_UPPER)
			{
				tft.setCursor(0, 20);

				tft.println("Drum One");
			}
			else
			{
				tft.setCursor(0, 165);
				tft.println("Drum Two");
			}

			tft.print("Freq:       ");
			tft.print(drumFreq[i]);
			tft.println("  ");

			tft.print("Length:     ");
			tft.print(drumLength[i]);
			tft.println("  ");

			tft.print("2nd:        ");
			tft.print(drumSecondMix[i]);
			tft.println("  ");

			tft.print("PitchMod:   ");
			tft.print(drumPitchMod[i]);
			tft.println("  ");

			tft.print("FilterMode: ");
			tft.print(filterModeText[filterModeIndex[i]]);
			tft.println("  ");

			tft.print("FilterFreq: ");
			tft.print(filterFreq[i]);
			tft.println("  ");

			tft.print("FilterReso: ");
			tft.print(filterReso[i]);
			tft.println("  ");

			tft.print("Reverb:     ");
			tft.print(reverbLvl[i] * 100);
			tft.print("%");
			tft.println("  ");

			tft.print("Roomsize:   ");
			tft.print(roomsize[i] * 100);
			tft.print("%");
			tft.println("  ");

			tft.print("Dampening:  ");
			tft.print(dampening[i] * 100);
			tft.print("%");
			tft.println("  ");
			tft.println(potValue[i]);
			for (int x = 0; x < 3; x++)
			{
				tft.println(mixerSettings[filterModeIndex[i]][x]);
			}

			timeElapsed = 0;
		}
#endif

#ifdef SERIAL_OUT
		if (timeElapsed_Serial > 1000) {
			Serial.print("DRUM");
			Serial.print(i);
			Serial.print("---");

			Serial.print("Freq:");
			Serial.print(drumFreq[i]);
			Serial.print("---");

			Serial.print("Length:");
			Serial.print(drumLength[i]);
			Serial.print("---");

			Serial.print("2nd:");
			Serial.print(drumSecondMix[i]);
			Serial.print("---");

			Serial.print("PitchMod:");
			Serial.print(drumPitchMod[i]);
			Serial.print("---");

			Serial.print("FilterMode:");
			Serial.print(filterModeText[filterModeIndex[i]]);
			Serial.print("---");

			Serial.print("FilterFreq:");
			Serial.print(filterFreq[i]);
			Serial.print("---");

			Serial.print("FilterReso:");
			Serial.print(filterReso[i]);
			Serial.print("---");

			Serial.print("Reverb:");
			Serial.print(reverbLvl[i] * 100);
			Serial.print("%");
			Serial.print("---");

			Serial.print("Roomsize:");
			Serial.print(roomsize[i] * 100);
			Serial.print("%");
			Serial.print("---");

			Serial.print("Dampening:");
			Serial.print(dampening[i] * 100);
			Serial.print("%");
			Serial.print("---");
			Serial.print("PotValue:");
			Serial.println(potValue[i]);

			if (i == DRUM_LOWER)

			{
				timeElapsed_Serial = 0;
				Serial.println("---");
			}
		}

#endif
	}
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) //function for mapping floats
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}