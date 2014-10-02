function callbackSensorSimTimer( timerSim, event, obj, serialObj )
% SensorSim Timer callback
% Input:
% timerSim - Timer object (required argument for timers)
% event - Structure, contains information about timer call
%   (required argument for timers)
% obj - SensorSim object, contains information about Sensor

% Advance time on the Sensor (just because we can)
currentTime = obj.updateCurrentTime(timerSim.Period);

% Do an A/D Reading (as if we were the timer on the Sensor PIC)
obj.doADReading();

sendSensorData(serialObj, obj);

% Print something out so that we know the timer is running
fprintf('SensorSim Timer callback: AD updated, current time = %f\n',...
    currentTime);

end

function sendSensorData(obj, objSensor)

persistent ntimes;

% The first time this is called, ntimes does not exist, otherwise
% we increment it.
if isempty(ntimes) 
    ntimes = 1;
elseif (mod(ntimes, 256) == 12)
    ntimes = mod((ntimes + 2), 256);
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
fwrite(obj,bin2dec(sprintf('%s', dec2bin(255, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(0, 8))));
%fwrite(obj,sensorValue_Binary2);

end


