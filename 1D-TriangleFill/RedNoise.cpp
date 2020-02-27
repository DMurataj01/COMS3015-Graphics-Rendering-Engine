#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

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

  //** Draw a line.
  drawLine(CanvasPoint(50, 50), CanvasPoint(50, 100), Colour(255, 0, 0));
  //** Draw a stroked triangle.

  drawStrokedTriangle( CanvasTriangle(CanvasPoint(200, 50), CanvasPoint(100, 50), CanvasPoint(150, 0), Colour(0, 255, 255)) );
  
  drawFilledTriangle( CanvasTriangle(CanvasPoint(50, 0), CanvasPoint(0, 50), CanvasPoint(100, 50), Colour(255, 0, 255)) );
  drawFilledTriangle( CanvasTriangle(CanvasPoint(0, 50), CanvasPoint(50, 0), CanvasPoint(100, 50), Colour(255, 0, 255)) );
  drawFilledTriangle( CanvasTriangle(CanvasPoint(100, 50), CanvasPoint(0, 50), CanvasPoint(50, 0), Colour(255, 0, 255)) );

  //drawFilledTriangle(CanvasTriangle triangle);
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
    
    for (int i = 0; i<numberOfValues; i++){
        vec_list.push_back(from + (i*increment));
    }
    
    return vec_list;
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

void drawLine(CanvasPoint ptStart, CanvasPoint ptEnd, Colour ptClr) {
  float diffX = (ptEnd.x - ptStart.x);
  float diffY = (ptEnd.y - ptStart.y);
  float noOfSteps = glm::max(abs(diffX), abs(diffY));
  cout << "Diff: x " << diffX << " y " << diffY << " max " << noOfSteps << "\n";
  float stepSizeX = diffX/noOfSteps;
  float stepSizeY = diffY/noOfSteps;
  cout << "Step X: " << stepSizeX << " .. Step Y: " << stepSizeY << "\n";
  
  for (int i = 0 ; i < noOfSteps ; i++){
    int x = int(ptStart.x + (i * stepSizeX));
    int y = int(ptStart.y + (i * stepSizeY));

    window.setPixelColour( x, y, getColour(ptClr));
  }

  return;
}

void sortTriangleVertices(CanvasTriangle tri) {
  if (tri.vertices[1].y < tri.vertices[0].y)
    swap(tri.vertices[0], tri.vertices[1]);
  else if (tri.vertices[1].y == tri.vertices[0].y)
      if (tri.vertices[1].x < tri.vertices[0].x)
        //swapVertices(tri, 0, 1);
        swap(tri.vertices[0], tri.vertices[1]);

  if (tri.vertices[2].y < tri.vertices[1].y)
    //swapVertices(tri, 1, 2);
    swap(tri.vertices[1], tri.vertices[2]);
  else if (tri.vertices[2].y == tri.vertices[1].y)
      if (tri.vertices[2].x < tri.vertices[1].x)
        //swapVertices(tri, 1, 2);
        swap(tri.vertices[1], tri.vertices[2]);


  if (tri.vertices[1].y < tri.vertices[0].y)
    //swapVertices(tri, 0, 1);
    swap(tri.vertices[0], tri.vertices[1]);
  else if (tri.vertices[1].y == tri.vertices[0].y)
      if (tri.vertices[1].x < tri.vertices[0].x)
        //swapVertices(tri, 0, 1);
        swap(tri.vertices[0], tri.vertices[1]);
  
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
void drawRandomTriangle() {
  return;
}



void drawFilledTriangle(CanvasTriangle triangle){
  //** Draw outline.
  drawStrokedTriangle(triangle);
  //** Sort Vertices before fill.  
  sortTriangleVertices(triangle);

  /** Guarantee that all the vertices are sorted on Y then X... **/



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
