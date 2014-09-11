function callbackARMSimWiFly( obj, event, h )
% Callback for WiFly serial object on ARMSim side
% One would not actual do the communication in this way (i.e., via
% string with the terminator 'o'.). This is just to show how to
% Set up the callbacks and to communicate between the two programs.

% Keep around a persistent array that we add data to as we receive it.
% We also keep around the current plot axis data.
persistent dataArray;
persistent figureAxis;

% We are in the callback because we received some data. We calculate
% how much and then read it into a data array.
bytesAvailable = obj.BytesAvailable;
[data count msg] = fread(obj, bytesAvailable, 'char');
valuesReceived = obj.ValuesReceived;
fprintf('callbackARMSimWiFly: Received data %s, %d bytes. Total of %d bytes read.\n',...
    data, bytesAvailable, valuesReceived);

% We convert the string back into a value. This is, of course, completely
% ridiculous, and your code will not do this.
str = sprintf('%s',data(1:(length(data)-1)));
fprintf('callbackARMSimWiFly: Modified data = %s\n',str);
value = str2double(str);
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

end

