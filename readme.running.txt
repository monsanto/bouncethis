First, you must create a conf file. Make a file called "bt.conf" in your bouncethis dir. The commands are:

Admin <user> <pass> - Lets you use remote admin with /admin <user> <pass>
Port <port> - Defines a port.
Password <password> - Sets the password for the last defined port.
Ident <ident> - Sets up Identd to reply with <ident>
LogFile <file.log> - Logs all errors/connections to <file.log>
FilterMode <accept|deny> - Sets filter mode.
Filter <xxx.xxx.xxx.xxx> - Adds a filter with the IP. You can leave off numbers to ban subnets, like 10.24.22. would ban 10.24.22.5, etc.
LimitClones <number> - Allows only <number> connections from the same IP.
LimitConn <number> - Allows only <number> connections on the server.
VHost <xxx.xxx.xxx.xx> - tell bt to bind on that ip.

That should about do it. Have fun!

(Use BounceEditor.exe if you are unsure!)