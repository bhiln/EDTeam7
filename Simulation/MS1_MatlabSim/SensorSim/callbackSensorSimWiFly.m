function callbackSensorSimWiFly( obj, event, objSensor )
% This is the callback function for the WiFly on the sensor side
% One would not actual do the communication in this way (i.e., via
% string with the terminator 'o'.). This is just to show how to
% set up the callbacks and to communicate between the two programs.

% We are in the callback because we received some data. We calculate
% how much and then read it into a data array.
bytesAvailable = obj.BytesAvailable;
[data count msg] = fread(obj, bytesAvailable);
% We print stuff out just to show that it is working.
valuesReceived = obj.ValuesReceived;
fprintf('callbackSimWiFly: Received %d bytes. Total of %d bytes read.\n',...
    bytesAvailable,valuesReceived);

% We get a sensor value from our simulated sensor and send it back
% as a string with the terminator 'o'. You will change this.
sensorValue = objSensor.getSensorReading();
fprintf('callbackSimWiFly: Simulated sensor value is %f, being sent to ARMSim\n',...
    sensorValue);
str = sprintf('%fo',sensorValue);
fwrite(obj,str);

end

