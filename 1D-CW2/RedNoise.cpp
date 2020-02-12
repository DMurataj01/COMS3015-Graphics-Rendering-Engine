#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace glm;

#define WIDTH 320
#define HEIGHT 240

void drawRedNoise();
void drawColour();
void update();
void handleEvent(SDL_Event event);
vector<vec3> interpolate(vec3 from, vec3 to, float numberOfValues);
void drawLine(vec2 start, vec2 end, vec3 colour);
void drawStrokedTriangle(vec2 point1, vec2 point2, vec2 point3, vec3 colour);
void drawRandomTriangle();
void drawFilledTriangle(vec2 point1, vec2 point2, vec2 point3, vec3 colour);
void drawRandomFilledTriangle();
void readImage();

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[])
{
  window.clearPixels();
  SDL_Event event;
  readImage();
  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();
    //drawFilledTriangle(vec2 (20,0), vec2 (100,100), vec2 (300,200), vec3 (255,0,0));
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
  
}

void drawRedNoise(){
  window.clearPixels();
  for(int y=0; y<window.height ;y++) {
    for(int x=0; x<window.width ;x++) {
      float red = rand() % 255;
      float green = 0.0;
      float blue = 0.0;
      uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
      window.setPixelColour(x, y, colour);
    }
  }
}

void drawColour()
{
  window.clearPixels();
  int width = window.width;
  int height = window.height;
  vec3 topLeft(255,0,0); //red
  vec3 topRight(0,0,255); //blue
  vec3 bottomLeft(255,255,0); //yellow
  vec3 bottomRight(0,255,0); //green

  //First get the top and bottom rows and then for each matching pair on the vertical, interpolate between them to get the column
  vector<vec3> topRow = interpolate(topLeft, topRight, width);
  vector<vec3> bottomRow = interpolate(bottomLeft, bottomRight, width);

  //For each column
  for(int i=0 ; i<width ; i++){
    //Get the correct rgb values for each pixel
    vec3 top = topRow[i];
    vec3 bottom = bottomRow[i];
    vector<vec3> column = interpolate(top, bottom, height);
    //For each pixel in the column, set the rgb value
    for(int j=0 ; j<height ; j++){
      vec3 rgb = column[j];
      int red = int(rgb[0]);
      int green = int(rgb[1]);
      int blue = int(rgb[2]);
      uint32_t colour = (255<<24) + (red<<16) + (green<<8) + blue;
      window.setPixelColour(i, j, colour);
    }
  }
}

void update()
{
  // Function for performing animation (shifting artifacts or moving the camera)
}

