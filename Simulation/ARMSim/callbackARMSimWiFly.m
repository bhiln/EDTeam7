function callbackARMSimWiFly( obj, event, h, AC )
% Callback for WiFly serial object on ARMSim side
% One would not actual do the communication in this way (i.e., via
% string with the terminator 'o'.). This is just to show how to
% Set up the callbacks and to communicate between the two programs.

% Keep around a persistent array that we add data to as we receive it.
% We also keep around the current plot axis data.
%global dataBuffer;
persistent dataBuffer;

% We are in the callback because we received some data. We calculate
% how much and then read it into a data array.
bytesAvailable = obj.BytesAvailable;
[recievedByte, ~, ~] = fread(obj, 1, 'char');
fprintf('%d\n', recievedByte);
if (length(dataBuffer) == 0)
    dataBuffer = [recievedByte];
else
    dataBuffer = [dataBuffer, recievedByte];
end

if (recievedByte == 255)
    %fprintf('len);
    if (dataBuffer(1) == 10)
        AC.setIRReady(false);
        
        x=uint8(dataBuffer(3)); % first byte
        y=uint8(dataBuffer(2)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(1, bitor(bytepack,uint16(y)));
        
        x=uint8(dataBuffer(5)); % first byte
        y=uint8(dataBuffer(4)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(2, bitor(bytepack,uint16(y)));
        
        x=uint8(dataBuffer(7)); % first byte
        y=uint8(dataBuffer(6)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(3, bitor(bytepack,uint16(y)));
        
        x=uint8(dataBuffer(9)); % first byte
        y=uint8(dataBuffer(8)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(4, bitor(bytepack,uint16(y)));
        
        x=uint8(dataBuffer(11)); % first byte
        y=uint8(dataBuffer(10)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(5, bitor(bytepack,uint16(y)));
        
        x=uint8(dataBuffer(13)); % first byte
        y=uint8(dataBuffer(12)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(6, bitor(bytepack,uint16(y)));
        
        x=uint8(dataBuffer(15)); % first byte
        y=uint8(dataBuffer(14)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(7, bitor(bytepack,uint16(y)));
        
        x=uint8(dataBuffer(17)); % first byte
        y=uint8(dataBuffer(16)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(8, bitor(bytepack,uint16(y)));
        
        x=uint8(dataBuffer(19)); % first byte
        y=uint8(dataBuffer(18)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(9, bitor(bytepack,uint16(y)));
        
        x=uint8(dataBuffer(21)); % first byte
        y=uint8(dataBuffer(20)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        AC.setIRSensor(10, bitor(bytepack,uint16(y)));
        
        AC.setIRReady(true);
        
        fprintf('RECIEVED SENSOR DATA\n');
        AC.plotSensorData();
    end
    if (dataBuffer(1) == 51)
        if (dataBuffer(2) ~= AC.getLastMessageID())
            fprintf('\n\nMISSED MOTOR MESSAGE: %d\n\n', AC.getLastMessageID());
        else
            fprintf('RECIEVED CORRECT ECHO: %d\n', dataBuffer(2));
            AC.setConfirmed(dataBuffer(2));
        end
    end
    if (dataBuffer(1) == 52)
        fprintf('MOVING STATUS UPDATE: %d\n', dataBuffer(2));
        AC.roverStep();
    end
    if (dataBuffer(1) == 53)
        fprintf('ROVER HAS FINISHED MOVING: %d\n', dataBuffer(2));
        AC.setDone(true);
    end
    
    dataBuffer = [];
end

if (recievedByte == 254)
    dataBuffer = [];
end

end



function isNextIndex = checkIndex( index )
    persistent lastIndex;
    fprintf('\n\n%d\n\n', lastIndex);

    if (isempty(lastIndex))
        lastIndex = index-1;
    end
    if (lastIndex+1 == index)
        isNextIndex = true;
    else
        %TODO: handle lost packet
        fprintf('Missed packet\n');
        isNextIndex = false;
    end
    lastIndex = index;
end

function callbackProcessSensor( obj, event )

end