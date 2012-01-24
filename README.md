HTTPCoffeePot
=============

***Special Features***

* 15 minute automatic shutoff
* Controlled via HTTP Requests
* Status, start, and stop requests
* Returns JSON for meant for AJAX interfaces
* Ready and Running LEDs.

HTTPCoffeePot can be connected to your arduino with an ethernet shield and attached to the network. It will listen on port 80, like a normal web server, and take several different request urls to control it. The urls return JSON with the intention that an ajax interface will be built over it:

* /status/ - This will return some information about the current status of the coffee pot
* /start/ - This will start the coffee pot assuming that it is currently ready to be started and not started already
* /stop/ - This will turn off the coffee pot


Currently the process will work like this:

1. Put in coffee grounds/filter/water
2. Press ready button to enter coffee pot into ready state
3. When coffee is desired, send /start/ request
4. Wait for coffee to finish
5. Send manual /stop/ request *OR* wait for timed automatic shut off to take place after 15 minutes


I have a very simple coffee maker which is really only a switch. The modifications I made to the pot were to cut the power inside of it between the wall and the switch on the pot and place a relay in between. The relay is then activated by the Arduino board when sent the /start/ request.

You can find the circuit diagram for the relay hookup [here](http://www.arduino.cc/playground/uploads/Main/relays.pdf)