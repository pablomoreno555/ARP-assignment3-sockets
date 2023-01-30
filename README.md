# ARP Client/Server Assignment
Third Advanced and Robot Programming (ARP) assignment, by Pablo Moreno Moreno (S5646698).

## Compiling and running the code
In order to compile all the processes that make up the program, you just need to execute the following command:
```console
./compile.sh
```

Then, you can run the program by executing:
```console
./run.sh
```

## Content of the Repository
Apart from the *compile.sh* and *run.sh* files, the repository is organized as follows:
- The `src` folder contains the source code for the processes: **Master**, **ProcessA**, **ProcessB**, **ProcessAserver**, and **ProcessAclient**.
- The `bin` folder is where the executable files corresponding to the previous processes are generated after compilation.
- The `out` folder is where the snapshot of the image will be saved as a .bmp file.
- The `include` folder contains all the data structures and methods used within the *ncurses* framework to build the GUIs.
- The `log` folder is where the log files corresponding to the above processes will be genearated after running the program.

## Project Description
The project consists in the design and development of a modfied version of Assignment 2, including client/server features using sockets. In this modification, **ProcessB**, the shared memory and the second ncurses window will remain unchanged, while **ProcessA** includes now the possibility to establish a connection with a client or server for a similar application running on a different machine in the network.

The application, when launched, asks the user to select one of the following execution modalities:
- *normal*: the beahavior is exactly the same as in Assignment 2.
- *server*: the application does not use the keyboard for input: it receives input from another application (on a different machine) running in client mode.
- *client*: the application runs normally as Assignment 2 and, additionally, sends its keyboard input to another application (on a different machine) running in server mode.

When selecting modes 2 and 3, the application asks the user to enter the IP address and the port number of the companion application.


## Processes

The following processes have been developed:

- **ProcessA** (the same as in Assignment 2). It simulates a video-camera, creating a moving image using an ncurses window. Using the arrow keys, the user will move a circle in the window, producing an image in the simulated, shared, video memory. Additionally, when the user presses the button in the ncurses window, a snapshot of the image will be saved in a .bmp file.
- **ProcessB** (the same as in Assignment 2). It simulates the extraction of the center of the circle from the acquired moving image. Also, it will show the position trace of the center of the circle in a second ncurses window.
- **ProcessAclient**. It is a modification of **ProcessA** which, additionally, sends the keyboard input to another application (on a different machine) running in server mode.
- **ProcessAserver**. It is a modification of **ProcessA** which, instead of receiving the commands from the keyboard, receives them from another application (on a different machine) running in client mode.
- **Master**. It has been modified with respect to the one in Assignment 2. Now, it asks the user to select the execution modality between the previous three options, spawning the appropriate processes in each case.


## User Guide
After compiling and running the program as mentioned above, it will ask the user to select the execution modality. In case of selecting the *normal* mode, the behavior will be identical as in Assignment 2.

However, if the user selects the *server* mode, the program will ask them to enter the IP address and the port number of the client they want to connect to and will spawn **ProcessAserver** and **ProcessB**. The server is now listening and waits for the client to connect. When that happens, the two ncurses windows will be displayed and the server becomes able to be commanded by the client, showing the current position of the circle in the first window and its trajectory in the second one.

On the other hand, if the user selects the *client* mode, the program will ask them to enter the IP address and the port number of the server they want to connect to, and will spawn **ProcessAsclient** and **ProcessB**. Before running this mode, it is required that there is a server listening and waiting for the client to connect, otherwise, it will not be possible to establish the connection, terminating the program. However, if there is a server listening to that port, the connection between them will be established and the client will be able to command the application running on the server, in addition to running its own application and displaying the current position of the circle and its trajectory in the corresponding ncurses windows.


## Required libraries/packages
- *ncurses* library, used to build the two GUIs.
- *libbitmap* library, used to work with the dynamic bitmap data structure.
- *konsole* application, used to display the two ncurses windows.
