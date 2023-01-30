# ARP Shared Memory Assignment
Second Advanced and Robot Programming (ARP) assignment, by Pablo Moreno Moreno (S5646698).

## Compiling and running the code
In order to compile all the three processes that make up the program, you just need to execute the following command:
```console
./compile.sh
```

Then, you can run the program by executing:
```console
./run.sh
```

Two new windows will pop up: one corresponding to **ProcessA** and another corresponding to **ProcessB**.


## Content of the Repository
Apart from the *compile.sh* and *run.sh* files, the repository is organized as follows:
- The `src` folder contains the source code for the processes: **Master**, **ProcessA**, and **ProcessB**.
- The `bin` folder is where the executable files corresponding to the previous processes are generated after compilation.
- The `out` folder is where the snapshot of the image will be saved as a .bmp file.
- The `include` folder contains all the data structures and methods used within the *ncurses* framework to build the two GUIs.
- The `log` folder is where the log files corresponding to the above processes will be genearated after running the program.

## Project Description
The project consists is the design and development of an interactive simulator of a simplified vision system, able to track an object in a 2-D plane. It requires the use of a shared memory in which two processes operate simultaneously:

- **ProcessA**. It simulates a video-camera, creating a moving image using an ncurses window. Using the arrow keys, the user will move a circle in the window, producing an image in the simulated, shared, video memory. Additionally, when the user presses the button in the ncurses window, a snapshot of the image will be saved in a .bmp file.
- **ProcessB**. It simulates the extraction of the center of the circle from the acquired moving image. Also, it will show the position trace of the center of the circle in a second ncurses window.

On the other hand, the **Master** process is in charge of spawning the previous two processes.


## User Guide
After compiling and running the program as mentioned above, two new windows will pop up: one corresponding to **ProcessA** and another corresponding to **ProcessB**.

In the window corresponding to **ProcessA**, the user can move the representation of the circle using the four arrow keys. As the circle moves in this first window, the position trace of its center will be shown in the window corresponding to **ProcessB**, describing its trajectory.

Additionally, when the user presses the 'P' button in the first window, a snapshot of the dynamic bitmap will be saved as a .bmp file in the */out* folder.

Finally, it should be noted that the first window must have a size of 90x30, while the second one should be 80x30, since that is the defined workspace in which the circle is able to move.


## Required libraries/packages
- *ncurses* library, used to build the two GUIs.
- *libbitmap* library, used to work with the dynamic bitmap data structure.
- *konsole* application, used to display the two ncurses windows.
