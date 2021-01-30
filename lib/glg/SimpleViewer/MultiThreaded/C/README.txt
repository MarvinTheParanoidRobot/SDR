OVERVIEW

This C example demonstrates the use of a separate data thread for
supplying data. The data obtained by the data thread are sent to 
the GUI thread using messaging implemented with the ZeroMQ library. 

The C++ version of this example shows an alternative method of 
exchanging data between the data and GUI threads using a queue
and thread locking for thread synchronization.

LINUX BUILD INSTRUCTIONS

To build this example on Linux, make sure the libzmq3-dev package is
installed and add -lzmq library to the list of linked libraries in the 
makefile. The sample makefile is located in the src directory of the 
GLG installation.

WINDOWS BUILD INSTRUCTIONS

To build this example on Windows, install ZeroMQ library using prebuilt
executables listed in the following link:

  http://zeromq.org/distro:microsoft-windows

Edit the ZeroMQ include directory in the project settings to point to
the include directory of the ZeroMQ installation and modify the ZeroMQ
library path to point to an appropriate ZerroMQ library in the ZeroMQ
lib directory. The above link provides a list of ZeroMQ library names 
for different versions of the Visual Studio.
