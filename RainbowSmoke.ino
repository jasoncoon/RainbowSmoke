/*
  Rainbow Smoke pattern for an 32x32 RGB LED Matrix.
 */

#include "SmartMatrix_32x32.h"
#include "Types.h"

SmartMatrix matrix;

const int DEFAULT_BRIGHTNESS = 100;

const rgb24 COLOR_BLACK = { 
  0, 0, 0 };

static const int NUMCOLORS = 11;
static const int COLOR_COUNT = 1024;
static const int WIDTH = 32;
static const int HEIGHT = 32;
int startx = 15;
int starty = 15;

rgb24 colors[COLOR_COUNT];
bool hasColor[WIDTH][HEIGHT];
bool isAvailable[WIDTH][HEIGHT];

// the setup routine runs once when you press reset:
void setup() {   
  // Setup serial interface
  Serial.begin(250000);

  // Initialize 32x32 LED Matrix
  matrix.begin();
  matrix.setBrightness(DEFAULT_BRIGHTNESS);
  matrix.setColorCorrection(cc24);

  // Clear screen
  matrix.fillScreen(COLOR_BLACK);
  matrix.swapBuffers();

  randomSeed(analogRead(5));
}

// the loop routine runs over and over again forever:
void loop() {
  // clear all flags
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      hasColor[x][y] = false;
      isAvailable[x][y] = false;
    }
  }

  matrix.fillScreen(COLOR_BLACK);

  sortColors();

  bool first = true;

  int algorithm = random(3);

  int update = 0;

  for (int i = 0; i < COLOR_COUNT; i++) {
    Point point;
    rgb24 color = colors[i];

    if (first) {
      // use the starting point
      point.x = random(32);
      point.y = random(32);
      first = false;
    }
    else {
      point = getAvailablePoint(algorithm, color);
    }

    isAvailable[point.x][point.y] = false;
    hasColor[point.x][point.y] = true;

    matrix.drawPixel(point.x, point.y, color);

    if (update == 4)
    {
      matrix.swapBuffers();
      update = 0;
    }

    update++;

    markAvailableNeighbors(point);
  }

  matrix.swapBuffers();

  // wait a bit
  delay(3000);
}

void markAvailableNeighbors(Point point) {
  for (int dy = -1; dy <= 1; dy++) {
    int ny = point.y + dy;

    if (ny == -1 || ny == HEIGHT)
      continue;

    for (int dx = -1; dx <= 1; dx++) {
      int nx = point.x + dx;

      if (nx == -1 || nx == WIDTH)
        continue;

      if (!hasColor[nx][ny]) {
        isAvailable[nx][ny] = true;
      }
    }
  }
}

Point getAvailablePoint(int algorithm, rgb24 color) {
  switch (algorithm) {
  case 0:
    return getAvailablePointWithClosestNeighborColor(color);
  case 1:
    return getAvailablePointWithClosestAverageNeighborColor(color);
  case 2:
    // random mix of both
    if (random(2) == 0) {
      return getAvailablePointWithClosestNeighborColor(color);
    }
    else {
      return getAvailablePointWithClosestAverageNeighborColor(color);
    }
  }
}

Point getAvailablePointWithClosestNeighborColor(rgb24 color) {
  Point best;

  // find the pixel with the smallest difference between the current color and all of it's neighbors' colors
  int smallestDifference = 999999;
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      // skip any that arent' available
      if (!isAvailable[x][y])
        continue;

      // loop through its neighbors
      int smallestDifferenceAmongNeighbors = 999999;
      for (int dy = -1; dy <= 1; dy++) {
        if (y + dy == -1 || y + dy == HEIGHT)
          continue;

        for (int dx = -1; dx <= 1; dx++) {
          if (x + dx == -1 || x + dx == WIDTH)
            continue;

          int nx = x + dx;
          int ny = y + dy;

          // skip any neighbors that don't already have a color
          if (!hasColor[nx][ny])
            continue;

          rgb24 neighborColor = matrix.readPixel(nx, ny);

          int difference = colorDifference(neighborColor, color);
          if (difference < smallestDifferenceAmongNeighbors) {
            smallestDifferenceAmongNeighbors = difference;
          }
        }
      }

      if (smallestDifferenceAmongNeighbors < smallestDifference) {
        smallestDifference = smallestDifferenceAmongNeighbors;
        best.x = x;
        best.y = y;
      }
    }
  }

  return best;
}

Point getAvailablePointWithClosestAverageNeighborColor(rgb24 color) {
  Point best;

  int smallestAverageDifference = 999999;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      // skip any that arent' available
      if (!isAvailable[x][y])
        continue;

      int neighborCount = 0;
      int neighborColorDifferenceTotal = 0;

      // loop through its neighbors
      for (int dy = -1; dy <= 1; dy++) {
        if (y + dy == -1 || y + dy == HEIGHT)
          continue;

        for (int dx = -1; dx <= 1; dx++) {
          if (x + dx == -1 || x + dx == WIDTH)
            continue;

          int nx = x + dx;
          int ny = y + dy;

          // skip any neighbors that don't already have a color
          if (!hasColor[nx][ny])
            continue;

          neighborCount++;

          rgb24 neighborColor = matrix.readPixel(nx, ny);

          int difference = colorDifference(neighborColor, color);
          neighborColorDifferenceTotal += difference;
        }
      }

      int averageDifferenceAmongNeighbors = neighborColorDifferenceTotal / neighborCount;

      if (averageDifferenceAmongNeighbors < smallestAverageDifference) {
        smallestAverageDifference = averageDifferenceAmongNeighbors;
        best.x = x;
        best.y = y;
      }
    }
  }

  return best;
}

