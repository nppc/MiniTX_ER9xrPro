# MiniTX_ER9xrPro
Project for mini transmitter with gimbals from Er9xr Pro TX
## Software
Software based on arduinorc project (http://www.reseau.org/arduinorc) with mine modifications.
To chnage models or to configure the transmitter, press and hold button "C" for 2 seconds to enter/exit Configuration mode.
So, to change Model you need to go to Configuration mode, press "M" button, and then exit from Configuration mode.
Every press of "M" button changes to the next Model. If bext model is not configured, then it goes back to the first model.

Swtches on side of the transmitter are used for safe arming. They are both connected to channel 5 with resistors network.
If both swithces are OFF, then Throttle cut mode activated.
If one of the swithces is ON, then Throttle cut mode is off.
For simple copters one of the switches can be used as arming switch.
For normal FC safe arming with 2 swithes is possible.
