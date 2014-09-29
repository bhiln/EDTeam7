function callbackSensorSimWiFly( obj, event, objSensor )
% This is the callback function for the WiFly on the sensor side
% One would not actual do the communication in this way (i.e., via
% string with the terminator 'o'.). This is just to show how to
% set up the callbacks and to communicate between the two programs.

% We keep around the number of times this timer has been called
persistent ntimes;

% We are in the callback because we received some data. We calculate
% how much and then read it into a data array.
bytesAvailable = obj.BytesAvailable;
[data count msg] = fread(obj, bytesAvailable);

% The first time this is called, ntimes does not exist, otherwise
% we increment it.
if isempty(ntimes) 
    ntimes = 1;
else
    ntimes = ntimes + 1;
end

% We print stuff out just to show that it is working.
valuesReceived = obj.ValuesReceived;
fprintf('callbackSimWiFly: Received %d bytes. Data: %s. Total of %d bytes read.\n',...
    bytesAvailable, data, valuesReceived);

% We get a sensor value from our simulated sensor and send it back
% as a string with the terminator 'o'. You will change this.
sensorValue = objSensor.getSensorReading() * 100;

%get data length
%sensorValue_String = sprintf('%f', sensorValue);
sensorValue_String_Binary = sprintf('%s', dec2bin(sensorValue, 16));
sensorValue_Binary1 = sensorValue_String_Binary(1:8);
sensorValue_Binary2 = sensorValue_String_Binary(9:16);
sensorValue_String = sprintf('%s%s', bin2dec(sensorValue_Binary1), bin2dec(sensorValue_Binary2));
sensorLength = length(sensorValue_String);

fprintf('callbackSimWiFly: Simulated sensor value is %s, ascii is %s, binary is %s, length is %d, being sent to ARMSim\n',...
    sensorValue, sensorValue_String, sensorValue_String_Binary, sensorLength);
%str = sprintf('x%d%s', sensorLength, sensorValue_String);
%fwrite(obj,str);
fwrite(obj,'x');
fwrite(obj,sprintf('%s', bin2dec(sensorValue_Binary1)));
fwrite(obj,sprintf('%s', bin2dec(sensorValue_Binary2)));
%fwrite(obj,sensorValue_Binary2);

end

