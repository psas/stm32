This is the flight software for the RocketNet Hub.

##Command documentation
 - Commands are standard RCI commands issued over a tcp socket on port 23. 
 - They are ASCII and terminated with CRLF.
 - May return ASCII data terminated with CRLF
 - Will close the socket after a successful command, or after a 5 second timeout.
 - They follow the general format
   - #
   - four character code
   - optional single character subcommand
   - arbitrary binary data of varying lengh encoded in ASCII hex
   - CRLF

### Implemented commands
 - #PORT  - Manages the RNH power/data ports
   - Subcommands
     - S  - Returns two bytes of ASCII hex representing the bitmask of active ports.
     - F  - Returns two bytes of ASCII hex representing the bitmask of faulting ports
     - O + two bytes ASCII hex  - Turns on ports given by the bitmask in the data section. Returns active ports
     - X + two bytes ASCII hex  - Turns off ports given by the bitmask in the data section. Returns active ports.
     - Q + four bytes ASCII hex  - Sets the port current sample rate to the value in the data section Returns -1 in in ASCII hex (FF) if invalid or nothing on success. 
 - #YOLO  - Arms the RNH
   - Subcommands
     - None
   - Returns
     - P + two bytes of ASCII hex  - not all ports are on + bitmask of active ports
     - F + two bytes of ASCII hex  - ports are faulting + bitmask of faulting ports
     - A + six bytes of ASCII hex  - battery alarms are active + raw values of BQ3060 alarm registers
     - S  - Shorepower is on
     - go go go!  - Successful arm
 - #SAFE  - Sets the RNH into a safe state. Will not fail.
 - #RRDY  - Manages the Rocket Ready signal.
   - Subcommands
     - A  - Sets Rocket Ready
     - Any single character but A  - Clears Rocket Ready
   - Returns
     - 1 if Rocket Ready is active, 0 if inactive
 - #UMBD
   - Returns Umbilical detect status. 1 if connected, 0 if disconnected
 - #TIME
   - Returns nanoseconds since boot as 16 ASCII hex characters.
 - #VERS
   - Returns version string
 - #SLEP  - Puts the RNH to sleep if all conditions are met
   - Returns
     - P  - if any ports are on
     - S  - if shorepower is on
     - Will not return if successful, as the ethernet switch turns off.
