function finalRad= ExampleControlProgram(serPort)
% Simple program for autonomously control the iRobot Create on either the
% physical Create or the simulated version. This will simply spiral outward
% and turn away from obstacles that detects with the bump sensors.
%
% For the physical Create only, it is assumed that the function call
% serPort= RoombaInit(comPort) was done prior to running this program.
% Calling RoombaInit is unnecessary if using the simulator.
%
% Input:
% serPort - Serial port object, used for communicating over bluetooth
%
% Output:
% finalRad - Double, final turning radius of the Create (m)

% ExampleControlProgram.m
% Copyright (C) 2011 Cornell University
% This code is released under the open-source BSD license.  A copy of this
% license should be provided with the software.  If not, email:
% CreateMatlabSim@gmail.com

    % Set constants for this program
    maxDuration= 50000;  % Max time to allow the program to run (s)
    maxDistSansBump= 5; % Max distance to travel without obstacles (m)
    maxFwdVel= 0.5;     % Max allowable forward velocity with no angular 
                        % velocity at the time (m/s)
    maxVelIncr= 0.005;  % Max incrementation of forward velocity (m/s)
    maxOdomAng= pi/4;   % Max angle to move around a circle before 
                        % increasing the turning radius (rad)
    
    
    % Initialize loop variables
    tStart= tic;        % Time limit marker
    distSansBump= 0;    % Distance traveled without hitting obstacles (m)
    angTurned= 0;       % Angle turned since turning radius increase (rad)
    maxV = 0.1;
    v= maxV;               % Forward velocity (m/s)
    regW = 0.1;
    w= regW;          % Angular velocity (rad/s)
    oldW = w;
    oldv = v;

    % Start robot moving
    SetFwdVelAngVelCreate(serPort,v,w)
    
    % Enter main loop
    objInSight = 0;
    onRoute = false;
    routeThwarted = 0;
    firstSiting = true;
    distanceToGo = 2 + ((2*rand)-1); % go between 1 and 3 meters before looking around
    while toc(tStart) < maxDuration 
        % Check for and react to bump sensor readings
        bumped= bumpCheckReact(serPort);
        
        % If obstacle was hit reset distance and angle recorders
        if bumped
            DistanceSensorRoomba(serPort);  % Reset odometry too
            AngleSensorRoomba(serPort);
            distSansBump= 0;
            angTurned= 0;
            w = regW;
            
            [camAngle camDist camColor] = CameraSensorCreate(serPort);
            if (length(camDist) > 0),
                objInSight = 1;
            else,
                objInSight = 0;
            end;
            % in new direction
            SetFwdVelAngVelCreate(serPort,v,w);
            if (onRoute),
                routeThwarted = routeThwarted + 1;
                onRoute = false;
                fprintf('Bumped: Not on route (%d)\n',routeThwarted);
            elseif (objInSight),
                routeThwarted = routeThwarted - 0.5;
                if (routeThwarted <= 0),
                    routeThwarted = 0;
                    onRoute = true;
                    fprintf('Bumped: Back on route\n');
                else,
                    fprintf('Bumped: thwarted on route, %d\n',routeThwarted);
                end;
            else,
                routeThwarted = routeThwarted - 0.25;
                fprintf('Bumped: Cannot see object, %d\n',routeThwarted);
            end;
            distanceToGo = 2 + ((2*rand)-1); % go between 1 and 3 meters before looking around
        elseif (onRoute == 0),
            w = -w;
            SetFwdVelAngVelCreate(serPort,v,w);
        end

        % check the camera
        [camAngle camDist camColor] = CameraSensorCreate(serPort);
        if (length(camDist) > 0),
            if ((abs(camAngle(1)) < 45) & (camDist(1) < 0.5)),
                fprintf('Found object\n');
                return;
            end;
            if (objInSight == 0),
                if (firstSiting),
                    firstSiting = false;
                    oldW = w;
                     oldV = v;
                     % start new route
                     w = 0;
                     v = maxV;
                     routeThwarted = 0;
                     onRoute = true;
                     SetFwdVelAngVelCreate(serPort,v,w);
                     fprintf('First Siting\n');
                end;
                objInSight = 1;
            else,
                if ((onRoute) | (routeThwarted <= 0.0)),
                  onRoute = 1;
                  if (camAngle < 0),
                        w = -0.1;
                  elseif (camAngle > 0),
                     w = 0.1;
                  end;
                  SetFwdVelAngVelCreate(serPort,v,w);
                  fprintf('Object in sight: %f %f\n',v,w);
                else
                  fprintf('Thwarted (%d):Object in sight: %f %f\n',routeThwarted,v,w);
                end;
            end;    
        else,
          % lost the object, back to the old route
          if (objInSight == 1),
              w = oldW;
              v = oldV;
              SetFwdVelAngVelCreate(serPort,v,w);
          end;
          objInSight = 0;
          onRoute = false;
          distSansBump = distSansBump + DistanceSensorRoomba(serPort);
          if ((distSansBump > distanceToGo) & (routeThwarted <= 0)),
              % turn in place for 360 or until we see the object
              fprintf('Traveled %f of %f\n',distSansBump,distanceToGo);
              scannedAng = 0;
              SetFwdVelAngVelCreate(serPort,0,0.2);
              objInSight = 0;
              while ((scannedAng < 360) & (objInSight == 0)),
                  scannedAng = scannedAng + AngleSensorRoomba(serPort);
                  [camAngle camDist camColor] = CameraSensorCreate(serPort);
                  if (length(camDist) > 0),
                      objInSight = 1;
                      onRoute = true;
                      routeThwarted = 0;
                      w = 0;
                  end;
              end;
              SetFwdVelAngVelCreate(serPort,v,w);
              distSansBump= 0;
          end;
        end;
        
        
        % Briefly pause to avoid continuous loop iteration
        pause(0.001)
    end
    fprintf('Used %f of %f time\n',toc(tStart),maxDuration);
    % Specify output parameter
    finalRad= v/w;
    
    % Stop robot motion
    v= 0;
    w= 0;
    SetFwdVelAngVelCreate(serPort,v,w)
    
    % If you call RoombaInit inside the control program, this would be a
    % good place to clean up the serial port with...
    % fclose(serPort)
    % delete(serPort)
    % clear(serPort)
    % Don't use these if you call RoombaInit prior to the control program
