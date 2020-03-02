#include "OBJStream.cpp"
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

/* Custom Functions */
void drawLine(CanvasPoint ptStart, CanvasPoint ptEnd, Colour ptClr);
void drawStrokedTriangle(CanvasTriangle triangle);
void drawFilledTriangle(CanvasTriangle triangle);
void rasterize(vec3 cameraPosition, mat3 cameraOrientation, vector<ModelTriangle> faces);


DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

vec3 cameraPosition (0,-2,-3.5);

mat3 cameraOrientation (vec3(1,0,0), vec3(0,-1,0), vec3(0,0,-1)); //Camera Right, Up, Forward.
float focalLength = WIDTH / 2;


float depthMap [WIDTH*HEIGHT];
void initializeDepthMap(){
  for (int i = 0 ; i < (HEIGHT*WIDTH) ; i++)
    depthMap[i] = numeric_limits<float>::infinity();
}


int main(int argc, char* argv[]) {
  
  SDL_Event event;
  window.clearPixels();
  initializeDepthMap();

  vector<ModelTriangle> objFaces = readOBJ("cornell-box.obj", "cornell-box.mtl", 1.f);
  rasterize(cameraPosition, cameraOrientation, objFaces);

  //** Draw a stroked triangle.
  //drawStrokedTriangle( CanvasTriangle(CanvasPoint(200, 50), CanvasPoint(100, 50), CanvasPoint(150, 0), Colour(0, 255, 255)) );
  //drawStrokedTriangle( CanvasTriangle(CanvasPoint(50, 0), CanvasPoint(0, 50), CanvasPoint(100, 50), Colour(255, 0, 255)) );
  //** Draw a filled triangle.

  //drawFilledTriangle( CanvasTriangle(CanvasPoint(400, 300), CanvasPoint(300, 150), CanvasPoint(200, 350), Colour(205, 150, 50)) );
  //drawStrokedTriangle( CanvasTriangle(CanvasPoint(400, 300), CanvasPoint(300, 150), CanvasPoint(200, 350), Colour(50, 255, 255)) );

  while(true) {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

vector<CanvasPoint> interpolate(CanvasPoint from, CanvasPoint to, float  numberOfValues) {
  vector<CanvasPoint> vecInterpVectors;

  //Add the first number in.
  vecInterpVectors.push_back(from);
  vec3 castedFrom = vec3(from.x, from.y, from.depth);
  vec3 castedTo = vec3(to.x, to.y, from.y);
  vec3 stepValue = (castedTo - castedFrom) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted
  vec3 previous = castedFrom;

  for (int i = 1; i < numberOfValues ; i++) {
    vec3 input = previous + stepValue;
    vecInterpVectors.push_back(CanvasPoint(input.x, input.y, input.z));
    previous = input;
  }
  return vecInterpVectors;
}

void drawLine(CanvasPoint ptStart, CanvasPoint ptEnd, Colour ptClr) {
  float diffX = (ptEnd.x - ptStart.x);
  float diffY = (ptEnd.y - ptStart.y);
  //float diffZ = (ptEnd.depth - ptStart.depth);
  
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


  vector<CanvasPoint> interpLeft = interpolate(triangle.vertices[0], triangle.vertices[1], noOfRows+1);
  vector<CanvasPoint> interpRight = interpolate(triangle.vertices[0], triangle.vertices[2], noOfRows+1);

  //** Do line by line interp.
  for (int i=0; i<= noOfRows; i++)
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

  vector<CanvasPoint> interpLeft = interpolate(triangle.vertices[0], triangle.vertices[2], noOfRows+1);
  vector<CanvasPoint> interpRight = interpolate(triangle.vertices[1], triangle.vertices[2], noOfRows+1);

  //** Do line by line interp.
  for (int i=0; i<= noOfRows; i++)
    drawLine(CanvasPoint(interpLeft[i].x, triangle.vertices[0].y + i), CanvasPoint(interpRight[i].x, triangle.vertices[0].y + i), triangle.colour);

  return;
}
void drawFilledTriangle(CanvasTriangle triangle){
  
  //** 1. Draw outline.
  //drawStrokedTriangle(triangle);

  //** 2. Sort Vertices before fill.  
  triangle = sortTriangleVertices(triangle);

  //** 3. Get the Cut Point & Fill.
    
  float yDiff = glm::abs(triangle.vertices[2].y - triangle.vertices[0].y);

  if (yDiff == 0.f) {
    //** TODO: Draw a line.
    cout << "Y diff is zero.\n\n";
    drawLine(triangle.vertices[0], triangle.vertices[2], triangle.colour);
  } else {

    /* Get y difference to left_most vertex */
    float minorYDiff2D;
    if (triangle.vertices[0].x < triangle.vertices[2].x) minorYDiff2D = triangle.vertices[1].y - triangle.vertices[0].y;
    else minorYDiff2D = triangle.vertices[2].y - triangle.vertices[1].y;

    float minorYDiff3D;
    if (triangle.vertices[0].depth < triangle.vertices[2].depth) minorYDiff3D = triangle.vertices[1].y - triangle.vertices[0].y;
    else minorYDiff3D = triangle.vertices[2].y - triangle.vertices[1].y;


    float xDiff = glm::abs(triangle.vertices[2].x - triangle.vertices[0].x);
    float zDiff = glm::abs(triangle.vertices[2].depth - triangle.vertices[0].depth);

    float cut_x = glm::min(triangle.vertices[0].x, triangle.vertices[2].x) + ( minorYDiff2D * xDiff/yDiff );
    float cut_z = glm::min(triangle.vertices[0].depth, triangle.vertices[2].depth) + ( minorYDiff3D * zDiff/yDiff );
    
    CanvasPoint cutPoint = CanvasPoint(cut_x, triangle.vertices[1].y, cut_z);

    //** 4.1. Fill the Top Flat Bottom Triangle.
    fillFlatBottomTriangle(CanvasTriangle(triangle.vertices[0], cutPoint, triangle.vertices[1], triangle.colour));
    //** 4.2. Fill the Bottom Flat Top Triangle.
    fillFlatTopTriangle(CanvasTriangle(cutPoint, triangle.vertices[1], triangle.vertices[2], triangle.colour));

  }
  

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
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}

void rasterize(vec3 cameraPosition, mat3 cameraOrientation, vector<ModelTriangle> faces){
  // for each face
  for (int i = 0 ; i < faces.size() ; i++){
    ModelTriangle triangle = faces[i];
    CanvasTriangle canvasTriangle;
    canvasTriangle.colour = triangle.colour;
    
    // Loop through each of vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = triangle.vertices[j];
      
      vec3 vertexCSpace = (cameraOrientation * vertex) - cameraPosition; // change from world coordinates to camera space
      
      // save the depth of this point for later
      float depth = vertexCSpace[2];

      // calculating the projection onto the 2D image plane by using interpolation and the z depth
      // only worth doing if the vertex is in front of camera
      if (depth > 0){
        float proportion = focalLength / depth;
        vec3 vertexProjected = vertexCSpace * proportion; // the coordinates of the 3D point (in camera space) projected onto the image plane
        float x = vertexProjected[0];
        float y = vertexProjected[1];

        float xNormalised = (x / WIDTH) + 0.5;
        float yNormalised = (y / HEIGHT) + 0.5;
        float xPixel = xNormalised * WIDTH;
        float yPixel = yNormalised * HEIGHT;

        //Keep pixels on screen.
        if (xPixel < 0) xPixel = 0;
        else if (xPixel > WIDTH) xPixel = WIDTH;
        if (yPixel < 0) yPixel = 0;
        else if (yPixel > HEIGHT) yPixel = HEIGHT;
        
        // store the pixel values as a Canvas Point and save it for this triangle
        canvasTriangle.vertices[j] = CanvasPoint(xPixel, yPixel, depth); // we save the depth of the 2D point too
        //cout << "Vertex: " << j << " x: " << xPixel << " y: " << yPixel << " depth: " << depth << "\n";
      }
    }
    drawFilledTriangle(canvasTriangle);
  }
}
