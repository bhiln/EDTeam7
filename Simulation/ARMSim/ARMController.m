classdef ARMController < handle
    
    methods(Static = true)
        
        function [ toReturn ] = getLastMessageID()
            global ntimes;
            toReturn = ntimes-1;
        end
        
        function resendLastMessage()
            global lastMessage;
            global ioWiFly;
            
            fwrite(ioWiFly,lastMessage);
        end
        
        function tr = test()
            tr = true;
        end
        
    end
    
    methods
        
        function toReturn = setConfirmed( c )
            global confirmed;
            confirmed = [confirmed, c];
            fprintf('CONFIRMED\n');
            toReturn = true;
        end
        
        function obj = ARMController(ioW)
            global ioWiFly;
            global ntimes;
            ioWiFly = ioW;
            ntimes = 0;
            
            f = figure('Name','Rover Motor Controller','Visible','on','Position',[360,500,450,285]);
            %h = rectangle('Position',[x,50,20,40])

            % Construct the components.
            forward = uicontrol('Style','pushbutton','String','Forward','Position',[200,220,70,25],'Callback',{@forwardbutton_Callback});
            %forwardstep = uicontrol('Style','pushbutton','String','Forward Step','Position',[200,250,70,25],'Callback',{@forwardstepbutton_Callback});
            backwards = uicontrol('Style','pushbutton','String','Backwards','Position',[200,150,70,25],'Callback',{@backwardsbutton_Callback});
            %backwardsstep = uicontrol('Style','pushbutton','String','Backwards Step','Position',[200,120,70,25],'Callback',{@backwardsstepbutton_Callback});
            left = uicontrol('Style','pushbutton','String','Left','Position',[120,185,70,25],'Callback',{@leftbutton_Callback});
            %left15 = uicontrol('Style','pushbutton','String','Left15','Position',[120,150,70,25],'Callback',{@left15button_Callback});
            right = uicontrol('Style','pushbutton','String','Right','Position',[280,185,70,25],'Callback',{@rightbutton_Callback});
            %right15 = uicontrol('Style','pushbutton','String','Right15','Position',[280,150,70,25],'Callback',{@right15button_Callback});
            stop = uicontrol('Style','pushbutton','String','Stop','Position',[200,185,70,25],'Callback',{@stopbutton_Callback});

            speedSlider = uicontrol('Style','slider','Max',63,'Min',0,'SliderStep',[1/64,10/64],'Value',35,'Position',[300,50,100,25]);
            try % R2013b and older
                speedSliderListener = addlistener(speedSlider,'ActionEvent',@speedSliderCallback);
            catch % R2014a and newer
                speedSliderListener = addlistener(speedSlider,'ContinuousValueChange',@speedSliderCallback);
            end
            getSpeed = @() round(get(speedSlider, 'Value'));
            speedText = uicontrol('Style','text','String','Speed:','Position',[300,90,40,15]);
            speedValueText = uicontrol('Style','text','String',getSpeed()/16,'Position',[340,90,60,15]);

            distanceEdit = uicontrol('Style','edit','String','5','Position',[210,50,50,25]);
            getDistance = @() round(str2num(get(distanceEdit, 'String')));
            distanceText = uicontrol('Style','text','String','Distance','Position',[210,90,50,15]);
            
            angleSlider = uicontrol('Style','slider','Max',180,'Min',0,'SliderStep',[1/180,10/180],'Value',45,'Position',[80,50,100,25]);
            try % R2013b and older
                angleSliderListener = addlistener(angleSlider,'ActionEvent',@angleSliderCallback);
            catch % R2014a and newer
                angleSliderListener = addlistener(angleSlider,'ContinuousValueChange',@angleSliderCallback);
            end
            getAngle = @() round(get(angleSlider, 'Value'));
            angleText = uicontrol('Style','text','String','Turn Angle:','Position',[80,90,75,15]);
            angleValueText = uicontrol('Style','text','String',getAngle()*2,'Position',[155,90,22,15]);

                function speedSliderCallback(~,~)
                    delete(speedValueText);
                    speedValueText = uicontrol('Style','text','String',getSpeed()/16,'Position',[340,90,60,15]);
                end
                
                function angleSliderCallback(~,~)
                    delete(angleValueText);
                    angleValueText = uicontrol('Style','text','String',getAngle()*2,'Position',[155,90,22,15]);
                end

%                 function forwardstepbutton_Callback(source,eventdata) 
%                 % Display surf plot of the currently selected data.
%                     %indexToSend = ntimes
%                     %global confirmed;
%                     %sendForward(indexToSend);
%                     sendForward();
%                     pause(0.1);
%                     %while (ismember(confirmed, indexToSend))
%                     %    sendForward(indexToSend);
%                     %    pause(0.1);
%                     %end
%                     %confirmed = confirmed(confirmed~=indexToSend);
%                     sendStop();
%                 end

                function forwardbutton_Callback(source,eventdata) 
                    sendForward();
                end

%                 function backwardsstepbutton_Callback(source,eventdata) 
%                     sendBackwards();
%                     pause(0.1);
%                     sendStop();
%                 end

                function backwardsbutton_Callback(source,eventdata) 
                    sendBackwards();
                end

                function stopbutton_Callback(source,eventdata) 
                    sendStop();
                end

                function leftbutton_Callback(source,eventdata) 
                    sendLeft();
%                     pause(0.81);
%                     sendStop();
                end

%                 function left15button_Callback(source,eventdata) 
%                     sendLeft();
%                     pause(0.2);
%                     sendStop();
%                 end

                function rightbutton_Callback(source,eventdata) 
                    sendRight();
%                     pause(0.81);
%                     sendStop()
                end

%                 function right15button_Callback(source,eventdata) 
%                     sendRight();
%                     pause(0.2);
%                     sendStop();
%                 end

                function sendForward()
                    sendMotorCommand(10)
                end

                function sendBackwards()
                    sendMotorCommand(14)
                end

                function sendLeft()
                    sendMotorCommand(11)
                end

                function sendRight()
                    sendMotorCommand(12)
                end

                function sendStop()
                    sendMotorCommand(13)
                end
                
                function sendMotorCommand(command)
                    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(ntimes, 8))));
                    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(50, 8))));
                    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(command, 8))));
                    
                    if ((command == 11) || (command == 12))
                        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getAngle(), 8))));
                    else
                        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getDistance(), 8))));
                    end
                    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getSpeed(), 8))));
                    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(255, 8))));
                    
                    ntimes = ntimes + 1;
                end
        end
    end
end