end

function [foundCliff leftCliff rightCliff] = isCliff(serPort)
    c1 = CliffRightSignalStrengthRoomba(serPort);
    c2 = CliffLeftSignalStrengthRoomba(serPort);
    c3 = CliffFrontRightSignalStrengthRoomba(serPort);
    c4 = CliffFrontLeftSignalStrengthRoomba(serPort);
    if (min([c2 c4]) < 10),
        leftCliff = true;
    else,
        leftCliff = false;
    end;
    if (min([c1 c3]) < 10),
        rightCliff = true;
    else,
        rightCliff = false;
    end;
    if (leftCliff | rightCliff),
        foundCliff = true;
    else,
        foundCliff = false;
    end;
end

function bumped= bumpCheckReact(serPort)
% Check bump sensors and steer the robot away from obstacles if necessary
%
% Input:
% serPort - Serial port object, used for communicating over bluetooth
%
% Output:
% bumped - Boolean, true if bump sensor is activated

    % Check bump sensors (ignore wheel drop sensors)
    [BumpRight BumpLeft WheDropRight WheDropLeft WheDropCaster ...
        BumpFront] = BumpsWheelDropsSensorsRoomba(serPort);
    bumped= BumpRight || BumpLeft || BumpFront;
    
    [foundCliff leftCliff rightCliff] = isCliff(serPort);
    if (foundCliff),
        fprintf('Cliff Here: %d %d\n',leftCliff,rightCliff);
    end;
    % Halt forward motion and turn only if bumped
    if ((bumped) | foundCliff),
        AngleSensorRoomba(serPort);     % Reset angular odometry

        if (BumpFront | (leftCliff & rightCliff)),
            ang = 0;
            while ((abs(ang) < 95) | (abs(ang) > 265)),
                ang = ((2*rand)-1)*360;
            end;
        elseif (BumpRight | rightCliff),
            ang = ((2*rand)-1)*360;  
            while ((ang < 50) | (ang > 220)),
                ang = ((2*rand)-1)*360;
            end;
        elseif (BumpLeft | leftCliff),
            ang = ((2*rand)-1)*360;
            while ((ang > -50) | (ang < -220)),
                ang = ((2*rand)-1)*360;
            end;
        end;
        % stop, choose a random direction, then turn that direction
        SetFwdVelAngVelCreate(serPort,0,0);   

        turnAngle(serPort,0.2,ang);
        fprintf('Done with turn (%f)\n',ang);
        bumped = true;
    else,
        if (ReadSonar(serPort,2) < 1.0),
            bumped = true;
            AngleSensorRoomba(serPort);
            % stop, rotate until open        
            SetFwdVelAngVelCreate(serPort,0,1);
            while (ReadSonar(serPort,2) < 1.0),
            end;
            SetFwdVelAngVelCreate(serPort,0,0);
            fprintf('Sonar Move: %f\n',AngleSensorRoomba(serPort));
        end;
            
    end
end