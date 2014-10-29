function commandGUI( ioWiFly )
%COMMANDGUI Summary of this function goes here
%   Detailed explanation goes here


%rectangle('Curvature',[x,y])
%rectangle('PropertyName',propertyvalue,...)
x = 1;

f = figure('Visible','on','Position',[360,500,450,285]);
%h = rectangle('Position',[x,50,20,40])

% Construct the components.
forward    = uicontrol('Style','pushbutton',...
             'String','Forward','Position',[200,220,70,25],'Callback',{@forwardbutton_Callback});
forwardstep    = uicontrol('Style','pushbutton',...
             'String','Forward Step','Position',[200,250,70,25],'Callback',{@forwardstepbutton_Callback});
backwards    = uicontrol('Style','pushbutton',...
             'String','Backwards','Position',[200,150,70,25],'Callback',{@backwardsbutton_Callback});
backwardsstep    = uicontrol('Style','pushbutton',...
             'String','Backwards Step','Position',[200,120,70,25],'Callback',{@backwardsstepbutton_Callback});
left90 = uicontrol('Style','pushbutton',...
             'String','Left90','Position',[120,185,70,25],'Callback',{@left90button_Callback});
left15 = uicontrol('Style','pushbutton',...
             'String','Left15','Position',[120,150,70,25],'Callback',{@left15button_Callback});
right90  = uicontrol('Style','pushbutton',...
            'String','Right90','Position',[280,185,70,25],'Callback',{@right90button_Callback});
right15 = uicontrol('Style','pushbutton',...
             'String','Right15','Position',[280,150,70,25],'Callback',{@right15button_Callback});
stop  = uicontrol('Style','pushbutton',...
            'String','Stop','Position',[200,185,70,25],'Callback',{@stopbutton_Callback});
  
function forwardstepbutton_Callback(source,eventdata) 
% Display surf plot of the currently selected data.
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(10, 8))));
    pause(0.1);
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(13, 8))));
    x = x+1;
    h = rectangle('Position',[x,50,20,40])
    %refresh(f);
end

function forwardbutton_Callback(source,eventdata) 
% Display surf plot of the currently selected data.
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(10, 8))));
end

function backwardsstepbutton_Callback(source,eventdata) 
% Display surf plot of the currently selected data.
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(14, 8))));
    pause(0.1);
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(13, 8))));
end

function backwardsbutton_Callback(source,eventdata) 
% Display surf plot of the currently selected data.
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(14, 8))));
end

function stopbutton_Callback(source,eventdata) 
% Display surf plot of the currently selected data.
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(13, 8))));
end

function left90button_Callback(source,eventdata) 
% Display surf plot of the currently selected data.
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(11, 8))));
    pause(0.81);
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(13, 8))));
end

function left15button_Callback(source,eventdata) 
% Display surf plot of the currently selected data.
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(11, 8))));
    pause(0.2);
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(13, 8))));
end

function right90button_Callback(source,eventdata) 
% Display surf plot of the currently selected data.
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(12, 8))));
    pause(0.81);
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(13, 8))));
end

function right15button_Callback(source,eventdata) 
% Display surf plot of the currently selected data.
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(12, 8))));
    pause(0.2);
    fwrite(ioWiFly,bin2dec(sprintf('%s', dec2bin(13, 8))));
end

end

