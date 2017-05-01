/* arduinotx_config.h - Tx configuration ; all user-customizable settings are defined here
** Edit this file as needed. Normally, you do not have to make changes to any other file.
** 28-05-2014
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#ifndef arduinotx_config_h
#define arduinotx_config_h

// ----------------------------------------------------------------------------------------
// Hardware settings
// ----------------------------------------------------------------------------------------

// Number of channels [1,9]
// With the Arduino Nano, 8 analog input pins: A0 to A7 are available for
// proportional channels, and 6 digital input pins: D2 to D7 for discrete channels
// You can configure each channel to use either a potentiometer or a switch,
// see model variables ICT and ICN
#define CHANNELS 6

#include "arduinotx_eeprom.h" // this include must follow CHANNELS definition

// Number of potentiometers (8 max) installed in the transmitter
// With the Arduino Nano, 8 analog input pins: A0 to A7 are available for proportional channels
// Potentiometer 1 is connected to A0, pot 2 to A1, ... pot 8 to A7
#define NPOTS 5	// ch5 is for Flight Controller mode changes (CH5 has own circuity)

// Number of user switches (6 max) installed in the transmitter for discrete channels
// With the Arduino Nano, 6 digital input pins: D2 to D7 are available 
// Switch 1 is connected to D2, switch 2 to D3, ... switch 6 to D7
#define NSWITCHES  1 //(NPPC for future needs (change acro/stab mode for Eachine H8 for example)

// The led is connected to this pin
#define LED_PIN  13

// Buzzer (optional); comment this line if you do not implement it
#define BUZZER_ENABLED
// The buzzer is connected to this pin
#define BUZZER_PIN 7

// Battery check (optional); comment this line if you do not implement it
#define BATCHECK_ENABLED
// Analog pin connected to the 1/2 voltage divider
#define BATCHECK_PIN  A7
// Battery voltage correction: if PRINT VOLT does not return the exact voltage measured with a multimeter,
// then you define the correction here. In my case, PRINT VOLT returns 7.8V and my multimeter 7.7V
// so the correction should be - 0.1V ; and since 1V is 102 (in ADC units, see ArduinoTx::ReadBattery()) then 0.1V is int(10.2) = - 10 ADC units
// like this: #define BATVOLT_CORRECTION -10
#define BATVOLT_CORRECTION 0

// Switches used to configure the transmitter, not to control channels
// A switch is ON when opened because there is a pullup resistor on the corresponding input
#define MODE_SWITCH_PIN 9			// opened=transmission, closed=command mode
#define THROTTLECUT_SWITCH_PIN A4	// NPPC (Same as POT5 (ch5). While reading this pin we will invert value. Pulldown resistor is connected here)
//#define DUALRATE_SWITCH_PIN 12	// opened=dual rate ON, closed=OFF (NPPC we will not using Dual Rates switch)
#define MODEL_SWITCH_PIN 8		// opened=use model selected by the MODEL command (CDS global variable), closed=use model corresponding to the ADS global variable

// define two pins for choosing protocol of MultiProtocol module (currently only three protocols)
#define MULTIPROTOCOL_SWITCH_MODE_PIN 12	// connected as push button. Will be read during startup and changing to one of three protocols.
#define MULTIPROTOCOL_POWER_MODULE1_PIN 3		// On/Off power to TX module1
#define MULTIPROTOCOL_POWER_MODULE2_PIN 4		// On/Off power to TX module2
#define MULTIPROTOCOL_CONTROL1_PIN 5		// connected as rotary encoder of MultiProtocol module
#define MULTIPROTOCOL_CONTROL2_PIN 6		// connected as rotary encoder of MultiProtocol module
#define MULTIPROTOCOL_POWER_TX_PIN 11		// On/Off power to TX arduino


// next 2 const are used only if MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_ROTATING (see below)
//#define MODEL_ROTATING_SWITCH_PIN A6 		// use an analog input to read the rotating switch position (NPPC we will reuse this pin)
#define MODEL_ROTATING_SWITCH_STEPS 10 	// my rotating switch has 10 steps; you must customize this value

// ----------------------------------------------------------------------------------------
// Software settings
// ----------------------------------------------------------------------------------------

// Model switch behaviour ; you can choose among 3 options:
#define MODEL_SWITCH_SIMPLE 1		// Option #1: switching between 2 models: opened=use model selected by the MODEL command (CDS global variable), closed=use model corresponding to the ADS global variable
#define MODEL_SWITCH_STEPPING 2	// Option #2: stepping through all NDATASETS models. In Command mode, CDS is incremented each time the switch is briefly closed.
//#define MODEL_SWITCH_ROTATING 3	// Option #3: selecting a model among all NDATASETS models with a rotating switch (NPPC never use this mode)
#define MODEL_SWITCH_BEHAVIOUR MODEL_SWITCH_STEPPING // defines the desired behaviour of the Model switch

// Endpoints algorithm : you can choose among 2 options:
#define ENDPOINTS_LIMITED	1	// Option #1: the control stick has 2 dead-angles corresponding to each endpoint. Moving the stick beyond this angle will have no effect on the PPM signal.
#define ENDPOINTS_BILINEAR 2	// Option #2: the control stick has no dead-angles: moving it from min to max will output a PPM signal within the endpoints interval. However, the variation rate of the signal in the lower half of the interval will not be the same as in the higher half if CHAN_EPL != CHAN_EPH. This may be acceptable or not.
#define ENDPOINTS_ALGORITHM ENDPOINTS_BILINEAR

#endif
