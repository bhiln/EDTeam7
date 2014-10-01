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

if (length(dataBuffer) > 1 && dataBuffer(length(dataBuffer)-1) == 255 && dataBuffer(length(dataBuffer)) == 0)
    messageType = dataBuffer(2);
    fprintf('callbackSimWiFly: Received %d bytes. Data: %s. Total of %d bytes read.\n',...
        bytesAvailable, dataBuffer, length(dataBuffer));

    % Message Handler------------------------------------------------
    if (messageType == 70 && checkIndex(dataBuffer(1)))
        %motor message
    end
    % Message Handler------------------------------------------------
    
    dataBuffer = [];
end

end

function isNextIndex = checkIndex( index )

persistent lastIndex;

if (isempty(lastIndex))
    lastIndex = index-1;
end
if (lastIndex == index-1)
    isNextIndex = true;
else
    %TODO: handle lost packet
    fprintf('Missed packet\n');
    isNextIndex = false;
end
lastIndex = index;
end

