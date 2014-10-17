function callbackProcessData(obj, event, dataBuffer)

%global dataBuffer
persistent dataArray;
persistent figureAxis;

fprintf('calling\n');

end
% if (dataBuffer(length(dataBuffer)) == 255)
%     if (dataBuffer(2) == 10)
%         fprintf('plotting\n');
%         data = dataBuffer(3:4);
%         data2 = dataBuffer(5:6);
%         data3 = dataBuffer(7:8);
%         data4 = dataBuffer(9:10);
%         data5 = dataBuffer(11:12);
%         data6 = dataBuffer(13:14);
%         data7 = dataBuffer(15:16);
%         data8 = dataBuffer(17:18);
%         data9 = dataBuffer(19:20);
%         data10 = dataBuffer(21:22);
%         data_String = sprintf('%s', data);
%         data_String_Binary = sprintf('%s%s', dec2bin((data_String(1)), 8), dec2bin((data_String(2)), 8));
%         data_Dec = bin2dec(data_String_Binary)/100;
%         fprintf('callbackARMSimWiFly: Received binary data %s. Total of %d bytes read.\n',...
%             data_String_Binary, length(dataBuffer));
% 
%         % We convert the string back into a value. This is, of course, completely
%         % ridiculous, and your code will not do this.
%         %str = sprintf('%s',data(1:(length(data)-1)));
%         %fprintf('callbackARMSimWiFly: Modified data = %s\n',str);
%         value = data_Dec;
%         %analogVolt = (data_Dec/310);
%         %centimeters = (47.25530465*(analogVolt^2))-(193.4144014*analogVolt)+216.5086058
%         %value = centimeters/100;
%         fprintf('callbackARMSimWiFly: Received the value %f\n',value);
% 
%         % We use the value just received and include it in our dataArray. We
%         % then update our plot of data received from the SensorSim.
%         figure(h);
%         if isempty(dataArray)
%             dataArray = [value];
%             figureAxis = axis();
%             figureAxis(1) = 0;
%             figureAxis(2) = 10;
%             figureAxis(3) = 0;
%             figureAxis(4) = 10;
%             plot(1,dataArray,'+');
%             axis(figureAxis);
%             title('Data received from SensorSim');
%             xlabel('Data index');
%             ylabel('Data value (ft)');
%             % We hold the plot so that we can just add data values one at a time
%             % without the figure flashing on and off.
%             hold on;
%         else
%             if (checkIndex(dataBuffer(1)))
%                 dataArray = [dataArray value];
%             else
%                 fprintf('\n\nMISSED A SENSOR MESSAGE, SKIPPED\n\n');
%                 dataArray = [dataArray 0 value];
%             end
%             if(length(dataArray)>figureAxis(2))
%                 figureAxis(2) = figureAxis(2) + 10;
%                 axis(figureAxis);
%             end
%             plot(length(dataArray),value,'+');
%         end
%     elseif(dataBuffer(2) == 153)
%         if (dataBuffer(4) == AC.getLastMessageID())
%             fprintf('\n\nMESSAGE WAS RECIEVED CORRECTLY: %d, %d\n\n', dataBuffer(4), AC.getLastMessageID());
%         else
%             fprintf('\n\nMESSAGE WAS NOT RECIEVED CORRECTLY: %d, %d\n\n', dataBuffer(4), AC.getLastMessageID());
%             AC.resendLastMessage();
%         end
% 
%     %more message types
%     end
% 
%     clear dataBuffer;
% end
% 
% end
