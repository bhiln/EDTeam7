function callbackARMSimWiFly( obj, event, h )
% Callback for WiFly serial object on ARMSim side
% One would not actual do the communication in this way (i.e., via
% string with the terminator 'o'.). This is just to show how to
% Set up the callbacks and to communicate between the two programs.

% Keep around a persistent array that we add data to as we receive it.
% We also keep around the current plot axis data.
persistent dataBuffer;
persistent dataArray;
persistent figureAxis;
persistent lastIndex;

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
    if (isempty(lastIndex))
        lastIndex = dataBuffer(1)-1;
    end
    if (lastIndex == dataBuffer(1)-1)
        lastIndex = dataBuffer(1);
    else
        fprintf('Lost packet\n');
        return;
    end
    data = dataBuffer(3:4);
    data_String = sprintf('%s', data);
    data_String_Binary = sprintf('%s%s', dec2bin((data_String(1)+0), 8), dec2bin((data_String(2)+0), 8));
    data_Dec = bin2dec(data_String_Binary)/100;
    fprintf('callbackARMSimWiFly: Received data %s, binary data %s, dec %d, %d bytes. Total of %d bytes read.\n',...
        dataBuffer, data_String_Binary, data_Dec, bytesAvailable, length(dataBuffer));

    % We convert the string back into a value. This is, of course, completely
    % ridiculous, and your code will not do this.
    %str = sprintf('%s',data(1:(length(data)-1)));
    %fprintf('callbackARMSimWiFly: Modified data = %s\n',str);
    value = data_Dec;
    %analogVolt = (data_Dec/310);
    %centimeters = (47.25530465*(analogVolt^2))-(193.4144014*analogVolt)+216.5086058
    %value = centimeters/100;
    fprintf('callbackARMSimWiFly: Received the value %f\n',value);

    % We use the value just received and include it in our dataArray. We
    % then update our plot of data received from the SensorSim.
    figure(h);
    if isempty(dataArray)
        dataArray = [value];
        figureAxis = axis();
        figureAxis(1) = 0;
        figureAxis(2) = 10;
        figureAxis(3) = 0;
        figureAxis(4) = 10;
        plot(1,dataArray,'+');
        axis(figureAxis);
        title('Data received from SensorSim');
        xlabel('Data index');
        ylabel('Data value (ft)');
        % We hold the plot so that we can just add data values one at a time
        % without the figure flashing on and off.
        hold on;
    else
        dataArray = [dataArray value];
        if(length(dataArray)>figureAxis(2))
            figureAxis(2) = figureAxis(2) + 10;
            axis(figureAxis);
        end
        plot(length(dataArray),value,'+');
    end
    
    dataBuffer = [];
end

end

