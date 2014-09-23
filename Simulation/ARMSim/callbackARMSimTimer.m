function callbackARMSimTimer( obj, event, ioWiFly )
% ARMSIM timer callback
% Input:
% obj - Timer object (required argument for timers)
% event - Structure, contains information about timer call
%   (required argument for timers)
% ioWiFly - ARM WiFly FID - We need to send data over the 
% WiFly to the SensorSim from this timer

% We keep around the number of times this timer has been called
persistent ntimes;

% The first time this is called, ntimes does not exist, otherwise
% we increment it.
if isempty(ntimes) 
    ntimes = 1;
else
    ntimes = ntimes + 1;
end

% Add the silly terminator and send the string over WiFly
str = sprintf('%do',ntimes);
fprintf('ARMSim timer callback: Sending %s over WiFly\n',str);
fwrite(ioWiFly,str);

end

