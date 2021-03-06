function callbackSensorSimWiFly( obj, event )
% This is the callback function for the WiFly on the sensor side
% One would not actual do the communication in this way (i.e., via
% string with the terminator 'o'.). This is just to show how to
% set up the callbacks and to communicate between the two programs.

persistent dataBuffer;

% We are in the callback because we received some data. We calculate
% how much and then read it into a data array.
bytesAvailable = obj.BytesAvailable;
[recievedByte, ~, ~] = fread(obj, 1, 'char');
fprintf('%d\n', recievedByte);
if (isempty(dataBuffer))
   dataBuffer = [recievedByte];
else
   dataBuffer = [dataBuffer, recievedByte];
end

if (recievedByte == 255)
    messageType = dataBuffer(2);
    fprintf('callbackSimWiFly: Received %d bytes. Data: %s. Total of %d bytes read.\n',...
        bytesAvailable, dataBuffer, length(dataBuffer));

    % Message Handler------------------------------------------------
    if (messageType == 50)
        %handle motor instruction
        fprintf('\n\nGOT MOTOR MESSAGE\n\n');
        sendAcknowledge(obj, dataBuffer(1));
        
        counter = 0;
        while (counter < dataBuffer(4))
            sendStatusUpdate(obj, dataBuffer(1));
            pause(.1);
            counter = counter + 1;
        end
        
        sendDone(obj, dataBuffer(1));
%         if (dataBuffer(1) == 3)
%             sendAcknowledge(obj, dataBuffer(1)+1);
%         else
%             sendAcknowledge(obj, dataBuffer(1));
%         end
    end
    % Message Handler------------------------------------------------
    
    dataBuffer = [];
end

if (recievedByte == 254)
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

function sendAcknowledge(obj, messageIndex)

fwrite(obj,bin2dec(sprintf('%s', dec2bin(254, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(51, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(messageIndex, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(255, 8))));

fprintf('SENDING MESSAGE\n');

end

function sendStatusUpdate(obj, messageIndex)

fwrite(obj,bin2dec(sprintf('%s', dec2bin(254, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(52, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(messageIndex, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(255, 8))));

fprintf('SENDING UPDATE\n');

end

function sendDone(obj, messageIndex)

fwrite(obj,bin2dec(sprintf('%s', dec2bin(254, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(53, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(messageIndex, 8))));
fwrite(obj,bin2dec(sprintf('%s', dec2bin(255, 8))));

fprintf('SENDING DONE\n');

end