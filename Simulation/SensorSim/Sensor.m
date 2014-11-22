classdef Sensor < handle
    % An instance of this class is my sensor model
    % We implement this as a subclass of the handle class so that
    % we do not have to return the obj in the property setters
    % Note that the timer obj is external to this object
    % Also, the serial object (that talks to the WiFly) is external
    % These are all defined in the "SensorSim" script
    
    properties(Constant, GetAccess= 'private')
        % Values cannot be changed
        % Units are feet and seconds
        maxRange = 10.0;    % Maximum distance the sensor can see
        minRange = 1.0;     % Minimum distance the sensor can see
        timerPeriod = 5.0;  % How often we read the sensor
    end
    
    properties(GetAccess= 'private', SetAccess= 'private')
        % Values require get and set methods to view or manipulate
        % Sensor variables
        distance;   % Actual distance to wall
        spread;     % Variance of sensor
        currentTime;% Current time
        lastADReading % Most recent A/D reading
    end
    
    methods(Access= 'public')
        % Constructor Function
        function obj = Sensor(dist)
            % obj = SensorSim
            obj.distance = dist; % initial distance to wall
            obj.spread = .5;
            obj.currentTime = 0.0;
        end
        
        % Functions available to call from controller function or other files
        function dist = checkRanges(obj,inDist)
            % Check range and reset if outside of (max,min)
            % dist = obj.checkRanges(inDist)
            if(inDist > obj.maxRange)
                dist = obj.maxRange;
            elseif(inDist < obj.minRange)
                dist = obj.minRange;
            else
                dist = inDist;
            end
        end
        
        function dt = getTimerPeriod(obj)
            % dt = obj.getTimerPeriod()
            % fprintf('timer period = %f\n',obj.timerPeriod);
            dt = obj.timerPeriod;
        end
 
        function currentTime = updateCurrentTime(obj,dt)
            % time = obj.updateCurrentTime(dt)
            % fprintf('current time = %d, update = %d\n',obj.currentTime,dt);
            obj.currentTime = obj.currentTime + dt;
            currentTime = obj.currentTime;
        end

        function dist = getCurrentDistance(obj)
            % obj.getCurrentDistance()
            % fprintf('current distance = %f\n',obj.distance);
            dist = obj.distance;
        end

        function setCurrentDistance(obj,dist)
            % obj.setCurrentDistance(dist)
            fprintf('current distance = %f, new value = %f\n',...
                obj.distance, dist);
            obj.distance = dist;
        end
        
        function doADReading(obj)
            % obj.doADReading()
            % pretend that the PIC does an A/D reading and
            % saves if for any queries over WiFLy
            rawdist = obj.distance + obj.spread*(randn-0.5);
            obj.lastADReading = obj.checkRanges(rawdist);
            fprintf('Sensor: AD reading updated = %f\n',obj.lastADReading);
        end
        
        function dist = getSensorReading(obj)
            % dist = obj.getSensorReading()
            % returns the most recent AD reading
            dist = obj.lastADReading;
        end
        
    end
    
end

