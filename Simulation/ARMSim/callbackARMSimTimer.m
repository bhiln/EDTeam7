function callbackARMSimTimer( obj, event, ioWiFly )
% ARMSIM timer callback
% Input:
% obj - Timer object (required argument for timers)
% event - Structure, contains information about timer call
%   (required argument for timers)
% ioWiFly - ARM WiFly FID - We need to send data over the 
% WiFly to the SensorSim from this timer

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

% Add the silly terminator and send the string over WiFly
str = sprintf('%s%s%s%s%s',bin2dec(sprintf('%s', dec2bin(0, 8))), bin2dec(sprintf('%s', dec2bin(49, 8))), bin2dec(sprintf('%s', dec2bin(50, 16))), bin2dec(sprintf('%s', dec2bin(127, 16))), bin2dec(sprintf('%s', dec2bin(0, 16))));
fprintf('ARMSim timer callback: Sending %s over WiFly\n',str);
fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(ntimes_Char, 8))));
fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(50, 8))));
fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(23, 8))));
fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(49, 8))));
fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(255, 8))));
end