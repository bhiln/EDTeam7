function callbackARMSimWiFly( obj, event, h, AC, otherWiFly )
% Callback for WiFly serial object on ARMSim side
% One would not actual do the communication in this way (i.e., via
% string with the terminator 'o'.). This is just to show how to
% Set up the callbacks and to communicate between the two programs.

% Keep around a persistent array that we add data to as we receive it.
% We also keep around the current plot axis data.
%global dataBuffer;

% We are in the callback because we received some data. We calculate
% how much and then read it into a data array.
bytesAvailable = obj.BytesAvailable;
[recievedByte, ~, ~] = fread(obj, 1, 'char');
fwrite(otherWiFly,recievedByte);
end