Compiling:
---------------
You can compile it with "make" command.

Running
---------------
To run just type -> "make run".
To change mount directory add -> fusedir="/targetdirectory" to argument
To change html directory add -> htmldir="/targetdirectory" to argument
Otherwise it is assumed that fusedir ./html_con, and htmldir ./html

Unmounting
---------------
In case of error or unmounting type to command prompt "make clean"
If your fusedir is different than default directory, it should be entered as well with fusedir argument.


HTML Files
---------------
Html files is categorized its name.It may be not possible to converting a encoding to another encoding.They are indicated as "with illegal characters" in its name.
Other files is added to show, our FUSE system reads only html files.
