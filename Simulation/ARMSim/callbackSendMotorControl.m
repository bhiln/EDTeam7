function callbackSendMotorControl( ioWiFly, ntimes_Char, command )
%CALLBACKSENDMOTORCONTROL Summary of this function goes here
%   Detailed explanation goes here

    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(ntimes_Char, 8))));
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(50, 8))));
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(command(1), 8))));
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(command(2), 8))));
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(255, 8))));

end

