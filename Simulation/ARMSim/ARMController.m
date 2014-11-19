classdef ARMController < handle
    
    methods(Static = true)
        
        function [ toReturn ] = getLastMessageID()
            global ntimes;
            toReturn = ntimes;
        end
        
        function [ toReturn ] = setConfirmed(ta)
            global confirmed;
            confirmed = ta;
            fprintf('CONFIRMED\n');
            toReturn = confirmed;
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
        
        
        
        function obj = ARMController(ioW)
            global ioWiFly;
            global ntimes;
            global confirmed;
            global x;
            global y;
            global alpha
            global XY;
            global R;
            global stop;
            ioWiFly = ioW;
            ntimes = 0;
            confirmed = -99;
            x = [-10,-8,-8,-10,-10];
            y = [-2,-2,0,0,-2];
            alpha = 0;
            stop = false;
            
            f = figure(1);
            set(f,'Name','Rover Motor Controller','Position',[360,500,450,285],'numbertitle','Off');

            % Construct the components.
            forward = uicontrol('Style','pushbutton','String','Forward','Position',[200,220,70,25],'Callback',{@forwardbutton_Callback});
            %forwardstep = uicontrol('Style','pushbutton','String','Forward Step','Position',[200,250,70,25],'Callback',{@forwardstepbutton_Callback});
            backwards = uicontrol('Style','pushbutton','String','Backwards','Position',[200,150,70,25],'Callback',{@backwardsbutton_Callback});
            %backwardsstep = uicontrol('Style','pushbutton','String','Backwards Step','Position',[200,120,70,25],'Callback',{@backwardsstepbutton_Callback});
            left = uicontrol('Style','pushbutton','String','Left','Position',[120,185,70,25],'Callback',{@leftbutton_Callback});
            leftSquare = uicontrol('Style','pushbutton','String','Left Square','Position',[120,150,70,25],'Callback',{@leftSbutton_Callback});
            right = uicontrol('Style','pushbutton','String','Right','Position',[280,185,70,25],'Callback',{@rightbutton_Callback});
            %right15 = uicontrol('Style','pushbutton','String','Right15','Position',[280,150,70,25],'Callback',{@right15button_Callback});
            stop = uicontrol('Style','pushbutton','String','Stop','Position',[200,185,70,25],'Callback',{@stopbutton_Callback});

            speedSlider = uicontrol('Style','slider','Max',63,'Min',0,'SliderStep',[1/64,10/64],'Value',35,'Position',[300,50,100,25]);
            speedSliderListener = addlistener(speedSlider,'ContinuousValueChange',@speedSliderCallback);
            getSpeed = @() round(get(speedSlider, 'Value'));
            speedText = uicontrol('Style','text','String','Speed:','Position',[300,90,40,15]);
            speedValueText = uicontrol('Style','text','String',(ceil((getSpeed()/16)*10)/10),'Position',[340,90,60,15]);

            distanceEdit = uicontrol('Style','edit','String','5','Position',[210,50,50,25]);
            getDistance = @() round(str2num(get(distanceEdit, 'String')));
            distanceText = uicontrol('Style','text','String','Distance','Position',[210,90,50,15]);
            
            angleSlider = uicontrol('Style','slider','Max',180,'Min',0,'SliderStep',[1/180,10/180],'Value',45,'Position',[80,50,100,25]);
            angleSliderListener = addlistener(angleSlider,'ContinuousValueChange',@angleSliderCallback);
            getAngle = @() round(get(angleSlider, 'Value'));
            angleText = uicontrol('Style','text','String','Turn Angle:','Position',[80,90,75,15]);
            angleValueText = uicontrol('Style','text','String',getAngle()*2,'Position',[155,90,22,15]);
            
            % Filename
            filename = 'Map1.txt';

            % Read the Map
            [XY, Ramp_Center, Ramp_Entrance, Ramp_Exit, Target] = Read_Map_File(filename);

            % Use figure 1
            h = figure(2);
            clf(h);

            % Plot the Map outline
            Plot_Map(h, XY);

            % Plot the Ramps on the Map
            Plot_Ramps(h, Ramp_Center, Ramp_Entrance, Ramp_Exit);

            % Show the Target
            Plot_Target(h, Target);

            R(1,:)=x;R(2,:)=y;
            alpha=0;
            XY=[cos(alpha) -sin(alpha);sin(alpha) cos(alpha)]*R;
            hold on;
            plot(XY(1,:),XY(2,:),'r');
            hold off;
%             r = rectangle('Position',[1,1,2,2]);

%             rotate2d(r, pi);
%             hand = plot(xx,yy,'r.'); 
%             hold on 
%             rectangle('Position',[1,1,2,2]);
%             plot(verticies);
%             axis([0 100 0 100])
%             r = rectangle()
            
                function speedSliderCallback(~,~)
                    figure(1);
                    delete(speedValueText);
                    speedValueText = uicontrol('Style','text','String',(ceil((getSpeed()/16)*10)/10),'Position',[340,90,60,15]);
                end
                
                function angleSliderCallback(~,~)
                    figure(1);
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

                function leftSbutton_Callback(source,eventdata)
                    sendSquare();
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
                    %xpos = xpos + 1;
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
                    stop = true;
                end
                
                function sendSquare()
                    stop = false;
                    while (stop == false)
                        sendMotorCommand(10)
                        pause(4-(ceil((getSpeed()/16)*10)/10));
                        sendMotorCommand(11)
                        pause(4-(ceil((getSpeed()/16)*10)/10));
                        sendMotorCommand(10)
                        pause(4-(ceil((getSpeed()/16)*10)/10));
                        sendMotorCommand(11)
                        pause(4-(ceil((getSpeed()/16)*10)/10));
                        sendMotorCommand(10)
                        pause(4-(ceil((getSpeed()/16)*10)/10));
                        sendMotorCommand(11)
                        pause(4-(ceil((getSpeed()/16)*10)/10));
                        sendMotorCommand(10)
                        pause(4-(ceil((getSpeed()/16)*10)/10));
                        sendMotorCommand(11)
                        pause(4-(ceil((getSpeed()/16)*10)/10));
                    end
                end
                
                function sendMotorCommand(command)

                    
                    
                    while (1)
                        if (confirmed == ntimes)
                            ntimes = mod(ntimes + 1, 254);
                            figure(2);
                            hold on;
                            plot(XY(1,:),XY(2,:),'c');
                            hold off;
                            if (command == 10)
                                x = x+((getDistance()/25)*cos(alpha));
                                y = y+((getDistance()/25)*sin(alpha));
                            end
                            if (command == 14)
                                x = x-((getDistance()/25)*cos(alpha));
                                y = y-((getDistance()/25)*sin(alpha));
                            end
                            R(1,:)=x-(x(1)+1);R(2,:)=y-(y(1)+1);
                            if (command == 11)
                                alpha= alpha+(getAngle()*4*pi/360);
                                fprintf('left: %d\n',alpha);
                            end
                            if (command == 12)
                                alpha= alpha-(getAngle()*4*pi/360);
                                fprintf('right: %d\n',alpha);
                            end
                            XY=[cos(alpha) -sin(alpha);sin(alpha) cos(alpha)]*R;
                            XY(1,:) = XY(1,:)+(x(1)-1);
                            XY(2,:) = XY(2,:)+(y(1)-1);
                           
                            hold on;
                            plot(XY(1,:),XY(2,:),'r');
                            hold off;
                            break;
                        end
                        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(254, 8))));
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
                        
                        pause(0.2);
                        
                    end
                end
        end
    end
end