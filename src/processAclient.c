#include "./../include/processA_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <bmpfile.h>
#include <math.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <sys/socket.h>

#define SEM_PATH_WRITER "/sem_AOS_writer"
#define SEM_PATH_READER "/sem_AOS_reader"
#define MAX 256 // size of the buffer to communicate using sockets
#define WIDTH 1600 // width of the image (in pixels)
#define HEIGHT 600 // height of the image (in pixels)
#define DEPTH 1 // depth of the image (1 for greyscale images, 4 for colored images)
#define RADIUS 30 // radius of the circle in the bitmap (in pixels)


// Data structure for storing the dynamic bitmap file
bmpfile_t *bmp;
// Data structure for defining a black pixel (BGRA)
rgb_pixel_t black_pixel = {0, 0, 0, 0};
// Data structure for defining a white pixel (BGRA)
rgb_pixel_t white_pixel = {255, 255, 255, 0};

// This map corresponds to the Shared Memory between both processes
static uint8_t (*map)[HEIGHT];
// Size of the Shared Memory
int SM_SIZE = sizeof(uint8_t[WIDTH][HEIGHT]);

// Variables needed for the log file
FILE * log_file;
time_t rawtime;
struct tm * timeinfo;
char * timestamp;

// Write the current time in the log file
void write_timestamp ()
{
    time(&rawtime); timeinfo = localtime(&rawtime);
    timestamp = asctime(timeinfo); timestamp[strlen(timestamp)-1] = 0; 
    fprintf(log_file, "%s -- ", timestamp);
}

// Draw a circle of the fixed defined radius in the bitmap
void plot_circle()
{
    for(int x = -RADIUS; x <= RADIUS; x++) {
        for(int y = -RADIUS; y <= RADIUS; y++) {
            if(sqrt(x*x + y*y) < RADIUS) // If distance is smaller, point is within the circle
            { 
                // Set the pixel at the specified (x,y) position to a black pixel. The center of the circle will depend on 
                // the position of the circle in the ncurses library (applying the x20 factor)
                bmp_set_pixel(bmp, 20*circle.x + x, 20*circle.y + y, black_pixel);
            }
        }
    }
}

// Erase the content of the whole bitmap
void erase_bitmap()
{
    for(int x = 0; x <= WIDTH-1; x++) {
        for(int y = 0; y <= HEIGHT-1; y++)
        {
            // Set the pixel at the specified (x,y) position to a white pixel
            bmp_set_pixel(bmp, x, y, white_pixel);
        }
    }
}

// Copy the local dynamic bitmap into the shared static map
void write_map()
{
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            rgb_pixel_t *pixel = bmp_get_pixel(bmp, x, y);
            map[x][y] = pixel->blue;
        }
    }
}


int main(int argc, char *argv[])
{
    // Open the log file
    log_file = fopen("log/processAclient.log", "w");
	if (log_file == NULL) {
        perror("Error while opening the log file");
        return 1;
    }

    // Create sockets
    int sockfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed...\n");
        exit(1);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&serv_addr, sizeof(serv_addr));

    // Assign IP ADDRESS and PORT NUMBER of the server according to the values entered by the user
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Connect the client socket to the server socket
    if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) != 0) {
        perror("Connection with the server failed...\n");
        exit(1);
    }
    else
        printf("Connected to the server..\n");

    // Variables needed to send data through the socket
    char buff[MAX];
    int n;


    // Utility variable to avoid trigger resize event on launch 
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Variables for the use of shared memory and semaphores
    const char *shm_name = "/AOS";
    int shm_fd;
    sem_t *sem_id_writer;
    sem_t *sem_id_reader;

    // Open the shared memory, apply 'ftruncate' and map it
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == 1) {
        printf("Shared memory segment failed\n");
        exit(1);
    }

    ftruncate(shm_fd, SM_SIZE);

    map = (uint8_t(*)[HEIGHT])mmap(0, SM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (map == MAP_FAILED) {
        printf("Map failed\n");
        return 1;
    }

    // Open the semaphores
    sem_id_writer = sem_open(SEM_PATH_WRITER, O_CREAT, 0644, 1);
    if(sem_id_writer== (void*)-1){
        perror("sem_open failure");
        exit(1);
    }
    sem_id_reader = sem_open(SEM_PATH_READER, O_CREAT, 0644, 1);
    if(sem_id_reader== (void*)-1){
        perror("sem_open failure");
        exit(1);
    }

    // Initialize the semaphores
    sem_init(sem_id_writer, 1, 1);
    sem_init(sem_id_reader, 1, 0);


    // Instantiate the local dynamic bitmap
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);

    // Plot a circle in the bitmap. Initially (before pressing any keys), the circle will be plotted in the center
    plot_circle();

    // Initialize the Shared Memory with the initial position of the circle. We write on the static map using semaphores
    sem_wait(sem_id_writer);
    write_map();
    sem_post(sem_id_reader);


    // Infinite loop
    while (TRUE)
    {
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Print button pressed");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                    // Save image as a .bmp file
                    bmp_save(bmp, "out/snapshot.bmp");

                    // Report in the logfile
                    write_timestamp(); fprintf(log_file, "%s\n", "Snapshot saved"); fflush(log_file);
                }
            }
        }

        // If input is an arrow key...
        else if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN)
        {
            // Move circle accordingly in the ncurses window
            move_circle(cmd);
            draw_circle();

            // Report in the log file
            if (cmd == KEY_LEFT) {
                write_timestamp(); fprintf(log_file, "%s\n", "Circle moved LEFT"); fflush(log_file);
            }
            else if (cmd == KEY_RIGHT) {
                write_timestamp(); fprintf(log_file, "%s\n", "Circle moved RIGHT"); fflush(log_file);
            }
            else if (cmd == KEY_UP) {
                write_timestamp(); fprintf(log_file, "%s\n", "Circle moved UP"); fflush(log_file);
            }
            else if (cmd == KEY_DOWN) {
                write_timestamp(); fprintf(log_file, "%s\n", "Circle moved DOWN"); fflush(log_file);
            }

            // Send the command through the socket
            sprintf(buff, "%d", cmd);
            if ((n = write(sockfd, buff, sizeof(buff))) < 0) {
                perror("ERROR writing to socket");
            }
            fflush(stdout);

            // Erase the content of the bitmap and replot the circle in the new position
            erase_bitmap();
            plot_circle();

            // Update the content of the Shared Memory, using semaphores
            sem_wait(sem_id_writer);
            write_map();
            sem_post(sem_id_reader);
        }
    }

    // Free resources before termination
    bmp_destroy(bmp);

    sem_close(sem_id_reader);
    sem_close(sem_id_writer);
    sem_unlink(SEM_PATH_READER);
    sem_unlink(SEM_PATH_WRITER);
    munmap(map, SM_SIZE);

    close(sockfd);
    fclose(log_file);
    endwin();
    return 0;
}
