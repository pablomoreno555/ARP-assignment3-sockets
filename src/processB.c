#include "./../include/processB_utilities.h"
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
#define SEM_PATH_WRITER "/sem_AOS_writer"
#define SEM_PATH_READER "/sem_AOS_reader"

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

// Variables used to track the trajectory of the circle
int trajectory_x[WIDTH*HEIGHT];
int trajectory_y[WIDTH*HEIGHT];
int trajectory_index = 0;

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

// Copy the shared static map into the local dynamic bitmap
void read_map()
{
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            if (map[x][y] == 0) {
                bmp_set_pixel(bmp, x, y, black_pixel);
            }
            else {
                bmp_set_pixel(bmp, x, y, white_pixel);
            }
        }
    }
}

// Find the center of the circle in the shared static map
void find_center()
{
    int current_diameter = 0;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) 
        {
            if (current_diameter == 2*RADIUS-1 && x != trajectory_x[trajectory_index-1] && y != trajectory_y[trajectory_index-1])
            {
                // Center found! Get the coordinates of the center and store them in the corresponding arrays
                int center_x = x - RADIUS;
                int center_y = y;

                trajectory_x[trajectory_index] = center_x;
                trajectory_y[trajectory_index] = center_y;
                trajectory_index++;

                // Report in the log file
                write_timestamp();
                fprintf(log_file, "%s", "Circle found centered at image coordinates: x = ");
                fprintf(log_file, "%d", x);
                fprintf(log_file, "%s", ", y = ");
                fprintf(log_file, "%d\n", y); fflush(log_file);

                return;
            }
            else if(map[x][y] == 0) { // Pixel corresponds to circle
                current_diameter++;
            }
            else { // Pixel corresponds to background
                current_diameter = 0;
            }        
        }
    }
}


int main(int argc, char const *argv[])
{
    // Open the log file
    log_file = fopen("log/processB.log", "w");
	if (log_file == NULL) {
        perror("Error while opening the log file");
        return 1;
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Variables for the use of shared memory and semaphores
    const char *shm_name = "/AOS";
    int shm_fd;
    sem_t *sem_id_writer;
    sem_t *sem_id_reader;

    // Open the shared memory and map it
    shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == 1) {
        printf("Shared memory segment failed\n");
        exit(1);
    }

    map = (uint8_t(*)[HEIGHT])mmap(0, SM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (map == MAP_FAILED) {
        printf("Map failed\n");
        return 1;
    }

    // Open the semaphores
    sem_id_writer = sem_open(SEM_PATH_WRITER, 0);
    if(sem_id_writer== (void*)-1){
        perror("sem_open failure");
        exit(1);
    }
    sem_id_reader = sem_open(SEM_PATH_READER, 0);
    if(sem_id_reader== (void*)-1){
        perror("sem_open failure");
        exit(1);
    }

    // Instantiate the local dynamic bitmap
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);

    // Initialize the trajectory of the center of the circle
    trajectory_x[0] = -1;
    trajectory_y[0] = -1;
    trajectory_index = 1;

    // Infinite loop
    while (TRUE)
    {
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE)
        {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();

                // Re-print the entire trajectory described by the circle in the ncurses window, applying the /20 factor
                for (int i = 1; i < trajectory_index; i++) {
                    mvaddch(ceil(trajectory_y[i] / 20), ceil(trajectory_x[i] / 20), '0');
                }
            }
        }
        else
        {
            // Read the shared static map and find the center of the circle, using semaphores to access the Shared Memory
            sem_wait(sem_id_reader);
            read_map();
            find_center();
            sem_post(sem_id_writer);
            
            // Print the entire trajectory described by the circle in the ncurses window, applying the /20 factor
            for (int i = 1; i < trajectory_index; i++) {
                mvaddch(ceil(trajectory_y[i] / 20), ceil(trajectory_x[i] / 20), '0');
            }

            refresh();
        }
    }

    // Unlink the shared memory
    if (shm_unlink(shm_name) == 1)
    {
        printf("Error removing %s\n", shm_name);
        exit(1);
    }

    // Free resources before termination
    bmp_destroy(bmp);

    sem_close(sem_id_reader);
    sem_close(sem_id_writer);
    sem_unlink(SEM_PATH_READER);
    sem_unlink(SEM_PATH_WRITER);

    fclose(log_file);
    endwin();
    return 0;
}
