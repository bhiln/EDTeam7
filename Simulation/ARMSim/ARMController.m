classdef ARMController < handle
    
    methods(Static = true)
        
        function [ toReturn ] = getLastMessageID()
            global ntimes;
            toReturn = ntimes;
        end
        
        function resendLastMessage()
            global lastMessage;
            global ioWiFly;
            
            fwrite(ioWiFly,lastMessage);
        end
            
        
        function [ listeningFor ] = sendMotorForward5()
            global ntimes;
            global ioWiFly;
            global lastMessage;
            
            ntimes = mod((ntimes+1),255);
            
            ntimes_Char = bin2dec(sprintf('%s', dec2bin(ntimes, 8)));

            % Add the silly terminator and send the string over WiFly
            str = sprintf('%s%s%s%s%s%s',ntimes_Char, bin2dec(sprintf('%s', dec2bin(50, 8))), bin2dec(sprintf('%s', dec2bin(49, 8))), bin2dec(sprintf('%s', dec2bin(255, 8))), bin2dec(sprintf('%s', dec2bin(0, 8))));
            fprintf('ARMSim timer callback: Sending %s over WiFly\n',str);
            fwrite(ioWiFly,str);
            
            lastMessage = str;

        end
    end
    
    methods
        
        function obj = ARMController(ioW)
            global ioWiFly;
            global ntimes;
            ioWiFly = ioW;
            ntimes = 0;
        end
    end
end