void handleEvent(SDL_Event event)
{
  if(event.type == SDL_KEYDOWN) {
    if(event.key.keysym.sym == SDLK_LEFT) cout << "LEFT" << endl;
    else if(event.key.keysym.sym == SDLK_RIGHT) cout << "RIGHT" << endl;
    else if(event.key.keysym.sym == SDLK_UP) cout << "UP" << endl;
    else if(event.key.keysym.sym == SDLK_DOWN) cout << "DOWN" << endl;
    else if(event.key.keysym.sym == SDLK_u) drawRandomTriangle();
    else if(event.key.keysym.sym == SDLK_f) drawRandomFilledTriangle();
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}


vector<vec3> interpolate(vec3 from, vec3 to, float numberOfValues)
{
  vector<vec3> out; //The output vector of numbers
  out.push_back(from); //Add the first number in
  vec3 stepValue = (to - from) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted
  vec3 previous = from;

  //For each step
  for (int i = 1; i < numberOfValues ; i++)
  {
    vec3 input = previous + stepValue;
    out.push_back(input);
    previous = input;
  }

  return out;
}

void drawLine(vec2 start, vec2 end, vec3 colour) {
  float diffX = end[0] - start[0];
  float diffY = end[1] - start[1];
  float numberOfSteps = std::max(abs(diffX),abs(diffY));
  float stepSizeX = diffX / numberOfSteps;
  float stepSizeY = diffY / numberOfSteps;
  
  // getting the colour
  int red = int(colour[0]);
  int green = int(colour[1]);
  int blue = int(colour[2]);
  uint32_t col = (255<<24) + (red<<16) + (green<<8) + blue;

  for (int i = 0 ; i < numberOfSteps ; i++){
    int x = round(start[0] + (i * stepSizeX));
    int y = round(start[1] + (i * stepSizeY));
    window.setPixelColour(x,y,col);
  }
}

void drawStrokedTriangle(vec2 point1, vec2 point2, vec2 point3, vec3 colour){
  drawLine(point1,point2,colour);
  drawLine(point2,point3,colour);
  drawLine(point3,point1,colour);
}

void drawRandomTriangle(){
  int x1 = round(rand()%WIDTH);
  int x2 = round(rand()%WIDTH);
  int x3 = round(rand()%WIDTH);
  int y1 = round(rand()%HEIGHT);
  int y2 = round(rand()%HEIGHT);
  int y3 = round(rand()%HEIGHT);

  int red = round(rand()%255);
  int green = round(rand()%255);
  int blue = round(rand()%255);

  drawStrokedTriangle(vec2(x1,y1), vec2(x2,y2), vec2(x3,y3), vec3(red,green,blue));
}

void drawFilledTriangle(vec2 point1, vec2 point2, vec2 point3, vec3 colour){
  // putting all the points in a vector so they can be retrieved by index
  vector<vec2> points;
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);

  // sort the points by the y value
  for (int i = 0 ; i < 2 ; i++){
    vec2 pointOne = points[i];
    vec2 pointTwo = points[i+1];
    // if the next point has a lower y value then swap them
    if (pointOne[1] > pointTwo[1]){
      points[i+1] = pointOne;
      points[i] = pointTwo;
    }
  }
  // still sorting : check the first two again
  vec2 pointOne = points[0];
  vec2 pointTwo = points[1];
  if (pointOne[1] > pointTwo[1]){
    points[0] = pointTwo;
    points[1] = pointOne;
  }

  vec2 maxPoint = points[0];
  vec2 middlePoint = points[1];
  vec2 minPoint = points[2];
  // the vertical distance between the highest point and the middle point
  float yDistance = middlePoint[1] - maxPoint[1];
  // interpolating to find the cutting point
  float maxYDistance = minPoint[1] - maxPoint[1];
  float maxXDistance = minPoint[0] - maxPoint[0];
  float yProportion = yDistance / maxYDistance;
  float xDistance = maxPoint[0] + (yProportion * maxXDistance);
  vec2 cutterPoint(xDistance, maxPoint[1] + yDistance);

  // the upper triangle
  // for each row, fill it in
  float steps = middlePoint[1] - maxPoint[1]; // how many rows
  for (int i = 0 ; i < steps + 1 ; i++){
    // find the two points which intersect this row
    float yDiff = i / steps;
    float maxXDiff1 = maxPoint[0] - middlePoint[0];
    float maxXDiff2 = maxPoint[0] - cutterPoint[0];
    float xStart = round(maxPoint[0] - (yDiff * maxXDiff1));
    float xEnd = round(maxPoint[0] - (yDiff * maxXDiff2));
    vec2 start(xStart, maxPoint[1] + i);
    vec2 end(xEnd, maxPoint[1] + i);
    drawLine(start,end,colour);
  }
  
  // the lower triangle
  // for each row, fill it in
  float steps2 = minPoint[1] - middlePoint[1]; // how many rows
  for (int i = steps2 ; i > 0 ; i--){
    // find the two points which intersect this row
    float yDiff = 1 - (i / steps2);
    float maxXDiff1 = minPoint[0] - middlePoint[0];
    float maxXDiff2 = minPoint[0] - cutterPoint[0];
    float xStart = round(minPoint[0] - (yDiff * maxXDiff1));
    float xEnd = round(minPoint[0] - (yDiff * maxXDiff2));
    vec2 start(xStart, cutterPoint[1] + i);
    vec2 end(xEnd, cutterPoint[1] + i);
    drawLine(start,end,colour);
  }
  /*
  drawLine(point1,point2,vec3 (255,255,255));
  drawLine(point2,point3,vec3 (255,255,255));
  drawLine(point3,point1,vec3 (255,255,255));
  drawLine(points[1],cutterPoint,vec3 (255,255,255));
  */
}

void drawRandomFilledTriangle(){
  int x1 = round(rand()%WIDTH);
  int x2 = round(rand()%WIDTH);
  int x3 = round(rand()%WIDTH);
  int y1 = round(rand()%HEIGHT);
  int y2 = round(rand()%HEIGHT);
  int y3 = round(rand()%HEIGHT);

  int red = round(rand()%255);
  int green = round(rand()%255);
  int blue = round(rand()%255);

  cout << "Point 1: " << x1 << ", " << y1 << "\n";
  cout << "Point 2: " << x2 << ", " << y2 << "\n";
  cout << "Point 3: " << x3 << ", " << y3 << "\n\n";

  drawFilledTriangle(vec2(x1,y1), vec2(x2,y2), vec2(x3,y3), vec3(red,green,blue));
}


  struct InputPixel {
    int r;
    int g;
    int b;
  };
void readImage() {

  vector<InputPixel> inputArray;

  std::ifstream ifs;
  ifs.open ("texture_no_header.ppm", std::ifstream::in);

  unsigned char c = ifs.get();
  int count = 1;
  cout << "val: " << c;

  while (ifs.good()) {
    std::cout << int(c) << " ";
    c = ifs.get();
    count++;
  }

  cout << "\n\n\nCount: " << count << "\n";
  ifs.close();

}

  /*
  while (fread(val, 1, 128, fp)) { //1 byte read
    cout << "Count: " << count << "\n";
    int val_integer = 0;
    switch (count) {
      case 0: 
        tempVec.r = val_integer;
        count++;
        break;
      case 1:
        tempVec.g = val_integer;
        count++;
        break;
      case 2: 
        tempVec.b = val_integer;
        inputArray.push_back(tempVec);
        count = 0;
        break;
      default:
        cout << "BROKEN!!!!!!!";
        break;
    }


    printf("%s", val);

    count++;
  } // end of for loop

  // close file
  */
