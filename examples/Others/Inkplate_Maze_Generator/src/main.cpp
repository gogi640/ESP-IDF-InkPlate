/*
   Inkplate_Maze_Generator sketch for Soldered Inkplate 6
   Select "Inkplate 6(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 6(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   This example renders a random maze every time!
   You can write on it with a whiteboard marker or a graphite pen to solve it.
   Just be sure not to use pernament markers!

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   15 July 2020 by Soldered
*/

// If your Inkplate doesn't have external (or second) MCP I/O expander, you should uncomment next line,
// otherwise your code could hang out when you send code to your Inkplate.
// You can easily check if your Inkplate has second MCP by turning it over and 
// if there is missing chip near place where "MCP23017-2" is written, but if there is
// chip soldered, you don't have to uncomment line and use external MCP I/O expander
//#define ONE_MCP_MODE

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "inkplate.hpp"

static const char * TAG = "Maze";

// Initialise Inkplate object
Inkplate display(DisplayMode::INKPLATE_1BIT);

// Here we define one cell size
const int cellSize = 10;

// Calculate screen width and height
int w, h;
char * maze;

// Move direction difference array
int dx[] = {-1, 0, 0, 1};
int dy[] = {0, -1, 1, 0};

void delay(int msec) { vTaskDelay(msec / portTICK_PERIOD_MS); }

int random(int a) 
{
  // ? -> a
  // r -> RAND_MAX

  long long r = std::rand();
  return ((a * r) / RAND_MAX);
}


// Display the maze
void showMaze(const char *maze, int width, int height)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            if (maze[x + y * width] == 1)
                for (int i = 0; i < 4; ++i)
                {
                    int xx = x + dx[i];
                    int yy = y + dy[i];
                    if (0 <= xx && xx < w && 0 <= yy && yy < h && maze[yy * width + xx] == 1)
                        display.drawLine(3 + x * cellSize + cellSize / 2, 3 + y * cellSize + cellSize / 2,
                                         3 + x * cellSize + cellSize / 2 + (dx[i] * cellSize / 2),
                                         3 + y * cellSize + cellSize / 2 + (dy[i] * cellSize / 2), BLACK);
                }

    display.display();
}

// Carve the maze starting at x, y.
void carveMaze(char *maze, int width, int height, int x, int y)
{
    int x1, y1;
    int x2, y2;
    int dx, dy;
    int dir, count;

    dir = random(4);
    count = 0;
    while (count < 4)
    {
        dx = 0;
        dy = 0;
        switch (dir)
        {
        case 0:
            dx = 1;
            break;
        case 1:
            dy = 1;
            break;
        case 2:
            dx = -1;
            break;
        default:
            dy = -1;
            break;
        }
        x1 = x + dx;
        y1 = y + dy;
        x2 = x1 + dx;
        y2 = y1 + dy;
        if (x2 > 0 && x2 < width && y2 > 0 && y2 < height && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1)
        {
            maze[y1 * width + x1] = 0;
            maze[y2 * width + x2] = 0;
            x = x2;
            y = y2;
            dir = random(4);
            count = 0;
        }
        else
        {
            dir = (dir + 1) % 4;
            count += 1;
        }
    }
}

// Generate maze in matrix maze with size width, height.
void generateMaze(char *maze, int width, int height)
{
  int x, y;

  // Initialize the maze.
  for (x = 0; x < width * height; x++)
  {
    maze[x] = 1;
  }
  maze[1 * width + 1] = 0;

  // Seed the random number generator.
  srand(time(0));

  // Carve the maze.
  for (y = 1; y < height; y += 2)
  {
    for (x = 1; x < width; x += 2)
    {
      carveMaze(maze, width, height, x, y);
    }
  }

  // Set up the entry and exit.
  maze[0 * width + 1] = 0;
  maze[(height - 1) * width + (width - 2)] = 0;
}

void maze_task(void * param)
{
  // Initialise Inkplate
  display.begin();
  display.clearDisplay();

  w = (display.width() - 10) / cellSize;
  h = (display.height() - 10) / cellSize;

  maze = new char[w * h];

  // Generate and display the maze
  generateMaze(maze, w, h);
  showMaze(maze, w, h);

  for (;;) {
    ESP_LOGI(TAG, "Completed...");
    delay(5000);
  }
}


#define STACK_SIZE 10000

extern "C" {

  void app_main()
  {
    TaskHandle_t xHandle = NULL;

    xTaskCreate(maze_task, "mainTask", STACK_SIZE, (void *) 1, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
  }

} // extern "C"