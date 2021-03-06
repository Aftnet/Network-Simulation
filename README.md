# Simple networked simulation application:

By [Alberto Fustinoni](http://aftnet.net)

Prototype of a networked simulation system, dead reckoning techniques are used to compensate for communication latency between instances and keep them as closely synchronized as possible.
The application is a hybrid client/server and uses Winsock to interface to the network.

Use file menu to host a simulation or to connect a server running one.
On the server, keys WASD can be used to move the user controlled marble around; 
clients run a parallel simulation kept in sync to the server's but allow no user interaction.

Executable file includes all resources and is standalone; requires at least Windows XP for operation. 
(might work with Windows 2000 but hasn't been tested)

Only one server can be run on a machine at any given time; as many clients as wanted can be run on a machine,
even if it is also running a server.
No support whatsoever for DNS resolving, when connecting to a server input its IP address.

## Credits

None of the images used in this program have been created by me. Here are their sources:

- Level background image:
["Theta Space" by Geier Sven](http://www.sgeier.net/fractals/fractals/06/Theta%20Space.jpg)

- Player controlled bodies image:
["Lightning Rage" by Mmorozbot](http://twewy.wikia.com/wiki/Pin_145)

- AI controlled bodies image:
["Self Found, Others Lost" by Mmorozbot](http://twewy.wikia.com/wiki/Pin_154)
