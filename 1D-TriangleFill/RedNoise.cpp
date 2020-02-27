#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <assert.h>

using namespace std;
using namespace glm;

#define WIDTH 600
#define HEIGHT 600

/*Pre-defined Functions*/
void update();
void handleEvent(SDL_Event event);

/* Helper Functions */
uint32_t getColour(Colour colour){
  return (255<<24) + (colour.red<<16) + (colour.green<<8) + colour.blue;;
}

vector<vec3> interpolate(vec3 from, vec3 to, int numberOfValues);

/* Custom Functions */

void drawLine(CanvasPoint ptStart, CanvasPoint ptEnd, Colour ptClr);
void drawRandomTriangle();
void drawStrokedTriangle(CanvasTriangle triangle);
void drawFilledTriangle(CanvasTriangle triangle);


DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[]) {
  
  SDL_Event event;
  window.clearPixels();

  //** Draw a stroked triangle.
  drawStrokedTriangle( CanvasTriangle(CanvasPoint(200, 50), CanvasPoint(100, 50), CanvasPoint(150, 0), Colour(0, 255, 255)) );
  drawStrokedTriangle( CanvasTriangle(CanvasPoint(50, 0), CanvasPoint(0, 50), CanvasPoint(100, 50), Colour(255, 0, 255)) );
  //** Draw a filled triangle.
  drawFilledTriangle( CanvasTriangle(CanvasPoint(200, 0), CanvasPoint(200, 100), CanvasPoint(250, 50), Colour(255, 0, 255)) );

  while(true) {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

vector<float> interpolate(float from, float to, int numberOfValues) {
    vector<float> vec_list;
    
    float increment = (to - from) / (numberOfValues-1);
    
    for (int i = 0; i<numberOfValues; i++)
        vec_list.push_back(from + (i*increment));
    
    return vec_list;
}

vector<CanvasPoint> interpolate(CanvasPoint from, CanvasPoint to, float numberOfValues) {
  vector<CanvasPoint> vecInterpVectors;

  //Add the first number in.
  vecInterpVectors.push_back(from);
  vec2 castedFrom = vec2(from.x, from.y);
  vec2 castedTo = vec2(to.x, to.y);
  vec2 stepValue = (castedTo - castedFrom) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted
  vec2 previous = castedFrom;

  for (int i = 1; i < numberOfValues ; i++) {
    vec2 input = previous + stepValue;
    vecInterpVectors.push_back(CanvasPoint(input.x, input.y));
    previous = input;
  }
  return vecInterpVectors;
}

vector<vec3> interpolate(vec3 from, vec3 to, float numberOfValues) {
  vector<vec3> vecInterpVectors;

  //Add the first number in.
  vecInterpVectors.push_back(from);

  vec3 stepValue = (to - from) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted
  vec3 previous = from;

  //For each step
  for (int i = 1; i < numberOfValues ; i++) {
    vec3 input = previous + stepValue;
    vecInterpVectors.push_back(input);
    previous = input;
  }
  return vecInterpVectors;
}


void drawLine(CanvasPoint ptStart, CanvasPoint ptEnd, Colour ptClr) {
  float diffX = (ptEnd.x - ptStart.x);
  float diffY = (ptEnd.y - ptStart.y);
  float noOfSteps = glm::max(abs(diffX), abs(diffY));
  //cout << "Diff: x " << diffX << " y " << diffY << " max " << noOfSteps << "\n";
  
  if (noOfSteps == 0.f) {
    //Draw a point!
    window.setPixelColour( ptStart.x, ptStart.y, getColour(ptClr));
  }
  else {
    float stepSizeX = diffX/noOfSteps;
    float stepSizeY = diffY/noOfSteps;
    //cout << "Step X: " << stepSizeX << " .. Step Y: " << stepSizeY << "\n";
    
    for (int i = 0 ; i < noOfSteps ; i++){
      int x = int(ptStart.x + (i * stepSizeX));
      int y = int(ptStart.y + (i * stepSizeY));

      window.setPixelColour( x, y, getColour(ptClr));
    }
  }
  return;
}

CanvasTriangle sortTriangleVertices(CanvasTriangle tri) {
  //** Unrolled 'Bubble Sort' for 3 items.

  if (tri.vertices[1].y < tri.vertices[0].y)
    swap(tri.vertices[0], tri.vertices[1]);
  else if (tri.vertices[1].y == tri.vertices[0].y)
      if (tri.vertices[1].x < tri.vertices[0].x)
        swap(tri.vertices[0], tri.vertices[1]);

  if (tri.vertices[2].y < tri.vertices[1].y)
    swap(tri.vertices[1], tri.vertices[2]);
  else if (tri.vertices[2].y == tri.vertices[1].y)
      if (tri.vertices[2].x < tri.vertices[1].x)
        swap(tri.vertices[1], tri.vertices[2]);

  if (tri.vertices[1].y < tri.vertices[0].y)
    swap(tri.vertices[0], tri.vertices[1]);
  else if (tri.vertices[1].y == tri.vertices[0].y)
      if (tri.vertices[1].x < tri.vertices[0].x)
        swap(tri.vertices[0], tri.vertices[1]);

  return tri;
}

void drawStrokedTriangle(CanvasTriangle triangle){
  /* Vertex sorting NOT required */
  CanvasPoint pt0 = triangle.vertices[0];
  CanvasPoint pt1 = triangle.vertices[1];
  CanvasPoint pt2 = triangle.vertices[2];

  drawLine(pt0, pt1, triangle.colour);
  drawLine(pt2, pt0, triangle.colour);
  drawLine(pt1, pt2, triangle.colour);

  return;
}



void fillFlatBottomTriangle(CanvasTriangle triangle) {
  //Assumption: last two vertices represent the flat bottom.
  //Assumption: last two y values are both the same.
  assert( triangle.vertices[1].y == triangle.vertices[2].y);

  if (triangle.vertices[2].x < triangle.vertices[1].x)
    swap(triangle.vertices[1], triangle.vertices[2]);
  
  int noOfRows = triangle.vertices[1].y - triangle.vertices[0].y;

  //cout << "FlatBot X0:  x: " << triangle.vertices[0].x << " y: " << triangle.vertices[0].y << "\n";
  //cout << "FlatBot X1:  x: " << triangle.vertices[1].x << " y: " << triangle.vertices[1].y << "\n";
  //cout << "FlatBot X2:  x: " << triangle.vertices[2].x << " y: " << triangle.vertices[2].y << "\n";


  vector<CanvasPoint> interpLeft = interpolate(triangle.vertices[0], triangle.vertices[1], noOfRows);
  vector<CanvasPoint> interpRight = interpolate(triangle.vertices[0], triangle.vertices[2], noOfRows);

  //** Do line by line interp.
  for (int i=0; i< noOfRows; i++)
    drawLine(CanvasPoint(interpLeft[i].x, triangle.vertices[0].y + i), CanvasPoint(interpRight[i].x, triangle.vertices[0].y + i), triangle.colour);

  return;
}

void fillFlatTopTriangle(CanvasTriangle triangle) {
  //Assumption: last two vertices represent the flat bottom.
  //Assumption: last two y values are both the same.
  assert( triangle.vertices[0].y == triangle.vertices[1].y);

  if (triangle.vertices[1].x < triangle.vertices[0].x)
    swap(triangle.vertices[0], triangle.vertices[1]);
  
  int noOfRows = triangle.vertices[2].y - triangle.vertices[0].y;

  //cout << "FlatTop X0:  x: " << triangle.vertices[0].x << " y: " << triangle.vertices[0].y << "\n";
  //cout << "FlatTop X1:  x: " << triangle.vertices[1].x << " y: " << triangle.vertices[1].y << "\n";
  //cout << "FlatTop X2:  x: " << triangle.vertices[2].x << " y: " << triangle.vertices[2].y << "\n";

  vector<CanvasPoint> interpLeft = interpolate(triangle.vertices[0], triangle.vertices[2], noOfRows);
  vector<CanvasPoint> interpRight = interpolate(triangle.vertices[1], triangle.vertices[2], noOfRows);

  //** Do line by line interp.
  for (int i=0; i< noOfRows; i++)
    drawLine(CanvasPoint(interpLeft[i].x, triangle.vertices[0].y + i), CanvasPoint(interpRight[i].x, triangle.vertices[0].y + i), triangle.colour);

  return;
}

void drawFilledTriangle(CanvasTriangle triangle){
  
  //** 1. Draw outline.
  //drawStrokedTriangle(triangle);

  //** 2. Sort Vertices before fill.  
  triangle = sortTriangleVertices(triangle);

  //** 3. Get the Cut Point  
  float xDiff = triangle.vertices[2].x - triangle.vertices[0].x;
  float yDiff = triangle.vertices[2].y - triangle.vertices[0].y;

  CanvasPoint cutPoint;
  if (yDiff == 0.f) 
    cout << "0"; //cutPoint = CanvasPoint(triangle.vertices[0].x, triangle.vertices[2].y);
  else {
    float cut_y = triangle.vertices[1].y;
    float cut_x = triangle.vertices[0].x + cut_y * xDiff / yDiff;
    cout << "Start Point:  x: " << triangle.vertices[0].x << " y: " << triangle.vertices[0].y << "\n";
    cout << "Middle Point:  x: " << triangle.vertices[1].x << " y: " << triangle.vertices[1].y << "\n";
    cout << "End Point:  x: " << triangle.vertices[2].x << " y: " << triangle.vertices[2].y << "\n";
    cout << "Cutting Point:  x: " << cut_x << " y: " << cut_y << "\n\n\n";
    cutPoint = CanvasPoint(cut_x, cut_y);
  }
  
  //drawLine(cutPoint, triangle.vertices[1], Colour(0, 200, 150));

  fillFlatBottomTriangle(CanvasTriangle(triangle.vertices[0], cutPoint, triangle.vertices[1]));
  fillFlatTopTriangle(CanvasTriangle(cutPoint, triangle.vertices[1], triangle.vertices[2]));
  return;
}


void drawRandomTriangle() {
  return;
}


void update() {
  // Function for performing animation (shifting artifacts or moving the camera)
}

void handleEvent(SDL_Event event) {
  if(event.type == SDL_KEYDOWN) {
    if(event.key.keysym.sym == SDLK_LEFT) cout << "LEFT" << endl;
    else if(event.key.keysym.sym == SDLK_RIGHT) cout << "RIGHT" << endl;
    else if(event.key.keysym.sym == SDLK_UP) cout << "UP" << endl;
    else if(event.key.keysym.sym == SDLK_DOWN) cout << "DOWN" << endl;
    else if(event.key.keysym.sym == SDLK_u) drawRandomTriangle();
    //else if(event.key.keysym.sym == SDLK_f) drawFilledTriangle();
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