int colorDifference(rgb24 c1, rgb24 c2) {
  int r = c1.red - c2.red;
  int g = c1.green - c2.green;
  int b = c1.blue - c2.blue;
  return r * r + g * g + b * b;
}

void sortColors() {
  int colorSort = random(5);

  switch (colorSort) {
  case 0:
    sortColorsRGB();
    break;
  case 1:
    sortColorsGBR();
    break;
  case 2:
    sortColorsBRG();
    break;
  case 3:
    sortColorsRGB();
    shuffleColors();
    break;
  case 4:
    sortColorsHSV();
    break;
  }
}

void sortColorsRGB() {
  int i = 0;

  int d = 255 / (NUMCOLORS - 1);

  for (int b = 0; b < NUMCOLORS; b++) {
    for (int g = 0; g < NUMCOLORS; g++) {
      for (int r = 0; r < NUMCOLORS; r++) {
        rgb24 color;
        color.red = r * d;
        color.green = g * d;
        color.blue = b * d;
        colors[i] = color;
        i++;
        if (i == COLOR_COUNT)
          return;
      }
    }
  }
}

void sortColorsGBR() {
  int i = 0;

  int d = 255 / (NUMCOLORS - 1);

  for (int r = 0; r < NUMCOLORS; r++) {
    for (int b = 0; b < NUMCOLORS; b++) {
      for (int g = 0; g < NUMCOLORS; g++) {
        rgb24 color;
        color.red = r * d;
        color.green = g * d;
        color.blue = b * d;
        colors[i] = color;
        i++;
        if (i == COLOR_COUNT)
          return;
      }
    }
  }
}

void sortColorsBRG() {
  int i = 0;

  int d = 255 / (NUMCOLORS - 1);

  for (int g = 0; g < NUMCOLORS; g++) {
    for (int r = 0; r < NUMCOLORS; r++) {
      for (int b = 0; b < NUMCOLORS; b++) {
        rgb24 color;
        color.red = r * d;
        color.green = g * d;
        color.blue = b * d;
        colors[i] = color;
        i++;
        if (i == COLOR_COUNT)
          return;
      }
    }
  }
}

void shuffleColors() {
  for (int a = 0; a < COLOR_COUNT; a++)
  {
    int r = random(a, COLOR_COUNT);
    rgb24 temp = colors[a];
    colors[a] = colors[r];
    colors[r] = temp;
  }
}

void sortColorsHSV() {
  int i = 0;

  float dh = 360.0 / NUMCOLORS;
  float sh = 1.0 / NUMCOLORS;

  for (float h = 0.0; h < 360; h += dh) {
    for (float s = 1.0; s > 0.0; s -= sh) {
      for (float v = 1.0; v > 0.0; v -= sh) {
        rgb24 color = createHSVColor(h, s, v);
        colors[i] = color;
        i++;
        if (i == COLOR_COUNT)
          return;
      }
    }
  }
}

// HSV to RGB color conversion
// Input arguments
// hue in degrees (0 - 360.0)
// saturation (0.0 - 1.0)
// value (0.0 - 1.0)
// Output arguments
// red, green blue (0.0 - 1.0)
void hsvToRGB(float hue, float saturation, float value, float * red, float * green, float * blue) {

  int i;
  float f, p, q, t;

  if (saturation == 0) {
    // achromatic (grey)
    *red = *green = *blue = value;
    return;
  }
  hue /= 60;                  // sector 0 to 5
  i = floor(hue);
  f = hue - i;                // factorial part of h
  p = value * (1 - saturation);
  q = value * (1 - saturation * f);
  t = value * (1 - saturation * (1 - f));
  switch (i) {
  case 0:
    *red = value;
    *green = t;
    *blue = p;
    break;
  case 1:
    *red = q;
    *green = value;
    *blue = p;
    break;
  case 2:
    *red = p;
    *green = value;
    *blue = t;
    break;
  case 3:
    *red = p;
    *green = q;
    *blue = value;
    break;
  case 4:
    *red = t;
    *green = p;
    *blue = value;
    break;
  default:
    *red = value;
    *green = p;
    *blue = q;
    break;
  }
}

#define MAX_COLOR_VALUE     255

// Create a HSV color
rgb24 createHSVColor(float hue, float saturation, float value) {

  float r, g, b;
  rgb24 color;

  hsvToRGB(hue, saturation, value, &r, &g, &b);

  color.red = r * MAX_COLOR_VALUE;
  color.green = g * MAX_COLOR_VALUE;
  color.blue = b * MAX_COLOR_VALUE;

  return color;
}
