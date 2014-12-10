function callbackARMSimWiFly( obj, event, h, AC, otherWiFly )
% Callback for WiFly serial object on ARMSim side
% One would not actual do the communication in this way (i.e., via
% string with the terminator 'o'.). This is just to show how to
% Set up the callbacks and to communicate between the two programs.

% Keep around a persistent array that we add data to as we receive it.
% We also keep around the current plot axis data.
%global dataBuffer;
persistent dataBuffer;
persistent sensor1;
persistent sensor2;
persistent sensor3;
persistent sensor4;
persistent sensor5;
persistent sensor6;
persistent sensor7;
persistent sensor8;
persistent sensor9;
persistent sensor10;

if isempty(sensor1)
    sensor1 = [0,0,0,0,0];
    sensor2 = [0,0,0,0,0];
    sensor3 = [0,0,0,0,0];
    sensor4 = [0,0,0,0,0];
    sensor5 = [0,0,0,0,0];
    sensor6 = [0,0,0,0,0];
    sensor7 = [0,0,0,0,0];
    sensor8 = [0,0,0,0,0];
    sensor9 = [0,0,0,0,0];
    sensor10 = [0,0,0,0,0];
end

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
        sensor1 = [sensor1(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(1, (sensor1(1) + sensor1(2) + sensor1(3) + sensor1(4) + sensor1(5))/5);
        
        x=uint8(dataBuffer(5)); % first byte
        y=uint8(dataBuffer(4)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        sensor2 = [sensor2(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(2, (sensor2(1) + sensor2(2) + sensor2(3) + sensor2(4) + sensor2(5))/5);
        
        x=uint8(dataBuffer(7)); % first byte
        y=uint8(dataBuffer(6)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        sensor3 = [sensor3(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(3, (sensor3(1) + sensor3(2) + sensor3(3) + sensor3(4) + sensor3(5))/5);
        
        x=uint8(dataBuffer(9)); % first byte
        y=uint8(dataBuffer(8)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        sensor4 = [sensor4(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(4, (sensor4(1) + sensor4(2) + sensor4(3) + sensor4(4) + sensor4(5))/5);
        
        x=uint8(dataBuffer(11)); % first byte
        y=uint8(dataBuffer(10)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        sensor5 = [sensor5(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(5, (sensor5(1) + sensor5(2) + sensor5(3) + sensor5(4) + sensor5(5))/5);
        
        x=uint8(dataBuffer(13)); % first byte
        y=uint8(dataBuffer(12)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        sensor6 = [sensor6(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(6, (sensor6(1) + sensor6(2) + sensor6(3) + sensor6(4) + sensor6(5))/5);
        
        x=uint8(dataBuffer(15)); % first byte
        y=uint8(dataBuffer(14)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        sensor7 = [sensor7(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(7, (sensor7(1) + sensor7(2) + sensor7(3) + sensor7(4) + sensor7(5))/5);
        
        x=uint8(dataBuffer(17)); % first byte
        y=uint8(dataBuffer(16)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        sensor8 = [sensor8(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(8, (sensor8(1) + sensor8(2) + sensor8(3) + sensor8(4) + sensor8(5))/5);
        
        x=uint8(dataBuffer(19)); % first byte
        y=uint8(dataBuffer(18)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        sensor9 = [sensor9(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(9, (sensor9(1) + sensor9(2) + sensor9(3) + sensor9(4) + sensor9(5))/5);
        
        x=uint8(dataBuffer(21)); % first byte
        y=uint8(dataBuffer(20)); % second byte
        bytepack=uint16(x);
        bytepack=bitshift(bytepack,8);
        sensor10 = [sensor10(2:5), bitor(bytepack,uint16(y))];
        AC.setIRSensor(10, (sensor10(1) + sensor10(2) + sensor10(3) + sensor10(4) + sensor10(5))/5);
        
        AC.setIRReady(true);
        
        fprintf('RECIEVED SENSOR DATA\n');
        AC.plotSensorData();
        
        if (dataBuffer(22) == 1)
            fprintf('ROVER HAS FINISHED MOVING: %d\n', dataBuffer(2));
            AC.roverFinish();
            AC.setDone(true);
        end
%         if (dataBuffer(22) == 2)
%             AC.roverStep();
%         end
        fwrite(otherWiFly,[bin2dec(sprintf('%s', dec2bin(254, 8))), dataBuffer]);
    end
    if (dataBuffer(1) == 51)
        if (dataBuffer(2) ~= AC.getLastMessageID())
            fprintf('\n\nMISSED MOTOR MESSAGE: %d\n\n', AC.getLastMessageID());
        else
            fprintf('RECIEVED CORRECT ECHO: %d\n', dataBuffer(2));
            AC.setConfirmed(dataBuffer(2));
            AC.setMoving();
        end
    end
    if (dataBuffer(1) == 52)
        fprintf('MOVING STATUS UPDATE: %d\n', dataBuffer(2));
        AC.roverStep();
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