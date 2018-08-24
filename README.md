# EuroshieldDrums
Simple Drum for the Euroshield Eurorack Module by 1010 Music

The Code provides two drum voices that can be triggered by the Upper/Lower CV Inputs. It uses the Teensy Audio Library

![Euroshield Drums Audio Scheme](/EuroShieldDrums_Audio_Scheme.JPG)

The sound of the voices can be altered with several parameters. The button selects the setting, the potentiometer changes the value.

To prevent a parameter is changed to the actual value of the potentiometer while selecting a new parameter, the potentiometer needs to 'catch' the parameter value first. (E.g if the parameter has the value 0.0, the potentiometer needs to turned fully CCW before the parameter is going to be changed)

The LEDs displays the number of the settings in binary format. The LSB is the bottommost LED. At the moment there are 10 Setting available:

### Drums
See also https://www.pjrc.com/teensy/gui/index.html?info=AudioSynthSimpleDrum

#### 1:Frequency
Frequency of the Drum from 60 - 5000Hz - can be changed by definition

#### 2:Length
Set the duration of the envelope.(10 to 2000 ms) At the moment retriggering is not implemented. If a trigger is received while the envelope is active, it will be ignored.

*Sending a Gate Signal will continuously trigger the drum if the envelope is short enough. 

#### 3:2nd
Emulates a two-headed tom, by adding a second sine wave that is harmonized a perfect fifth above the base frequency. Using this involves a slight CPU penalty. 

#### 4:PitchMod 
Set the depth of envelope of the pitch, by a maximum of two octaves. Default is 0.5, with no modulation. Values above 0.5 cause the pitch to sweep downwards, values lower than 0.5 cause the pitch to sweep upwards. 

### Filter:
See also https://www.pjrc.com/teensy/gui/index.html?info=AudioFilterStateVariable

#### 5:Filter Mode
Chooding between LowPass, BandPass and HighPass Filter

#### 6:Filter Frequency
Corner Frequency of the Filter (60-5000 Hz)

#### 7: Filter Resonance
Amplifies near the Corner Frequency. Could lead to clipping if the Values aer to high

### Reverb:
See also https://www.pjrc.com/teensy/gui/index.html?info=AudioEffectFreeverb

#### 8:Reverb
How much Wet Signal mixed into the output (0% is dry signal only)

#### 9:Reverb Roomsize
Amount of reverb

#### 10: Reverb Dampening
More damping causes higher frequency echo to decay, creating a softer sound.

## Output
My Euroshield uses a ILI9342 TFT to display the values, but the relevant parts of the code are commented out by default.
Per Default is a Serial Out which prints out the values of both Voices every second. Look out for the Definitions at the beginning. 

The TFT commands should be compatible with the Adafruit libraries.

## KnownIssues

* Most often the lowest value of the potentitometer is 1, but sometimes it is 0. This causes the parameter values to be negative, which can lead to unexpected behaviour
* Serial Out sometimes prints out only values for one drum voice instead of two
