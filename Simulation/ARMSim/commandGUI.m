function commandGUI( ioWiFly )
%COMMANDGUI Summary of this function goes here
%   Detailed explanation goes here


%rectangle('Curvature',[x,y])
%rectangle('PropertyName',propertyvalue,...)
x = 1;
ntimes = 0;

f = figure('Visible','on','Position',[360,500,450,285]);
%h = rectangle('Position',[x,50,20,40])

% Construct the components.
forward = uicontrol('Style','pushbutton','String','Forward','Position',[200,220,70,25],'Callback',{@forwardbutton_Callback});
forwardstep = uicontrol('Style','pushbutton','String','Forward Step','Position',[200,250,70,25],'Callback',{@forwardstepbutton_Callback});
backwards = uicontrol('Style','pushbutton','String','Backwards','Position',[200,150,70,25],'Callback',{@backwardsbutton_Callback});
backwardsstep = uicontrol('Style','pushbutton','String','Backwards Step','Position',[200,120,70,25],'Callback',{@backwardsstepbutton_Callback});
left = uicontrol('Style','pushbutton','String','Left','Position',[120,185,70,25],'Callback',{@left90button_Callback});
left15 = uicontrol('Style','pushbutton','String','Left15','Position',[120,150,70,25],'Callback',{@left15button_Callback});
right = uicontrol('Style','pushbutton','String','Right','Position',[280,185,70,25],'Callback',{@right90button_Callback});
right15 = uicontrol('Style','pushbutton','String','Right15','Position',[280,150,70,25],'Callback',{@right15button_Callback});
stop = uicontrol('Style','pushbutton','String','Stop','Position',[200,185,70,25],'Callback',{@stopbutton_Callback});

speedSlider = uicontrol('Style','slider','Max',63,'Min',0,'Position',[300,50,100,25]);
speedSliderListener = handle.listener(speedSlider,'ActionEvent',@speedSliderCallback);
getSpeed = @() get(speedSlider, 'Value');
speedText = uicontrol('Style','text','String','Speed:','Position',[300,90,40,15]);
speedValueText = uicontrol('Style','text','String',getSpeed(),'Position',[340,90,60,15]);

distanceEdit = uicontrol('Style','edit','Position',[210,50,50,25]);
getDistance = @() str2num(get(distanceEdit, 'String'));
distanceText = uicontrol('Style','text','String','Distance','Position',[210,90,50,15]);
  
    function speedSliderCallback(~,~)
        delete(speedValueText);
        speedValueText = uicontrol('Style','text','String',getSpeed(),'Position',[340,90,60,15]);
    end

    function forwardstepbutton_Callback(source,eventdata) 
    % Display surf plot of the currently selected data.
        sendForward();
        pause(0.1);
        sendStop();
    end

    function forwardbutton_Callback(source,eventdata) 
    % Display surf plot of the currently selected data.
        sendForward();
    end

    function backwardsstepbutton_Callback(source,eventdata) 
    % Display surf plot of the currently selected data.
        sendBackwards();
        pause(0.1);
        sendStop();
    end

    function backwardsbutton_Callback(source,eventdata) 
    % Display surf plot of the currently selected data.
        sendBackwards();
    end

    function stopbutton_Callback(source,eventdata) 
    % Display surf plot of the currently selected data.
        sendStop();
    end

    function left90button_Callback(source,eventdata) 
    % Display surf plot of the currently selected data.
        sendLeft();
        pause(0.81);
        sendStop();
    end

    function left15button_Callback(source,eventdata) 
    % Display surf plot of the currently selected data.
        sendLeft();
        pause(0.2);
        sendStop();
    end

    function right90button_Callback(source,eventdata) 
    % Display surf plot of the currently selected data.
        sendRight();
        pause(0.81);
        sendStop()
    end

    function right15button_Callback(source,eventdata) 
    % Display surf plot of the currently selected data.
        sendRight();
        pause(0.2);
        sendStop();
    end

    function sendForward()
        ntimes_Char = bin2dec(sprintf('%s', dec2bin(ntimes, 8)));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(ntimes_Char, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(50, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(10, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getDistance(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getSpeed(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(255, 8))));

        ntimes = ntimes + 1;
    end

    function sendBackwards()
        ntimes_Char = bin2dec(sprintf('%s', dec2bin(ntimes, 8)));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(ntimes_Char, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(50, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(14, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getDistance(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getSpeed(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(255, 8))));

        ntimes = ntimes + 1;
    end

    function sendLeft()
        ntimes_Char = bin2dec(sprintf('%s', dec2bin(ntimes, 8)));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(ntimes_Char, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(50, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(11, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getDistance(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getSpeed(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(255, 8))));

        ntimes = ntimes + 1;
    end

    function sendRight()
        ntimes_Char = bin2dec(sprintf('%s', dec2bin(ntimes, 8)));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(ntimes_Char, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(50, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(12, 8))));
       	fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getDistance(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getSpeed(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(255, 8))));

        ntimes = ntimes + 1;
    end

    function sendStop()
        ntimes_Char = bin2dec(sprintf('%s', dec2bin(ntimes, 8)));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(ntimes_Char, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(50, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(13, 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getDistance(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(getSpeed(), 8))));
        fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(255, 8))));

        ntimes = ntimes + 1;
    end

end

