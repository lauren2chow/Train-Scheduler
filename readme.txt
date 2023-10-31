How to compile the code in Linux:
-Compile with 'make' command. 
-Run the excecutable ./mst <input_file>
    The input file must contain the following format:
    <Train direction> <load time> <cross time>
    where the only valid inputs for train direction are e,E,w and W 
    where the capital letters are assigned as high priority trains.

_____________________________________________________________


Using multi thread scheduling to construct a simulator of an automated 
control system for a railway track. The program uses programming
constructs like threads, mutexes and conditional variables(convars).

The design has two stations (for high and low priority trains) on each side 
of the main track. At each station, one or more trains are loaded with commodities. 
Each train in the simulation commences its loading process at a common start time 
0 of the simulation program. Some trains take more time to load, some less. After 
a train is loaded, it patiently awaits permission to cross the main track, 
subject to the requirements specified below. Only one train can be on the main track 
at any given time. 

The requirements of the automated control system are:
    1. Only one train is on the main track at any given time.
    2. Only loaded trains can cross the main track.
    3. If there are multiple loaded trains, the one with the high priority crosses.
    4. If two loaded trains have the same priority, then:
        a. If they are both traveling in the same direction, the train which finished loading first gets the
        clearance to cross first. If they finished loading at the same time, the one that appeared first in
        the input file gets the clearance to cross first.
        b. If they are traveling in opposite directions, pick the train which will travel in the direction opposite
        of which the last train to cross the main track traveled. If no trains have crossed the main track
        yet, the Westbound train has the priority.
        c. To avoid starvation, if there are three trains in the same direction traveling through the main
        track back to back, the trains waiting in the opposite direction get a chance to dispatch one
        train if any.

