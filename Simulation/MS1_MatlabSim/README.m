% This is a simple example of how you should set up your matlab simulation
% code for MS 1 and beyond.

% For this example, there are two simulations, one for the "sensor" side
% and one for the "ARM" side. The two simulations run in separate Matlab
% command windows. They communicate with each other via two WiFlys that are
% connected to your computer by a serial interface over USB. The code for
% the two simulations are in separate directories. The "sensor" simlation
% in the directory "SensorSim" and the "ARM" simulation is in the directory
% "ARMSim".

% Of course, before you can run these simulations you have to have your
% WiFLys configured correctly. Follow the directions from the Scholar web
% site on how to configure your WiFlys to autoconnect. Make a note as to
% how to connect to these devices through the serial emulator on your
% computer.

% You will need to edit the two Matlab simulations so that you can use your
% particular WiFlys. For the SensorSim, you will have to change the line:
% ioSensorSimWiFly=serial('/dev/tty.usbserial-A702ZIPY','BaudRate',57600);
% in "SensorSim.m" to be the serial interface to one of your WiFlys. You
% will also have to change a similar line for the ARMSim in the file
% "ARMSim.m" to be the serial interface of the other of your WiFlys.

% To run the simulations, in one of the Matlab windows go to the SensorSim
% folder and type "SensorSim" in the command window. In the other Matlab
% window, go to the ARMSim folder and type "ARMSim". If all is well, the two
% simulations should talk to each other over the WiFlys and the ARMSim should
% draw a Matlab plot of what the simulated sensor is reading over on the
% SensorSim.

% In the window running the sensor simulation, you can change where the
% sensor is physically located in the simulation by typing:
% obj.setCurrentDistance(X)
% in the command window where X is a distance (e.g., 2.0) where you would
% like the sensor to be placed away from some object. You should see that
% the values in the plot in the ARMSim window change to be around this new
% value. Note that the Sensor simulation is adding some noise to the sensor
% data. Also, the range of the sensor data is between 1.0 and 10.0 feet.

% To stop the simulations you can run the two scripts "stopARMSim" and
% "stopSensorSim". These scripts will stop the timers, close the WiFly
% FIDs, clear all Matlab variables, and reset the graphics. You do not want
% to disconnect the USB cable to the WiFlys without closing the serial
% connections!

% Note that the Matlab code is based on event handing. The simulation
% events (e.g., the timers and receiving messages over WiFly) are handled
% asynchronously by the callback functions for each of these events. This
% is how you must write your code. Carefully look at how the callbacks are
% written, and how object data is passed to these callback so that external
% objects can be affected.

% Version 1.0 9/7/2014
% Version 1.1 9/8/2014 Added scripts to stop the two Sims