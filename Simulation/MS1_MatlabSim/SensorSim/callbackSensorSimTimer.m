function callbackSensorSimTimer( timerSim, event, obj )
% SensorSim Timer callback
% Input:
% timerSim - Timer object (required argument for timers)
% event - Structure, contains information about timer call
%   (required argument for timers)
% obj - SensorSim object, contains information about Sensor

% Advance time on the Sensor (just because we can)
currentTime = obj.updateCurrentTime(timerSim.Period);

% Do an A/D Reading (as if we were the timer on the Sensor PIC)
obj.doADReading();

% Print something out so that we know the timer is running
fprintf('SensorSim Timer callback: AD updated, current time = %f\n',...
    currentTime);

end

