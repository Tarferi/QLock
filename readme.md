# QLock
Starcraft Map Time Locking 

[The official thread](http://www.staredit.net/topic/17839/)

Since the sources for map time locking are already public in QCHK, it makes sense to make PyLock project public as well. Note that this doesn't provide enough protection by itself, maps need to be locked to prevent others from stripping the lock from the map.

This contains the latest development version of the project, unreleased features, with some bugs created, removed, or worse, undetermined.

[This project](https://github.com/ShadowFlare/SFmpqapi) contains sources for the DLL files that are needed by this tool.


Usage:

-i <input_file> Input map file
-o <output_file> Output map file (can be the same as output)
-f <unlock_begin> Relative specification of unlock begin (see below)
-t <unlock_end> Relative specification of unlock end (see below)
-m <message> Message to display when map is locked (see below)
-r Replace all occurances of variablesi in trigger actions
Date format:

\<Years\>:\<Months\>:\<Days\>:\<Hours\>:\<Minutes\>:\<Seconds\>


"Month" = always 30 days

Date Example:

"0:0:-&#8291;1:0:0:0 Means yesterday at this time"

Message format:

This uses Scmdraft string format (See Scmdraft string editor)

Message can include variables %YYYY[F|T]%, %MM[F|T]%, %DD[F|T]%, %HH[F|T]%, %mm[F|T]%, %SS[F|T]% where F means From and T means To


Example:

lock.exe -i my_cool_map.scx -o my_cool_map_locked.scx -f 0:0:-&#8291;1:0:0:0 -t 0:0:1:0:0:0 -m "Yo I made this map time-locked. It will be unplayable on %MMT%.%DDT% at %HHT%:%mmT%."


Above example command will produce my_cool_map_locked.scx that will be only playable for a day (timezones might vary). When it's no longer playable, it will display a message telling people until when it was playable, give them 5 seconds to read it and then defeat for all players.


This project repairs STR section and is safe to use in loops (unlike PyLock)
