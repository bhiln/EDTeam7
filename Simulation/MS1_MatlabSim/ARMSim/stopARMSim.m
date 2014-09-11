% Stop the ARMSim Timer
stop(ARMSimTimer);

% Close WiFly FID
fclose(ioARMSimWiFly);

% clear figure
clf(h);

% Clear all state
clear all;