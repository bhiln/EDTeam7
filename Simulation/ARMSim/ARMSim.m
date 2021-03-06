% ARMSim
%
% I will run this driver as a script so that you can use the 
% command window to interact with the simulation as it 
% is running. Ultimately this would probably a function 
% with some other interface (e.g., a GUI).
%
% This script does three things:
%    1. Open a matlab figure for plotting
%    2. Open the serial connection to the WiFly on the ARM side 
%    3. Start a timer that queries over WiFly (think PIC on ARM side)
%

% 1. Open up a matlab plot window and get its handle
% We will pass the handle to the callback for the WiFly
% so that we can update the figure when we receive new sensor data
%figure(1);
%h = gcf();
h = 1;

%dataBuffer = [];

% 2. Connect to WiFly and setup serial callback
% You will have to change this to agree with your WiFly
% Note that I have set up the WiFly to have a baud rate of 57600
ioARMWiFly = serial('COM6','BaudRate',57600);
ioRovWiFly = serial('COM5','BaudRate',57600);
AC = ARMController(ioRovWiFly);
%ntimes = commandGUI(ioARMSimWiFly);

% Note that we will pass the figure handle to the timer callback
% We will use this handle when we update our data plot
ioARMWiFly.BytesAvailableFcnCount = 1;
ioARMWiFly.BytesAvailableFcnMode = 'byte';
ioARMWiFly.BytesAvailableFcn = {@callbackARMSimWiFly,h,AC,ioRovWiFly};

ioRovWiFly.BytesAvailableFcnCount = 1;
ioRovWiFly.BytesAvailableFcnMode = 'byte';
ioRovWiFly.BytesAvailableFcn = {@callbackRovSimWiFly,h,AC,ioARMWiFly};

fopen(ioARMWiFly);
fopen(ioRovWiFly);

% ioARMSimWiFly.ReadAsyncMode = 'manual';

% 3. Create and start the ARMSimTimer object
% Note that we pass the WiFly serial object to the timer so
% that we can write to the WiFly from the timer callback
ARMSimTimer = timer;
ARMSimTimer.Period         = 1;
ARMSimTimer.ExecutionMode  = 'fixedRate';
ARMSimTimer.TimerFcn       = {@callbackARMSimTimer,ioRovWiFly};
ARMSimTimer.BusyMode       = 'drop';

%start(ARMSimTimer);

% last_ba = 0;
% while (1)
%     bytesAvailable = ioARMSimWiFly.BytesAvailable;
%     if (bytesAvailable > last_ba)
%         [data, count, msg] = fread(ioARMSimWiFly, 1);
%         fprintf('data length %s', data);
%     end
%     last_ba = bytesAvailable;
%     
% end
