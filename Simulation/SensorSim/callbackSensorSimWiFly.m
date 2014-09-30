function callbackSensorSimWiFly( obj, event, objSensor )
% This is the callback function for the WiFly on the sensor side
% One would not actual do the communication in this way (i.e., via
% string with the terminator 'o'.). This is just to show how to
% set up the callbacks and to communicate between the two programs.

persistent dataBuffer;

% We are in the callback because we received some data. We calculate
% how much and then read it into a data array.
bytesAvailable = obj.BytesAvailable;
[recievedByte, ~, ~] = fread(obj, 1, 'char');
if (isempty(dataBuffer))
    dataBuffer = [recievedByte];
else
    dataBuffer = [dataBuffer, recievedByte];
end

if (length(dataBuffer) > 1 && dataBuffer(length(dataBuffer)-1) == 127 && dataBuffer(length(dataBuffer)) == 0)
    messageType = dataBuffer(2);
    fprintf('callbackSimWiFly: Received %d bytes. Data: %s. Total of %d bytes read.\n',...
        bytesAvailable, dataBuffer, length(dataBuffer));

    if (messageType == 49)
        sensor_data_request(obj, objSensor);
    end
    
    dataBuffer = [];
end

end

function sensor_data_request(obj, objSensor)

persistent ntimes;

% The first time this is called, ntimes does not exist, otherwise
% we increment it.
if isempty(ntimes) 
    ntimes = 1;
else
    ntimes = mod((ntimes + 1), 256);
    fprintf('ntimes = %d\n', ntimes);
end
ntimes_Char = bin2dec(sprintf('%s', dec2bin(ntimes, 8)));

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

fprintf('callbackSimWiFly: Simulated sensor value is %s, ascii is %s, binary is %s, length is %d, data is %s, being sent to ARMSim\n',...
    sensorValue, sensorValue_String, sensorValue_String_Binary, sensorLength,...
    sprintf('%s%s%s%s%s%s', 'b', sprintf('%s', ntimes_Char), sprintf('%s', bin2dec(sprintf('%s', dec2bin(50, 8)))), sprintf('%s', bin2dec(sensorValue_Binary1)), sprintf('%s', bin2dec(sensorValue_Binary2)), 'e'));
%str = sprintf('x%d%s', sensorLength, sensorValue_String);
%fwrite(obj,str);
fwrite(obj,sprintf('%s', ntimes_Char));
fwrite(obj,sprintf('%s', bin2dec(sprintf('%s', dec2bin(50, 8)))));
fwrite(obj,sprintf('%s', bin2dec(sensorValue_Binary1)));
fwrite(obj,sprintf('%s', bin2dec(sensorValue_Binary2)));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(127, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(0, 8))));
%fwrite(obj,sensorValue_Binary2);

end
