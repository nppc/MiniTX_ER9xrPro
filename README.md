# MiniTX_ER9xrPro
Project for mini transmitter with gimbals from Er9xr Pro TX
# Software
Software based on arduinorc project (http://www.reseau.org/arduinorc) with mine modifications.

## Change Models
To chnage models or to configure the transmitter, press and hold button "C" for 2 seconds to enter/exit Configuration mode.
So, to change Model you need to go to Configuration mode, press "M" button, and then exit from Configuration mode.
Every press of "M" button changes to the next Model. If bext model is not configured, then it goes back to the first model.

## Arm/disarm
Swtches on side of the transmitter are used for safe arming. They are both connected to single analog channel A6 with resistors network.
If both swithces are OFF, then Throttle cut mode is activated.
If one of the swithces is ON, then Throttle cut mode is off.
For simple copters one of the switches can be used as arming switch.
For normal FC safe arming with 2 swithes is possible.

## Some Commands for configuring the transmitter
*To configure the transmitter, we need to go to Configure mode. In this mode also worcing Serial interface at 2400 baud rate.*
**DUMP** : print the values of all variables of current model
**DUMP GLOBAL** : print the values of the Global variables
**DUMP MODEL** : print the values of the Model variables of the current model
**DUMP [channel]** : print the values of the Channel variables of the current model
**DUMP MIXERS** : print the values of the Mixers variables of the current model
**MODEL [number]** : select the model.
**? (question mark)** : print the value of a variable (eg ? ICN1)
**? POTx** : print the analog value of potentiometer x (POT1 = pin A0 ... POT8 = pin A7)
**? PPM** : print the pulses widths of the PPM signal
**? SWx** : print the logical value of switch x (SW1 = pin D2 ... SW3 = pin D4)
**? VERSION** : print the version number of the software
**? VOLT** : print the battery voltage
**= (equal sign)** : change the value of a variable (eg ICN1=2)

## Variables for configuring transmitter
- **CDS** : current model ; this is the model number selected by the MODEL command.
- **ADS** : alternate model ; this model number is active when the Model switch is closed and MODEL_SWITCH_BEHAVIOUR = MODEL_SWITCH_SIMPLE in arduinotx_config.h
- **TSC** : throttle cutoff value used by the throttle security check, [0-511] default = 50. See also the Throttle calibration procedure.
- **BAT** : minimum voltage required for Tx operation, an alarm is triggered when voltage gets lower. Value in the range [0-1023] default = 740. See the Battery monitoring page.
- **KL1...KL8** : lowest analog value read on potentiometer #1, when the stick (and the hardware trim, if any) is at its lowest position. KL1...KL8 correspond to potentiometers 1 to 8. Value in the range [0-1023] default = 0. See the Potentiometer calibration procedure.
- **KH1...KH8** : highest analog value read on potentiometer #1, when the stick (and the hardware trim, if any) is at its highest position. KH1...KH8 correspond to potentiometers 1 to 8. Value in the range [0-1023] default = 1023. See the Potentiometer calibration procedure.

## Variables for configuring Models general data
- **NAM** : Model name, 8 characters, no spaces
- **THC** : Channel number used for throttle control, default=3, 0=no throttle channel.
- **PRT** : Protocol number of the Multiprotocol module that should be selected for current model 

## Variables for configuring every channel of selected Model
*The name of a variable is composed of 3 letters and one figure. The figure is a channel number.
For instance, REV1 is the Reverse setting for channel 1, REV2 is the Reverse setting for channel 2.*
- **ICT** : Input control type: 1=potentiometer, 2=switch, 3=mixer, 0=off (do not read this control)
- **ICN** : Input control number: potentiometer number or switch number. Potentiometer number belongs to the interval [1,8]; Potentiometer 1 is connected to A0, pot 2 to A1, ... pot 8 to A7. Switch number belongs to the interval [1,6]; Switch 1 is connected to D2, switch 2 to D3, ... switch 6 to D7. Mixer number belongs to the interval [1,2]
- **REV** : Reverse, 1=servo direction is reversed for this channel, 0=servo direction is not reversed.
- **DUA** : Dual rate applied to this channel when the Dual Rate switch is ON. Percentage [0-100]. Applies only if EXP=0.
- **EXP** : Exponential applied symetrically from the center of this channel when the Dual Rate switch is ON. Percentage [0-100]. 0=none, 25=medium, 50=strong, 100=very strong. If EXP is not zero then DUA is ignored. Exponential may also apply to the throttle channel (Model variable THC). This feature may be useful for gas engines. This exponential is calculated full-curve from 0.
- **PWL, PWH** : Minimal/Maximal pulse width for this channel, in microseconds. PWL default=720, PWH default=2200. These default values correspond to the Hextronic HXT500 servo and will accomodate most other servos. If your servos are moving erratically when you set the sticks near their minimum or maximum position, you must change these values: try with PWL=900 and PWH=2100. You can try to widen the default range to get a larger rotation angle if your servo supports it. See the Servo calibration procedure.
- **EPL, EPH** : End points position in percentage from the center of the channel [0,100]. EPL=low end point, EPH=high end point, both default=100. Set these variables if you want to limit the rotation angle of the servo.
- **SUB** : Subtrim signed percentage [-100,+100], default=0. Setting this variable shifts the neutral position of the servo in either direction, depending on the sign of the value: negative shifts left, positive shifts right.
