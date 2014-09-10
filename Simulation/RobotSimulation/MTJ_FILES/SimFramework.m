

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


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% MTJ: The above is included with the code.  Below is where you should
% start looking.
%
% As explained below, there are portions of the code, particularly its
%   structure that should not be changed if you hope to easily translate
%   what you implement here to something useful on the ARM.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    % You may change this.  It sets the maximum simulation time in seconds
    maxSimTime = 5000;
    
    % Do not change the following.
    initRobotControl();
    startTime = tic;
    elapsedTime = 0;
    fprintf('\n\nBegin Program\n');
    % end of do not change
    
    % These are "states" for a state machine that is for a very simple
    %   algorithm for trying to find the goal.  It is not a good algorithm.
    % You would likely add to, change, delete some/all of these states.
    IDLE_STATE = 0;
    MOVING_STATE = 1;
    BUMPED_TURNING = 2;
    RAND_TURNING = 3;
    BUMPED_IDLE = 4;
    BUMPED_STOPPED = 5;
    WAIT_TO_TURN = 6;
    WAIT_TO_FORWARD = 7;
    WAIT_TO_TARGET = 8;
    TURN_TO_TARGET = 9;
    WAIT_TO_TARGET_RUN = 10;
    TARGET_RUN = 11;
    robotState = IDLE_STATE;
    
    % We are going to assume that we know where we start in this simulation
    % We will not use these precise position routines  in the code to do
    %   anything because they aren't actually possible in our environment (i.e., we
    %   do not have a sensor to accurately create these values.
    % robotPosition: X,Y in meters;
    % robotOrientation: orientation in degrees, where 0 is the positive X axis
    [robotPosition(1) robotPosition(2) robotOrientation] = OverheadLocalizationCreate(serPort);
    robotOrientation = rad2deg(robotOrientation);
    
    % This sets the average forward distance moved
    forwardDistance = 2.0;
    
    % Speeds used for turning and forward
    turnSpeed = 0.2;
    forwardSpeed = 0.25;
    
    % Time in "waiting" states
    waitInStateTime = 1.0;
   
    % Used just to keep track of how often the robot position is printed
    robotPrintTime = 1.0; % print once per second, this can be changed
    robotPrintNextTime = 0.0;
 
    % If making use of the simulated compass sensor, then this is the max
    %   error on that compass
    global maxCompassErr;
    maxCompassErr = 20;
    
    % If making use of the camera, this is the maximum distance at which
    %   the camera can see an object
    global maxCameraRange;
    maxCameraRange = 3; % in meters
        
    % Do not alter the structure of this loop
    % You may add subroutines, etc, but don't change the structure or
    %   the parts that I say should not change or your design is likely
    %   to require re-working prior to your translating it for the ARM
    while (elapsedTime < maxSimTime),
        %fprintf('State %d, Time %f\n',robotState,elapsedTime);
        
        % Do not change this.  Here is where we check for a sensor 
        %   message.  Everything should happen within one of these case statements.
        %   you can call subroutines (of course) in these cases, but don't
        %   add something outside of them.  You could, however, add new
        %   cases for new sensor types.
        % When a message comes in, you act on it based on the "state" of your 
        %   program's state machine.  You may (or may not) change the state
        %   based on what you observe.  
        pause(0.001);
        [msgType msgVal] = getSensorMessage(serPort);
        switch (msgType)
            case 1,
                % Timer message: msgVal is in seconds
                %   You can use this to drive things that should be driven
                %   by a timer.
                %
                % Don't remove the next line
                elapsedTime = elapsedTime + msgVal;
                %
                % process waiting states (these aren't important, they are
                %   just here to show what can be done *and* to make sure
                %   we never send out more than one robot command between
                %   receiving messages.
                if (robotState == WAIT_TO_TURN),
                    waitTime = waitTime + msgVal;
                    if (waitTime >= waitInStateTime),
                        % Start a random turn, we'll stop when not bumped
                        randDir = ((2*rand(1))-1)*180;
                        fprintf('Starting random turn: %f degrees\n',randDir);
                        startRobotTurn(serPort,randDir,turnSpeed);
                        robotState = RAND_TURNING;  
                    else,
                        % fprintf('Waiting\n');
                    end;
                elseif (robotState == WAIT_TO_FORWARD),
                    waitTime = waitTime + msgVal;
                    if (waitTime >= waitInStateTime),
                        moveDistance = forwardDistance + forwardDistance*0.5*((rand(1)*2)-1);
                        startRobotForward(serPort,moveDistance,forwardSpeed);
                        robotState = MOVING_STATE;
                        fprintf('Starting new move of %f meters\n',moveDistance);  
                    else,
                        % fprintf('Waiting\n');
                    end;
                elseif (robotState == WAIT_TO_TARGET),
                    waitTime = waitTime + msgVal;
                    if (waitTime >= waitInStateTime),
                        startRobotTurn(serPort,targetAngle,turnSpeed);
                        robotState = TURN_TO_TARGET;
                        fprintf('Starting target turn %f\n',targetAngle);  
                    else,
                        % fprintf('Waiting\n');
                    end;
                elseif (robotState == WAIT_TO_TARGET_RUN),
                    waitTime = waitTime + msgVal;
                    if (waitTime >= waitInStateTime),
                        startRobotForward(serPort,targetDistance,forwardSpeed);
                        robotState = TARGET_RUN;
                        fprintf('Starting run to target within %f meters\n',targetDistance);  
                    else,
                        % fprintf('Waiting\n');
                    end;
                else
                    % Do nothing
                end;
            case 2,
                % Sonar message: msgVal is in distance (meters)
                %   Value of 1000 means nothing in range *OR* object too
                %   close
                %
                % The very simple algorithm here just ignores sonar
                % readings.
            case 3,
                % Bumper message: msgVal is an array of [left front right]
                %   where 1 is bumped and 0 is not
                if (max(msgVal) > 0),
                    if ((robotState == MOVING_STATE) || (robotState == RAND_TURNING) || (robotState == IDLE_STATE)),
                        stopRobot(serPort),
                        robotState = BUMPED_STOPPED;
                    elseif ((robotState == BUMPED_TURNING) || (robotState == BUMPED_STOPPED)),
                        % Do nothing, we need to turn until we are not
                        % bumped
                    elseif (robotState == BUMPED_IDLE),
                        % Start a full turn, we'll stop when not bumped
                        fprintf('Bumped: Turning to avoid (%d %d %d)\n',msgVal);
                        startRobotTurn(serPort,360,turnSpeed);
                        robotState = BUMPED_TURNING;
                    elseif ((robotState == TURN_TO_TARGET) || (robotState == TARGET_RUN)),
                        fprintf('Hit an obstacle on the way to the target: Algorithm terminated\n');
                        return;
                    else,
                        fprintf('Error: Unexpected state=%d (%d %d %d)\n',robotState,msgVal);
                        return;
                    end;
                elseif ((robotState == BUMPED_TURNING) || (robotState == BUMPED_IDLE) || (robotState == BUMPED_STOPPED)),
                    % No longer bumped, ready to start a new forward move
                    % Just stop the robot and make IDLE, let the motion
                    % update actually start it moving
                    stopRobot(serPort);
                    robotState = IDLE_STATE;
                end;
            case 4,
                % Robot Motion Update: msgVal is a 3x1 array of
                % 1: true if any previously initiated motion is complete
                % 2: meters moved forward since last update
                % 3: degrees turned since last update (positive is
                %    counter-clockwise
                % NOTE: The reported meters and angle turned are based
                %  on the speed of the motors, NOT on the real position
                %  of the robot.  There is  some error in this
                %  computed position and orientation.  Also, if the robot
                %  is up against a wall and the motors are still running
                %  it will report that it is moving -- but it isn't.  This
                %  is meant to reflect the use of encoders to determine
                %  distance moved.
                %
                % Update my estimated position
                %   calculation made under the (mostly correct) assumption that
                %   the robot is either turning *or* moving forward.
                robotOrientation = rem((robotOrientation + msgVal(3)),360);
                robotPosition(1) = robotPosition(1) + cos(deg2rad(robotOrientation))*msgVal(2);
                robotPosition(2) = robotPosition(2) + sin(deg2rad(robotOrientation))*msgVal(2);
                % Calling the true position NOT to use, but to just compute
                % error in the estimated position for reporting/debugging
                if (elapsedTime >= robotPrintNextTime),
                    robotPrintNextTime= robotPrintNextTime + robotPrintTime;
                    [trueX trueY trueOrient] = OverheadLocalizationCreate(serPort);
                    % make the degrees print out the way we want
                    errOrient = angleDiff(deg2rad(robotOrientation),trueOrient);
                    errX = robotPosition(1) - trueX;
                    errY = robotPosition(2) - trueY;
                    fprintf('Robot Position: est. %f %f %f (err %f %f %f)\n',robotPosition,robotOrientation,errX,errY,errOrient);
                end;
                %
                % If the motion is complete, then do something
                if (msgVal(1)),
                    if ((robotState == RAND_TURNING) || (robotState == IDLE_STATE)),
                        % Enter a state where we are waiting and
                        %   then we'll start to go forward 
                        robotState = WAIT_TO_FORWARD;
                        waitTime = 0.0;
                    elseif (robotState == MOVING_STATE),
                        % Enter a state where we are waiting and
                        %   then we'll start turning
                        robotState = WAIT_TO_TURN;
                        waitTime = 0.0;
                    elseif (robotState == BUMPED_STOPPED),
                        % We have stopped, now make ready to turn (won't
                        % happen here), but will happen *if* we get another
                        % message that says we are still seeing a
                        % bumpsensor "bumped"
                        robotState = BUMPED_IDLE;
                    elseif (robotState == TURN_TO_TARGET),
                        robotState = WAIT_TO_TARGET_RUN;
                        waitTime = 0.0;
                    elseif (robotState == TARGET_RUN),
                        robotState = IDLE_STATE;
                    else
                        % do nothing
                    end;
                end;
            case 5,
                % compass data with errors
                %
                % Use the compass data to improve the estimated position,
                % knowing that the compass data is not correct.
                %
                % Correction Method: We assume that the error of the
                % compass is up to maxCompassErr degrees in either direction.  *If*
                % the estimated position is outside of that range, then we
                % just use the compass reading.  This is simple, but I
                % don't think it is the best way to do it.
                % fprintf('Compass: %f, Est %f, Diff=%f\n',rad2deg(msgVal(1)),robotOrientation,angleDiff(deg2rad(robotOrientation),msgVal(1)));
                if (abs(angleDiff(deg2rad(robotOrientation),msgVal(1))) > maxCompassErr),
                    fprintf('Enforcing compass correction %f %f\n',robotOrientation,rad2deg(msgVal(1)));
                    robotOrientation = rad2deg(msgVal(1));
                end;
            case 6,
                % Camera data ==> looking for a "beacon" which is created
                %   in the map file
                % BEWARE: This camera is too good... but it will help you
                %   develop an algorithm.
                % msgVal(1): angle relative to robot heading (positive to
                % the left, negative to the right) of the "beacon"
                % msgVal(2): estimated distance to a "beacon"
                % Simple Algorithm: (which wouldn't always work and isn't
                % good)
                if (msgVal(2) < 0.2),
                    fprintf('I am at the target.  Finished.\n');
                    return;
                else,
                    if ((robotState == WAIT_TO_TARGET) || (robotState == TURN_TO_TARGET) || (robotState == WAIT_TO_TARGET_RUN) || (robotState == TARGET_RUN)),
                        if (msgVal(2) >= 1000),
                            % We had the target and we lost it
                            fprintf('Lost Target.  Failed\n');
                            return;
                        end;
                    else,
                        % Don't act on the camera value unless the
                        %   object is within a certain range
                        if (msgVal(2) < 2), % within 2 meters
                            targetDistance = msgVal(2);
                            targetAngle = rad2deg(msgVal(1));
                            fprintf('Acquired target, dist=%f, angle=%f\n',targetDistance,targetAngle);
                            stopRobot(serPort);
                            robotState = WAIT_TO_TARGET;
                            waitTime = 0.0;
                        elseif (msgVal(2) <= maxCameraRange),
                            fprintf('Object too far to follow\n');
                        end;
                    end;
                end;
            otherwise,
                fprintf('Fatal Error: %d\n',msgType);
                return;
        end;
    end;

    fprintf('Simulation Finished\n');
    finalRad = 0;
end

% compute the distance between two angles
function [angErr] = angleDiff(ang1,ang2)
    v1 = [cos(ang1) sin(ang1)];
    v2 = [cos(ang2) sin(ang2)];
    ip = v1*v2';
    angErr = rad2deg(acos(ip));
end
 
% returns a message from the sensors
function [msgType msgVal] = getSensorMessage(serPort)
    
    % loop until we have a message
    msgType = 0;
    while (msgType == 0),
        [msgType msgVal] = getTimerMessage(serPort);
        if (msgType ~= 0),
            return;
        end;
        [msgType msgVal] = getSonarMessage(serPort);
        if (msgType ~= 0),
            return;
        end;
        [msgType msgVal] = getBumperMessage(serPort);
        if (msgType ~= 0),
            return;
        end;
        [msgType msgVal] = robotMotionUpdate(serPort);
        if (msgType ~= 0),
            return;
        end;
        [msgType msgVal] = getCompassMessage(serPort);
        if (msgType ~= 0),
            return;
        end;        
        [msgType msgVal] = getCameraMessage(serPort);
        if (msgType ~= 0),
            return;
        end;
        if (msgType == 0),
            pause(0.001)
        end;
    end;      
end

function [msgType msgVal] = getTimerMessage(serPort)
    % This line may be modified to change the amount of time per timer tick
    timeUnit = 1.0/10.0; % Timer tick is 1/10 of a second

    persistent timerVal;
    if (isempty(timerVal)),
        timerVal = tic;
    elseif (class(timerVal) ~= 'uint64'),
        clear functions
        timerVal = tic;
    end;
    
    msgType = 0;
    msgVal = 0;
    if (toc(timerVal) >= timeUnit),
        msgType = 1;
        msgVal = timeUnit;
        timerVal = tic;
    end;      
end

function [msgType msgVal] = getSonarMessage(serPort)
    % This line may be modified to change the amount of time per sonar
    % value
    timeUnit = 1.0/5.0; % seconds

    persistent timerVal;
    if (isempty(timerVal)),
        timerVal = tic;
    elseif (class(timerVal) ~= 'uint64'),
        clear functions
        timerVal = tic;
    end;
    
    msgType = 0;
    msgVal = 0;
    if (toc(timerVal) >= timeUnit),
        msgType = 2;
        msgVal = ReadSonar(serPort,2);
        if (isempty(msgVal)),
        	msgVal = 1000;
        end;
        timerVal = tic;
    end;      
end

function [msgType msgVal] = getBumperMessage(serPort)
    % This line may be modified to change the amount of time per bumper
    % value read
    timeUnit = 1.0/5.0; % seconds

    persistent timerVal;
    if (isempty(timerVal)),
        timerVal = tic;
    elseif (class(timerVal) ~= 'uint64'),
        clear functions
        timerVal = tic;
    end;
    
    msgType = 0;
    msgVal = 0;
    if (toc(timerVal) >= timeUnit),
        msgType = 3;
        msgVal = zeros(3,1);
        [msgVal(3) msgVal(1) dummy1 dummy2 dummy3 msgVal(2)] = BumpsWheelDropsSensorsRoomba(serPort);
        timerVal = tic;
    end;      
end

function [msgType msgVal] = getCompassMessage(serPort)
    % This line may be modified to change the amount of time per sonar
    % value
    timeUnit = 1.0/1.0; % seconds
    global maxCompassErr;

    persistent timerVal;
    if (isempty(timerVal)),
        timerVal = tic;
    elseif (class(timerVal) ~= 'uint64'),
        clear functions
        timerVal = tic;
    end;
    
    % use this to make the compass behavior more like what the real world
    % would be
    persistent drift;
    if (isempty(drift)),
        drift = pi*((2*rand(1))-1);
    end;
    msgType = 0;
    msgVal = 0;
    if (toc(timerVal) >= timeUnit),
        drift = drift + 0.01*pi*rand(1);
        msgType = 5;
        [trueX trueY trueOrient] = OverheadLocalizationCreate(serPort);
        msgVal = trueOrient + deg2rad(maxCompassErr)*cos(trueOrient + drift);
        timerVal = tic;
    end;      
end

function [msgType msgVal] = getCameraMessage(serPort)
    % This line may be modified to change the amount of time per sonar
    % value
    timeUnit = 1.0/2.0; % seconds

    persistent timerVal;
    if (isempty(timerVal)),
        timerVal = tic;
    elseif (class(timerVal) ~= 'uint64'),
        clear functions
        timerVal = tic;
    end;
    
    msgType = 0;
    msgVal = 0;
    if (toc(timerVal) >= timeUnit),
        global maxCameraRange;
        msgType = 6;
        [angle dist color] = CameraSensorCreate(serPort);
        if (isempty(angle) || isempty(dist)),
            msgVal(1) = 0;
            msgVal(2) = 1000;
        elseif (dist > maxCameraRange),
            msgVal(1) = 0;
            msgVal(2) = 1000;
        else,
            msgVal(1) = angle;
            msgVal(2) = dist;
        end;
        timerVal = tic;
    end;      
end

function [msgType msgVal] = robotMotionUpdate(serPort)
    % This line may be modified to change the amount of time per motion
    % update read
    timeUnit = 1.0/20.0; % seconds

    persistent timerVal;
    if (isempty(timerVal)),
        timerVal = tic;
    elseif (class(timerVal) ~= 'uint64'),
        clear functions
        timerVal = tic;
    end;
    
    msgType = 0;
    msgVal = 0;
    %fprintf('Checking %f %d\n',toc(timerVal),timeUnit);
    if (toc(timerVal) >= timeUnit),
        msgType = 4;
        timerVal = tic;
        global goalAngle currentAngle;
        global goalDistance currentDistance;
    
        complete = false;
        msgVal = zeros(3,1);
        msgVal(2) = DistanceSensorRoomba(serPort);
        msgVal(3) = rad2deg(AngleSensorRoomba(serPort));
        if (goalDistance ~= 0), 
            currentDistance = currentDistance + msgVal(2);
            if (currentDistance >= goalDistance),
                complete = true;
                currentDistance = 0;
                goalDistance = 0;
                SetFwdVelAngVelCreate(serPort,0,0);
            end;
        elseif (goalAngle ~= 0),  
            currentAngle = currentAngle + msgVal(3);
            if (abs(currentAngle) >= abs(goalAngle)),
                complete = true;
                currentAngle = 0;
                goalAngle = 0;
                SetFwdVelAngVelCreate(serPort,0,0);
            end;
        else
            complete = true;
        end;
        msgVal(1) = complete;
    end;
end

% called at the start of the program
function initRobotControl()
    x= 3;
    global goalAngle currentAngle;
    global goalDistance currentDistance;

    currentRobotCommand = 0;
    goalAngle = 0;
    currentAngle = 0;
    goalDistance = 0;
    currentDistance = 0;
end

% Angle is in degrees, positive is counterClockwise
% turnSpeed units unknown to MTJ
function startRobotTurn(serPort,turnAngle,turnSpeed)
    global goalAngle currentAngle;
    global goalDistance currentDistance;

    if (goalAngle ~= 0),
     fprintf('Warning: Previous turn not completed, %f degrees remaining\n',goalAngle);
    end
    if (goalDistance ~= 0),
      fprintf('Warning: Previous forward move not completed, %f meters remaining\n',goalDistance);
    end
    goalAngle = turnAngle;
    currentAngle = 0;
    goalDistance = 0;
    currentDistance = 0;
    if (turnAngle < 0),
        tTurnSpeed = -turnSpeed;
    else
        tTurnSpeed = turnSpeed;
    end;
    SetFwdVelAngVelCreate(serPort,0,tTurnSpeed);
end

% moveDistance in meters
% moveSpeed units unknown to MTJ, but thinks they are meters/sec
function startRobotForward(serPort,moveDistance,moveSpeed)
    global goalAngle currentAngle;
    global goalDistance currentDistance;

    if ((moveSpeed < 0) || (moveDistance < 0)),
        fprintf('Bad Error: Distance or speed is set to less than zero %f %f\n',moveSpeed,moveDistance);
        fprintf('Refusing to move\n');
        return;
    end;
    if (goalAngle ~= 0),
     fprintf('Warning: Previous turn not completed, %f degrees remaining\n',goalAngle);
    end
    if (goalDistance ~= 0),
      fprintf('Warning: Previous forward move not completed, %f meters remaining\n',goalDistance);
    end
    goalAngle = 0;
    currentAngle = 0;
    goalDistance = moveDistance;
    currentDistance = 0;
    SetFwdVelAngVelCreate(serPort,moveSpeed,0);
end

% stop any type of move/turn
function stopRobot(serPort)
    global goalAngle currentAngle;
    global goalDistance currentDistance;

    goalAngle = 0;
    currentAngle = 0;
    goalDistance = 0;
    currentDistance = 0;
    SetFwdVelAngVelCreate(serPort,0,0);
end
