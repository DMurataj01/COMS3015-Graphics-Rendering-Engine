// Note that in this code we use a left-handed coordinate system 
 
#ifndef MODELTRIANGLE_H
  #define MODELTRIANGLE_H
  #include <ModelTriangle.h>
#endif

#include <CanvasTriangle.h> 
#include <DrawingWindow.h> 
#include <Utils.h> 
#include <glm/glm.hpp> 
#include <fstream> 
#include <vector> 
#include <sstream> 
#include <RayTriangleIntersection.h> 
#include <Object.h>
 
using namespace std; 
using namespace glm; 
 
#define W 600//2400//640 
#define H 600//2400//480 

const int ssScale = 1;

const int WIDTH = W * ssScale;
const int HEIGHT = H * ssScale;

vector<uint32_t> pixelBuffer; 
vector<float> depthMap;
 
enum MOVEMENT {UP, DOWN, LEFT, RIGHT, ROLL_LEFT, ROLL_RIGHT, PAN_LEFT, PAN_RIGHT, TILT_UP, TILT_DOWN};

enum RENDERTYPE{RAYTRACE, RASTERIZE, WIREFRAME};

RENDERTYPE currentRender = RAYTRACE; //Default to Raytrace.

// press '1' for wireframe 
// press '2' for rasterized 
// press '3' for raytraced  

 
/* STRUCTURE - ImageFile */ 
struct ImageFile { 
  vector<Colour> vecPixelList; 
  int width; 
  int height;   
}; 
 
 
void update(); 
void handleEvent(SDL_Event event); 
void render(); 
void clear(); 
vector<vec3> interpolate(vec3 from, vec3 to, float numberOfValues); 
uint32_t getColour(Colour colour); 
void printVec3(vec3 input); 
void drawLine(CanvasPoint start, CanvasPoint end, Colour colour); 
void drawStrokedTriangle(CanvasTriangle triangle); 
void drawFilledTriangle(CanvasTriangle triangle); 
void readPPM(); 
void createObjects();
vector<Colour> readOBJMTL(string filename); 
string removeLeadingWhitespace(string s); 
ImageFile readImage(string fileName); 
vector<string> separateLine(string inputLine); 
vec3 getVertex(string inputLine); 
ModelTriangle getFace(string inputLine, vector<vec3> vertices, Colour colour, float scalingFactor); 
vector<ModelTriangle> readOBJ(float scalingFactor); 
void rasterize(); 
void initializeDepthMap(); 
void updateView (MOVEMENT movement);
/* FUNCTION Declarations */ 
ImageFile readImage(string fileName); 
void renderImage(ImageFile imageFile); 
void lookAt(vec3 point); 
vec3 findCentreOfScene(); 
void test2(); 
void raytracer(); 
Colour solveLight(RayTriangleIntersection closest, vec3 rayDirection, float Ka, float Kd, float Ks); 
vec3 createRay(int i, int j); 
vector<vec4> checkForIntersections(vec3 point, vec3 rayDirection);
vector<vec4> faceIntersections(vector<ModelTriangle> inputFaces, vec3 point, vec3 rayDirection);
RayTriangleIntersection closestIntersection(vector<vec4> solutions, vec3 rayPoint); 
Colour shootRay(vec3 rayPoint, vec3 rayDirection, int depth, float currentIOR); 
Colour getFinalColour(Colour colour, float Ka, float Kd, float Ks); 
float intensityDropOff(vec3 point); 
vec3 getNormalOfTriangle(ModelTriangle triangle); 
float angleOfIncidence(RayTriangleIntersection intersection); 
float distanceVec3(vec3 from, vec3 to); 
bool checkForShadows(vec3 point); 
float calculateSpecularLight(vec3 point, vec3 rayDirection, vec3 normal);
float softShadows(RayTriangleIntersection intersection);
void averageVertexNormals(); 
Colour mirror(RayTriangleIntersection intersection, vec3 incident);
Colour glass(vec3 rayDirection, RayTriangleIntersection closest, int depth);
vec4 refract(vec3 I, vec3 N, float ior);
float fresnel(vec3 incident, vec3 normal, float ior);
void backfaceCulling(vec3 cameraPosition);
vector<ModelTriangle> boundingBox(vector<ModelTriangle> inputFaces);
 
 
 
 
DrawingWindow window;// = DrawingWindow(WIDTH, HEIGHT, false); 
 
 
// the files (scene) which we want to render 
string objFileName = "cornell-box.obj"; 
string mtlFileName = "cornell-box.mtl"; 
 
// this is where we will store the faces of the OBJ file 
vector<ModelTriangle> faces;
// this stores the faces split up into separate objects
vector<Object> objects;
 
  

bool print = false; 
 
 
// initial camera parameters 
vec3 cameraPosition (0,2,3.5);//(0,-2,-3.5); 
vec3 cameraRight (1,0,0); 
vec3 cameraUp (0,1,0);//(0,-1,0); 
vec3 cameraForward (0,0,1);//(0,0,-1); // this is actually backwards 
mat3 cameraOrientation (cameraRight, cameraUp, cameraForward); // this creates a matrix with each entry as separate columns 
//float focalLength = WIDTH / 2; 
float focalLength = 1; 
float imageWidth = 2; // WIDTH 
float imageHeight = 2; // HEIGHT 
 
 
// light parameters 
vec3 lightPosition (-0.234011, 5, -3); // this is roughly the centre of the white light box 
float lightIntensity = 100; 
 
 
 
int main(int argc, char* argv[]) { 
  // 1) Create New Display.
  window = DrawingWindow(W, H, false); 
  window.clearPixels();

  // 2) Initialise Depth Map.
  for (int i=0; i< WIDTH*HEIGHT; i++) {
    pixelBuffer.push_back(0);
    depthMap.push_back(std::numeric_limits<float>::infinity());
  }

  // 3) Read In OBJ.
  faces = readOBJ(1);

  // 4) Create textures
  // mirrored floor
  //faces[6].texture = "mirror"; 
  //faces[7].texture = "mirror";
  // making the 'red' box glass
  for (int i = 12 ; i < 22 ; i++){
    faces[i].texture = "glass";
  }
  
  // 5) Split the faces into objects
  createObjects();
  
  // 6) Run set-up funtions for lighting and culling
  averageVertexNormals();
  backfaceCulling(cameraPosition);

  // 7) Render the image
  render();
  SDL_Event event; 
  while(true) 
  { 
    // We MUST poll for events - otherwise the window will freeze ! 
    if(window.pollForInputEvents(&event)) handleEvent(event); 
    update(); 
 
    // Need to render the frame at the end, or nothing actually gets shown on the screen ! 
    window.renderFrame(); 
  } 
} 
 
 
 
void update() 
{ 
  // Function for performing animation (shifting artifacts or moving the camera) 
} 

void SetBufferColour(int x, int y, uint32_t col) {
  const int i = x + (y*WIDTH);
  pixelBuffer[i] = col;
}


uint8_t getRedValueFromColor(uint32_t c) {
  return (c >> 16);
}
uint8_t getGreenValueFromColor(uint32_t c) {
    return (c >> 8);
}
uint8_t getBlueValueFromColor(uint32_t c) {
    return (c);
}

 
// this function renders the scene, depending on what the value of STATE is (so whether we use wireframe, rasterize or raytrace) 
void render(){
  cout << "\nright: ";
  printVec3(cameraRight);
  cout << "up: ";
  printVec3(cameraUp);
  cout << "forward: ";
  printVec3(cameraForward);
  cout << endl;

  clear();

  switch (currentRender) {
    case RAYTRACE:
      raytracer();
      break;
    case RASTERIZE:
      rasterize();
      break;
    default:
      rasterize();
      break;
  }

  /* pixelBuffer ---> Display */

  for (int j=0; j<HEIGHT; j+=ssScale) {
    for (int i=0; i<WIDTH; i+=ssScale) {
      /* average [ i j ||  i+1 j ||  i j+1 || i+1 j+1 ] per channel. */
      
      //ssScale * ssScale Block.
      int R=0; int G=0; int B=0;
      for (int jj=0; jj<ssScale; jj++){
        for (int ii=0; ii<ssScale; ii++){
          int x = i + ii; 
          int y = j + jj;
          int index = x + (WIDTH*y);
          R += getRedValueFromColor(pixelBuffer[index]);
          G += getGreenValueFromColor(pixelBuffer[index]);
          B += getBlueValueFromColor(pixelBuffer[index]);

          //cout << "i j ii jj [RGB]: " << i << " " << j << " " << " " << ii << " " << jj << "[ " << R << "," << G << "," << B << "]\n";
        }
      }
      R /= (ssScale*ssScale);
      G /= (ssScale*ssScale);
      B /= (ssScale*ssScale);
      uint32_t colour = (255<<24) + (int(R)<<16) + (int(G)<<8) + int(B); 
      window.setPixelColour(i/ssScale, j/ssScale, colour);
    }
  }

  //uint32_t col = (255<<24) + (colour.red<<16) + (colour.green<<8) + colour.blue; 
  //return col; 

} 
 
void handleEvent(SDL_Event event) { 
  if(event.type == SDL_KEYDOWN) { 
    if(event.key.keysym.sym == SDLK_LEFT)       updateView(LEFT);  
    else if(event.key.keysym.sym == SDLK_RIGHT) updateView(RIGHT);
    else if(event.key.keysym.sym == SDLK_UP)    updateView(UP);
    else if(event.key.keysym.sym == SDLK_DOWN)  updateView(DOWN);  
    else if(event.key.keysym.sym == SDLK_a)     updateView(PAN_RIGHT);
    else if(event.key.keysym.sym == SDLK_s)     updateView(PAN_LEFT);
    else if(event.key.keysym.sym == SDLK_w)     updateView(TILT_DOWN); 
    else if(event.key.keysym.sym == SDLK_z)     updateView(TILT_UP); 
    else if(event.key.keysym.sym == SDLK_y)     test2(); 
    else if(event.key.keysym.sym == SDLK_c)     clear(); 

    // pressing 1 changes to wireframe mode 
    else if(event.key.keysym.sym == SDLK_1){ 
      currentRender = WIREFRAME;
      render(); 
    } 
    // pressing 2 changes to rasterize mode 
    else if(event.key.keysym.sym == SDLK_2){ 
      currentRender = RASTERIZE;
      render(); 
    } 
    // pressing 3 changes to raytrace mode 
    else if(event.key.keysym.sym == SDLK_3){ 
      currentRender = RAYTRACE;
      render(); 
    } 
  } 
} 
 
 
 
void clear(){ 
  window.clearPixels(); 
  for (int i = 0 ; i < (HEIGHT*WIDTH) ; i++){ 
    pixelBuffer[i] = 0;
    depthMap[i] = numeric_limits<float>::infinity(); 
  } 
} 
 



void setDepthPixelColour(int x, int y, double z, uint32_t clr) { 
  int index = (WIDTH*(y)) + x; 
  if (x > 0 && y > 0 ) {
    if (x < WIDTH && y< HEIGHT) {
      if (z < depthMap.at(index)){
        SetBufferColour(x,y, clr);
        depthMap.at(index) = z;
      }
    }
  }
};


// use this function to split the faces into separate objects (an 'Object' object has been created)
// if you want, you can store a bounding box with the object too
void createObjects(){
  Object object1;
  object1.faces = faces;
  //object1.hasBoundingBox = true;
  //object1.boxFaces = boundingBox(object1.faces);

  objects.push_back(object1);
}
 
 
 
void textures(){ 
  // mirrored floor
  //faces[6].texture = "mirror"; 
  //faces[7].texture = "mirror";
  // making the 'red' box glass
  for (int i = 12 ; i < 22 ; i++){
    faces[i].texture = "glass";
  }
} 
 
 
// given a start and an end value, step from the start to the end with the right number of steps 
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
 
 
 
// given a colour in (R,G,B) format, puts it into binary form in a single string (ASCII) 
uint32_t getColour(Colour colour){ 
  uint32_t col = (255<<24) + (colour.red<<16) + (colour.green<<8) + colour.blue; 
  return col; 
} 
 
 
// draws a 2D line from start to end (colour is in (r,g,b) format with 255 as max) 
void drawLine(CanvasPoint start, CanvasPoint end, Colour colour) { 
  float diffX = end.x - start.x; 
  float diffY = end.y - start.y; 
  float numberOfSteps = glm::max(abs(diffX),abs(diffY)); 
 
  // getting the colour in a single string 
  uint32_t col = getColour(colour); 
 
  // if we are starting and ending on the same pixel 
  if (numberOfSteps == 0){ 
    //SetBufferColour(start.x, start.y, col); 
  } 
 
  else { 
    float stepSizeX = diffX / numberOfSteps; 
    float stepSizeY = diffY / numberOfSteps; 
 
    // for each pixel across 
    for (int i = 0 ; i <= numberOfSteps ; i++){ 
       
      int x = round(start.x + (i * stepSizeX)); 
      int y = round(start.y + (i * stepSizeY)); 
      int index = (WIDTH*(y)) + x; // the index for the position of this pixel in the depth map 
     
      // interpolate to find the current depth of the line 
      float proportion = i / numberOfSteps; 
      float inverseDepth = ((1 - proportion) * (1 / start.depth)) + (proportion * (1 / end.depth)); // got this equation from notes 
      float depth = 1 / inverseDepth; 
      //cout << "X: " << x << " Y: " << y << ".. index: " << index << "\n"; 
      // only set the pixel colour if it is the closest object to the camera 
      if (x > 0 && y > 0 ) { 
        if (x < WIDTH && y< HEIGHT) { 
          if (depth < depthMap[index]){ 
            SetBufferColour(x,y,col); 
            depthMap[index] = depth; 
          } 
        } 
      } 
    } 
  } 
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

// draws an unfilled triangle with the input points as vertices 
void drawStrokedTriangle(CanvasTriangle triangle){ 
  CanvasPoint point1 = triangle.vertices[0]; 
  CanvasPoint point2 = triangle.vertices[1]; 
  CanvasPoint point3 = triangle.vertices[2]; 
  Colour colour = triangle.colour; 
 
  drawLine(point1,point2,colour); 
  drawLine(point2,point3,colour); 
  drawLine(point3,point1,colour); 
} 
 
 
 
void drawFilledTriangle(CanvasTriangle triangle){ 
  //Sort vertices.
  triangle = sortTriangleVertices(triangle);
  
  CanvasPoint maxPoint = triangle.vertices[0]; 
  CanvasPoint middlePoint = triangle.vertices[1]; 
  CanvasPoint minPoint = triangle.vertices[2]; 
  // the vertical distance between the highest point and the middle point 
  float yDistance = middlePoint.y - maxPoint.y; 
  // interpolating to find the cutting point 
  float maxYDistance = minPoint.y - maxPoint.y; 
  float maxXDistance = minPoint.x - maxPoint.x; 
  float yProportion = yDistance / maxYDistance; // the proportion of how far along in the y the point should be 
  float xDistance = maxPoint.x + (yProportion * maxXDistance); 
   
  // interpolate to find the right depth too 
  // because of perspective projection we interpolate using 1/depth of everything 
  float inverseDepth = ((1 - yProportion) * (1 / maxPoint.depth)) + (yProportion * (1 / minPoint.depth)); // equation from notes 
  float cutterDepth = (1 / inverseDepth); 
  CanvasPoint cutterPoint(xDistance, maxPoint.y + yDistance, cutterDepth); 
 
  // the upper triangle 
  // for each row, fill it in 
  float steps = middlePoint.y - maxPoint.y; // how many rows 
  // if the two vertices are on the same y line, then just draw a line between the two 
  if (steps == 0){ 
    drawLine(middlePoint, maxPoint, triangle.colour); 
  } 
  else { 
    for (int i = 0 ; i < steps ; i++){ 
      // find the two points which intersect this row 
      float proportion = i / steps; 
      float maxXDiff1 = maxPoint.x - middlePoint.x; 
      float maxXDiff2 = maxPoint.x - cutterPoint.x; 
      float xStart = round(maxPoint.x - (proportion * maxXDiff1)); 
      float xEnd = round(maxPoint.x - (proportion * maxXDiff2)); 
       
      // interpolating to find the right depth of both of the points 
      // point1: 
      float inverseDepthPoint1 = ((1 - proportion) * (1 / maxPoint.depth)) + (proportion * (1 / middlePoint.depth)); 
      float depthPoint1 = 1 / inverseDepthPoint1; 
      // point2: 
      float inverseDepthPoint2 = ((1 - proportion) * (1 / maxPoint.depth)) + (proportion * (1 / cutterPoint.depth)); 
      float depthPoint2 = 1 / inverseDepthPoint2; 
      CanvasPoint start(xStart, maxPoint.y + i, depthPoint1); 
      CanvasPoint end(xEnd, maxPoint.y + i, depthPoint2); 
      drawLine(start, end, triangle.colour); 
    } 
  } 
   
  // the lower triangle 
  // for each row, fill it in 
  float steps2 = minPoint.y - middlePoint.y; // how many rows 
  if (steps2 == 0) { 
    drawLine(minPoint, middlePoint, triangle.colour); 
  } 
  else { 
    for (int i = steps2 ; i >= 0 ; i--){ 
      // find the two points which intersect this row 
      float proportion = 1 - (i / steps2); // proportion going upwards from the min point to the middle 
      float maxXDiff1 = minPoint.x - middlePoint.x; 
      float maxXDiff2 = minPoint.x - cutterPoint.x; 
      float xStart = round(minPoint.x - (proportion * maxXDiff1)); 
      float xEnd = round(minPoint.x - (proportion * maxXDiff2)); 
       
      // interpolating to find the right depth 
      // point1: 
      float inverseDepthPoint1 = ((1 - proportion) * (1 / minPoint.depth)) + (proportion * (1 / middlePoint.depth)); 
      float depthPoint1 = 1 / inverseDepthPoint1; 
      // point2: 
      float inverseDepthPoint2 = ((1 - proportion) * (1 / minPoint.depth)) + (proportion * (1 / cutterPoint.depth)); 
      float depthPoint2 = 1 / inverseDepthPoint2; 
      CanvasPoint start(xStart, cutterPoint.y + i, depthPoint1); 
      CanvasPoint end(xEnd, cutterPoint.y + i, depthPoint2); 
      drawLine(start, end, triangle.colour); 
    } 
  } 
  /* 
  // this code draws the outline of the triangle ontop of the filled triangle to make sure it is correct 
  drawLine(point1,point2,Colour (255,255,255)); 
  drawLine(point2,point3,Colour (255,255,255)); 
  drawLine(point3,point1,Colour (255,255,255)); 
  drawLine(points[1],cutterPoint,Colour (255,255,255)); 
  */ 
}  
 
void readPPM(){ 
  // open the file, we then go through each line storing it in a string 
  string line; 
  ifstream myfile("texture.ppm"); 
  if (myfile.is_open()) 
  { 
    // the first 4 lines contain information about the image 
    for (int i = 0 ; i < 4 ; i++){ 
      getline(myfile, line); 
      cout << line << endl; 
 
      // the 3rd line defines the width and height 
      if (i == 2){ 
        // find where the space splits the numbers between the height and width 
        int n = line.length(); 
        int index = line.find(' '); 
        int width = stoi(line.substr(0,index)); // stoi turns a string containing a number into an int 
        int height = stoi(line.substr(index+1,n)); // the numbers after the space are the height 
        cout << width << height << "\n"; 
      } 
 
      // the 4th line contains the max value used for the rgb values 
      if (i == 3){ 
        int maxValue = stoi(line); 
        cout << maxValue << "\n"; 
      } 
    } 
 
    // now get all the rest of the bytes 
    char red, green, blue; 
    for (int i = 0 ; i < 1000 ; i++){ 
      red = myfile.get(); 
      green = myfile.get(); 
      blue = myfile.get(); 
      uint32_t a = getColour(Colour (red,green,blue)); 
      cout << a << "\n"; 
    } 
 
    myfile.close(); 
  } 
 
  else cout << "Unable to open file";  
} 
 
 
 
// this function reads in an OBJ material file and stores it in a vector of Colour objects, so an output may look like: 
// [['Red','1','0','0'] , ['Green','0','1','0']] 
vector<Colour> readOBJMTL(string filename){ 
  // store the colours in a vector of Colours 
  vector<Colour> colours; 
  // open the file, we then go through each line storing it in a string 
  string line; 
  ifstream myfile(filename); 
   
  if (myfile.is_open()){ 
    while (getline(myfile,line)){ 
      // if we have a new material, store it 
      if (line.find("newmtl") == 0){ 
        Colour colour; 
        vector<string> lineVector = separateLine(line); 
        colour.name = lineVector[1]; 
        // get new line to store the values 
        getline(myfile,line); 
        vector<string> values = separateLine(line); 
        colour.red = stof(values[1]) * 255; 
        colour.green = stof(values[2]) * 255; 
        colour.blue = stof(values[3]) * 255; 
        colours.push_back(colour); 
      } 
    } 
  } 
  else cout << "Unable to open file"; 
 
  return colours; 
} 
 
 
 
string removeLeadingWhitespace(string s){ 
  s.erase(0, s.find_first_not_of(" ")); 
  return s; 
} 
 
 
ImageFile readImage(string fileName) { 
 
  std::ifstream ifs; 
  ifs.open ("texture.ppm", std::ifstream::in); 
 
  /* Parse Header */ 
 
  //Check if header is a P6 file. 
  string headerInput = ""; 
  getline(ifs, headerInput); 
 
  if (headerInput != "P6") { 
    cout << "Error - Header file is invalid"; 
    ifs.close(); 
    throw 1; 
  } 
 
  int width = -1; 
  int height = -1; 
  int maxvalue = -1; 
 
  /* Following Specification: http://netpbm.sourceforge.net/doc/ppm.html */  
  // 1) Check if header is a P6 file. 
  // 2) Ignore Comments. 
  // 3) Parse Width + whitespace + Height. 
  // 4) Parse Max value 
 
  while (true || !ifs.eof()) { 
    string inputLine = ""; 
    getline(ifs,inputLine); 
    inputLine = removeLeadingWhitespace(inputLine); 
    if (inputLine[0] == '#'){ 
      //This is a comment line -> ignore them. 
    } 
    else { 
      //Parse Width + Height. 
      stringstream ss_wh(inputLine); 
      ss_wh >> width >> height; 
       
      //Read new line -> Parse Max value: // 0<val<65536. 
      getline(ifs,inputLine); 
      inputLine = removeLeadingWhitespace(inputLine); 
       
      stringstream ss_ml(inputLine); 
      ss_ml >> maxvalue; 
 
      cout << "\nHeader Parse --  Width: " << width << " -- Height: " << height << " -- Max Value: " << maxvalue << "\n"; 
      break; 
    } 
  } 
   
  /* Body RGB Parse */ 
 
  vector<Colour> vecPixelList; //RGB storage. 
 
  while (ifs.peek()!= EOF) { 
    vecPixelList.push_back(Colour ({ifs.get(), ifs.get(), ifs.get()})); //Create a Pixel element with three consecutive byte reads. 
  } 
  cout << "\nBody Parse -- Number of elements: " << vecPixelList.size() << "\n"; 
  ifs.close(); 
  ImageFile outputImageFile = ImageFile ({vecPixelList, width, height}); 
  return outputImageFile; 
} 
 
 
 
// given a line, say 'v 12 3 5', it retreives the values inbetween the spaces, outputting them as strings in a vector 
vector<string> separateLine(string inputLine){ 
  inputLine = inputLine + " "; // we add a space at the end to we know when to end; 
  vector<string> output; 
  int n = inputLine.length(); 
  int spaceIndex = inputLine.find(' '); 
  // if we have a space in the line still 
  while ((spaceIndex > 0) && (spaceIndex <= n)){ 
    string value = inputLine.substr(0, spaceIndex); 
    output.push_back(value); 
    inputLine = inputLine.substr(spaceIndex + 1, n); 
    n = inputLine.length(); 
    spaceIndex = inputLine.find(' '); 
  } 
  return output; 
} 
 
 
 
// given a string in form 'v 12 5 4' it returns (12,5,4) 
vec3 getVertex(string inputLine){ 
  vector<string> points = separateLine(inputLine); 
  vec3 output; 
  output[0] = stof(points[1]); // it is points[1] as the first element is 'v' 
  output[1] = stof(points[2]); 
  output[2] = stof(points[3]); 
  return output; 
} 
 
 
 
// given a line in the form 'f 1/1 2/2 3/3' and also all vertices found so far  
// it returns a ModelTriangle object, which contains the 3 vertices and the colour too 
ModelTriangle getFace(string inputLine, vector<vec3> vertices, Colour colour, float scalingFactor){ 
  // initialise the output 
  ModelTriangle output; 
  output.colour = colour; 
 
  vector<string> faces = separateLine(inputLine); // this turns 'f 1/1 2/2 3/3' into ['f','1/1','2/2','3/3'] 
  for (int i = 1 ; i < 4 ; i++){ 
    string element = faces[i]; // this will be '1/1' for example 
    int slashIndex = element.find('/'); 
    string number = element.substr(0, slashIndex); 
    int index = stoi(number); 
    vec3 vertex = vertices[index-1]; // -1 as the vertices are numbered from 1, but c++ indexes from 0 
    vertex = vertex * scalingFactor; 
    output.vertices[i-1] = vertex; // the -1 here comes from the face that we skip the 'f' in the vector and so i starts at 1 
  } 
 
  return output; 
} 
 
 
 
vector<ModelTriangle> readOBJ(float scalingFactor){ 
  // get the colours 
  vector<Colour> colours = readOBJMTL(mtlFileName); 
 
  // where we store all the vertices and faces 
  vector<vec3> vertices; 
  vector<ModelTriangle> faces; 
   
  // open the obj file and then go through each line storing it in a string 
  string line; 
  ifstream myfile(objFileName); 
 
  // if we cannot open the file, print an error 
  if (myfile.is_open() == 0){ 
    cout << "Unable to open file" << "\n"; 
  } 
 
  // this is where we will save the correct colour for each face once found 
  Colour colour (255,255,255); 
 
  int numberOfFaces = 0;
  // while we have a new line, get it 
  while (getline(myfile, line)){ 
    // if the line starts with 'usemtl', then we retrieve the new colour and use this colour until we get a new one 
    if (line.find("usemtl") == 0){ 
      vector<string> colourVector = separateLine(line); 
      string colourName = colourVector[1]; 
       
      // now go through each of the colours we have saved until we find it 
      int n = colours.size(); 
      for (int i = 0 ; i < n ; i++){ 
        Colour col = colours[i]; 
        if (colours[i].name == colourName){ 
          colour = colours[i]; 
          break; // can stop once we have found it 
        } 
      } 
    } 
 
    // if we have a vertex, then put it in a vec3 and store it with all other vertices 
    else if (line.find('v') == 0){ 
      vec3 vertex = getVertex(line); 
      vertex = vertex * scalingFactor; 
      vertices.push_back(vertex); 
    } 
 
    // if we have a face, then get the corresponding vertices and store it as a ModelTriangle object, then add it to the collection of faces 
    else if (line.find('f') == 0){ 
      ModelTriangle triangle = getFace(line, vertices, colour, scalingFactor);
      triangle.faceIndex = numberOfFaces;
      numberOfFaces = numberOfFaces + 1;
      faces.push_back(triangle); 
    } 
  } 
  //cout << "Printing the faces: \n"; 
  //for (int i = 0 ; i < faces.size() ; i++){ 
  //  cout << faces[i] << "\n"; 
  //} 
  return faces; 
} 
 
 
 
 
void rasterize(){  
  // for each face 
  for (int i = 0 ; i < faces.size() ; i++){ 
    ModelTriangle triangle = faces[i]; 
    CanvasTriangle canvasTriangle; 
    canvasTriangle.colour = triangle.colour; 
     
    // for each vertex 
    for (int j = 0 ; j < 3 ; j++){ 
      vec3 vertex = triangle.vertices[j]; 
       
      // change from world coordinates to camera

      vec3 col1 (cameraRight[0], cameraUp[0], cameraForward[0]);
      vec3 col2 (cameraRight[1], cameraUp[1], cameraForward[1]);
      vec3 col3 (cameraRight[2], cameraUp[2], cameraForward[2]);
      mat3 rotationMatrix (col1, col2, col3);

      vec3 vertexCSpace = rotationMatrix * vertex;
      vertexCSpace = vertexCSpace - cameraPosition;
      vertexCSpace[2] = -vertexCSpace[2];

       
      // save the depth of this point for later 
      float depth = vertexCSpace[2]; 
 
      // calculating the projection onto the 2D image plane by using interpolation and the z depth 
      // only worth doing if the vertex is in front of camera 
      if ((depth < 0) || (depth > 0)){ 
        float proportion = focalLength / depth; 
        vec3 vertexProjected = vertexCSpace * proportion; // the coordinates of the 3D point (in camera space) projected onto the image plane 
        //float x = vertexProjected[0]; 
        //float y = vertexProjected[1]; 

        // converting to get the pixel numbers
        // finding position of top left corner of image plane in camera space
        vec3 topLeft = vec3 (-imageWidth/2, imageHeight/2, focalLength);//(focalLength * cameraForward) + ((imageHeight/2) * cameraUp) - ((imageWidth/2) * cameraRight);
        vec3 topLeftToPoint = vertexProjected - topLeft;
        float xPixel = (topLeftToPoint[0] / imageWidth) * WIDTH;
        float yPixel = (-topLeftToPoint[1] / imageHeight) * HEIGHT;
 
        /* 
        // if the pixel goes off screen 
        if (xPixel < 0){ 
          xPixel = 0; 
        } 
        else if (xPixel > WIDTH){ 
          xPixel = WIDTH; 
        } 
        if (yPixel < 0){ 
          yPixel = 0; 
        } 
        else if (yPixel > HEIGHT){ 
          yPixel = HEIGHT; 
        } 
        */ 
 
        // store the pixel values as a Canvas Point and save it for this triangle 
        canvasTriangle.vertices[j] = CanvasPoint(xPixel, yPixel, depth); // we save the depth of the 2D point too 
 
      } 
    } 

    if (currentRender == WIREFRAME) drawStrokedTriangle(canvasTriangle); 
    else drawFilledTriangle(canvasTriangle); 
     
  } 
} 
 
 
 
void initializeDepthMap(){ 
  for (int i = 0 ; i < (HEIGHT*WIDTH) ; i++){ 
    depthMap[i] = numeric_limits<float>::infinity(); 
  } 
} 
 
 
 
void updateView (MOVEMENT movement) {

  float alpha, beta, gamma;
  vec3 col1, col2, col3;

  bool move = false;
  bool rotate = false;

  switch (movement) {
    /* Camera Position = Camera Position + Unit Vector */
    case UP: 
      cameraPosition += cameraUp;
      move = true;
      break;
    case DOWN:
      cameraPosition -= cameraUp;
      move = true;
      break;
    case RIGHT: 
      cameraPosition += cameraRight;
      move = true;
      break;
    case LEFT: 
      cameraPosition -= cameraRight;
      move = true;
      break;
    /* Camera Orientation = Camera Orientation * rotation */
    case ROLL_LEFT:
      alpha = 0.5; 
      col1 = vec3 (cos(alpha), sin(alpha), 0); 
      col2 = vec3 (-sin(alpha), cos(alpha), 0); 
      col3 = vec3 (0, 0, 1);
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;
    case ROLL_RIGHT:
      alpha = -0.5; 
      col1 = vec3 (cos(alpha), sin(alpha), 0); 
      col2 = vec3 (-sin(alpha), cos(alpha), 0); 
      col3 = vec3 (0, 0, 1);
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;
    case PAN_LEFT:
      beta = -0.5; 
      col1 = vec3 (cos(beta), 0, -sin(beta)); 
      col2 = vec3 (0, 1, 0); 
      col3 = vec3 (sin(beta), 0, cos(beta));
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;
    case PAN_RIGHT:
      beta = 0.5; 
      col1 = vec3 (cos(beta), 0, -sin(beta)); 
      col2 = vec3 (0, 1, 0); 
      col3 = vec3 (sin(beta), 0, cos(beta));
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;      
    case TILT_UP:
      gamma = -0.5; 
      col1 = vec3 (1, 0, 0); 
      col2 = vec3 (0, cos(gamma), sin(gamma)); 
      col3 = vec3 (0, -sin(gamma), cos(gamma));
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;
    case TILT_DOWN:
      gamma = 0.5; 
      col1 = vec3 (1, 0, 0); 
      col2 = vec3 (0, cos(gamma), sin(gamma)); 
      col3 = vec3 (0, -sin(gamma), cos(gamma));
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;
  }

  if (rotate) {
    mat3 rotation (col1, col2, col3); 
    cameraOrientation = cameraOrientation * rotation; 
    // getting the camera directions from the orientation matrix
    cameraRight = cameraOrientation[0];
    cameraUp = cameraOrientation[1];
    cameraForward = cameraOrientation[2];
  }


  // if the camera position has changed we need to redo the culling of faces
  if (move){
    int n = faces.size();
    for (int i = 0 ; i < n ; i++){
      // reset the faces
      faces[i].culled = false;
    }
    // cull them
    backfaceCulling(cameraPosition);
  }

  render();
} 
 
 
 
void lookAt(vec3 point){
  cout << "\n\nBEFORE:\n";
  cout << "Forward: "; 
  printVec3(cameraForward); 
  cout << "Up: "; 
  printVec3(cameraUp); 
  cout << "Right: ";
  printVec3(cameraRight); 
 
  vec3 direction = cameraPosition - point;
  direction = normalize(direction);
  cameraForward = direction; 
  vec3 randomVector (0,1,0); 
  cameraRight = glm::cross(randomVector, cameraForward); 
  cameraUp = glm::cross(cameraForward, cameraRight); 
  
  cout << "Forward: "; 
  printVec3(cameraForward); 
  cout << "Up: "; 
  printVec3(cameraUp); 
  cout << "Right: "; 
  printVec3(cameraRight);

  cameraForward = normalize(cameraForward); 
  cameraRight = normalize(cameraRight); 
  cameraUp = normalize(cameraUp); 
 
  cout << "\nAFTER\n";
  cout << "Forward: "; 
  printVec3(cameraForward); 
  cout << "Up: "; 
  printVec3(cameraUp); 
  cout << "Right: "; 
  printVec3(cameraRight);
   
  cameraOrientation = mat3 (cameraRight, cameraUp, cameraForward); 
 
  render(); 
} 
 
// this function averages all the vertices in the scene to find the centre of the scene 
vec3 findCentreOfScene(){ 
  int n = faces.size(); 
  vec3 sum (0,0,0); 
  // for each face 
  for (int i = 0 ; i < n ; i++){ 
    ModelTriangle face = faces[i]; 
    // for each vertex 
    for (int j = 0 ; j < 3 ; j++){ 
      sum = sum + face.vertices[j]; 
    } 
  } 
  sum = sum / (float)(n*3); 
  return sum; 
} 
  
void printVec3(vec3 name){ 
  cout << "[" << name[0] << ", " << name[1] << ", " << name[2] << "]\n"; 
} 
 
void test2(){ 
  vec3 centre = findCentreOfScene(); 
  cout << "centre: ";
  printVec3(centre); 
  lookAt(centre); 
} 
 
 
void renderImage(ImageFile imageFile){ 
 
  for (int i=0; i<imageFile.vecPixelList.size(); i++){ 
    int red = imageFile.vecPixelList.at(i).red; 
    int green = imageFile.vecPixelList.at(i).green; 
    int blue = imageFile.vecPixelList.at(i).blue; 
    uint32_t colour = (255<<24) + (red<<16) + (green<<8) + blue; 
    int row = int(i/imageFile.width); 
    int col = i - (row*imageFile.width); 
    SetBufferColour(col, row, colour); 
  } 
  cout << "\nImage render complete.\n"; 
} 
 
 
 
 
 
 
 
//////////////////////////////////////////////////////// 
// RAYTRACING CODE 
//////////////////////////////////////////////////////// 
  
// this will be the main function for the raytracer 
void raytracer(){
  textures(); 
  // for each pixel 
  #pragma omp parallel for
  for (int i = 0 ; i < WIDTH ; i++){ 
    #pragma omp simd
    for (int j = 0 ; j < HEIGHT ; j++){
      // create a ray 
      vec3 rayDirection = createRay(i,j);
      // shoot the ray and check for intersections 
      Colour colour = shootRay(cameraPosition, rayDirection, 0, 1); // depth starts at 0, IOR is 1 as travelling in air
      // colour the pixel accordingly 
      SetBufferColour(i, j, getColour(colour)); 
    } 
  } 
} 
 
 
 
Colour solveLight(RayTriangleIntersection closest, vec3 rayDirection, float Ka, float Kd, float Ks){ 
  Colour colour = closest.intersectedTriangle.colour; 
  vec3 point = closest.intersectionPoint; 
   
  // diffuse light 
  float diffuseIntensity = intensityDropOff(point) * angleOfIncidence(closest); 
  Kd = Kd * diffuseIntensity; 
   
  // specular light 
  vec3 normal = closest.normal; // this is the interpolated noral for Phong shading (it is previously calculated and stored in the RayTriangleIntersection object) 
  float specularIntensity = calculateSpecularLight(point, rayDirection, normal); 
  Ks = Ks * specularIntensity; 
 
  return getFinalColour(colour, Ka, Kd, Ks); 
} 
 
 
 
// given the pixel coordinates, this function calculates the direction of the ray (in vector format) 
vec3 createRay(int i, int j){
  vec2 pixel (i,j); 
  pixel = pixel + vec2(0.5,0.5); 
  vec2 distanceFromCentre = pixel - vec2(WIDTH/2, HEIGHT/2); // distance from the pixel to the centre of the image plane in terms of number of pixels 
  vec2 pixelSize = vec2 (imageWidth/WIDTH , imageHeight / HEIGHT); // the size of a pixel in world coordinates 
  float horizontalDistance = distanceFromCentre[0] * pixelSize[0]; 
  float verticalDistance = distanceFromCentre[1] * pixelSize[1]; 
  vec3 imagePlaneCentre = cameraPosition - (focalLength * cameraForward); // this is the 3D coordinate of the centre of the image plane 
  vec3 point = imagePlaneCentre + (horizontalDistance * cameraRight) + (verticalDistance * (-cameraUp)); 
  vec3 direction = point - cameraPosition; 
  direction = normalize(direction); 
  return direction; 
} 
 
 
// this function takes the ray and checks with all the faces to see which ones it intersects 
// it outputs a vector of vectors - a (t,u,v) vector for each face 
// if t>0, then that means that for that particular face the ray intersects it with u and v being local coordinates for the triangle 
vector<vec4> checkForIntersections(vec3 point, vec3 rayDirection){ 
  // this is the output vector, for every possible face it stores a possibleSolution 
  // the 4th value in this evctor is the index of the face if there is an intersection
  vector<vec4> solutions;

  // for each object, does it have a bounding box?
  // if so, does the ray intersect it?
  // if so, which of the faces in that object does the ray intersect?
  int n = objects.size();
  for (int i = 0 ; i < n ; i++){
    vector<vec4> objectSolutions; // to store the solutions if any for this object
    Object object = objects[i];
    if (object.hasBoundingBox){
      vector<vec4> boxSolutions = faceIntersections(object.boxFaces, point, rayDirection);
      // do we intersect the bounding box?
      for (int j = 0 ; j < boxSolutions.size() ; j++){
        vec4 solution = boxSolutions[j];
        // if we have an intersection, then check for intersections with all the faces
        if (solution[0] > 0){
          objectSolutions = faceIntersections(object.faces, point, rayDirection);
          break;
        }
      }
    }
    // if this object doesn't have a bounding box, then just check each face as normal
    else {
      objectSolutions = faceIntersections(object.faces, point, rayDirection);
    }
    // for all the object solutions (if there is any, put each solution into the main solutions)
    int len = objectSolutions.size();
    for (int j = 0 ; j < len ; j++){
      solutions.push_back(objectSolutions[j]);
    }
  }
  return solutions;
} 


vector<vec4> faceIntersections(vector<ModelTriangle> inputFaces, vec3 point, vec3 rayDirection){
  // this is the output vector, for every possible face it stores a possibleSolution 
  vector<vec4> solutions;

  // for each face 
  int n = inputFaces.size(); 
  for (int i = 0 ; i < n ; i++){ 
    ModelTriangle triangle = inputFaces[i];
    // only check for intersections on the faces that face the camera
    if (triangle.culled == false) {
      // got the following code from the worksheet 
      vec3 e0 = triangle.vertices[1] - triangle.vertices[0]; 
      vec3 e1 = triangle.vertices[2] - triangle.vertices[0]; 
      vec3 SPVector = point - triangle.vertices[0]; 
      mat3 DEMatrix(-rayDirection, e0, e1); 
      vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
      vec4 possibleSolutionWithIndex;
      possibleSolutionWithIndex[0] = possibleSolution[0];
      possibleSolutionWithIndex[1] = possibleSolution[1];
      possibleSolutionWithIndex[2] = possibleSolution[2];
      possibleSolutionWithIndex[3] = triangle.faceIndex;
      solutions.push_back(possibleSolutionWithIndex); 
    }
    // if it has been culled then return a fake solution
    else {
      solutions.push_back(vec4 (-1,0,0,-1));
    } 
  } 
  return solutions; 
}
 
 
// this function gives back the index of the closest face for a particular ray 
RayTriangleIntersection closestIntersection(vector<vec4> solutions, vec3 rayPoint){ 
  RayTriangleIntersection closest; // this is where we store the closest triangle of intersection, the point it intersects and the distance 
  closest.distanceFromCamera = -1; // initialize this so we can tell when there are no intersections 
  float closestT = numeric_limits<float>::infinity(); 
  int closestIndex = -1; 
  // for each possible solution / for each face 
  int n = solutions.size(); 
  for (int i = 0 ; i < n ; i++){ 
     
    vec4 possibleSolution = solutions[i]; 
    float t = possibleSolution[0]; 
    float u = possibleSolution[1]; 
    float v = possibleSolution[2];
    int index = possibleSolution[3];
     
    // if it is actually a solution 
    bool bool1 = (t > 0); 
    bool bool2 = (0 <= u) && (u <= 1); 
    bool bool3 = (0 <= v) && (v <= 1); 
    bool bool4 = (u + v) <= 1; 
    if (bool1 && bool2 && bool3 && bool4){ 
      ModelTriangle triangle = faces[index]; 
      // is it closer than what we currently have? 
      if (t < closestT){ 
        closestT = t; 
        closestIndex = i; 
         
        // calculating the point of intersection 
        vec3 p0 = triangle.vertices[0]; 
        vec3 p1 = triangle.vertices[1]; 
        vec3 p2 = triangle.vertices[2]; 
        vec3 intersection = p0 + (u * (p1 - p0)) + (v * (p2 - p0)); 
        // calculating the distance between the camera and intersection point 
        vec3 d = intersection - rayPoint; 
        float distance = sqrt( (d[0]*d[0]) + (d[1] * d[1]) + (d[2] * d[2])); 
        // calculating the normal of the intersection 
        vec3 n0 = triangle.normals[0]; 
        vec3 n1 = triangle.normals[1]; 
        vec3 n2 = triangle.normals[2]; 
        vec3 normal = n0 + (u * (n1 - n0)) + (v * (n2 - n0)); 
        closest.intersectionPoint = intersection; 
        closest.intersectUV = vec2(u, v);
        closest.distanceFromCamera = distance; 
        closest.intersectedTriangle = triangle; 
        closest.normal = normal; 
      } 
    } 
  } 
  return closest; 
} 
 
 
 
 
 
 
//////////////////////////////////////////////////////// 
// LIGHTING 
//////////////////////////////////////////////////////// 
 
// rayPoint is the point in which this ray starts (normalls camera, but could be an intersection point if recursed)
// rayDirection is the direction of the ray
// depth coutns how many recursions we have done (this happens when there are reflections) - it starts at 0 when rays are shot from camera
// currentIOR stores the index of refraction of the current medium we are in (air is 1 - glass is 1.5)
Colour shootRay(vec3 rayPoint, vec3 rayDirection, int depth, float currentIOR){ 
  // stop recursing if our reflections get too much
  if (depth == 7){ 
    return Colour(255,255,255); 
  } 

  // does this ray intersect any of the faces?
  vector<vec4> solutions = checkForIntersections(rayPoint, rayDirection); 
  // find the closest intersection
  RayTriangleIntersection closest = closestIntersection(solutions, rayPoint); 
  Colour colour = closest.intersectedTriangle.colour; 
  vec3 point = closest.intersectionPoint; 
 
  // if this ray doesn't intersect anything, then we return the colour black 
  if (closest.distanceFromCamera <= 0) { 
    return Colour (0,0,0); 
  } 
 
  // the ambient, diffuse and specular light constants 
  float Ka = 0.2; 
  float Kd = 0.4; 
  float Ks = 0.4; 
 
  ModelTriangle triangle = closest.intersectedTriangle;

  // if this face is a mirror, create a reflected ray and shoot this ray (recurse this function) 
  if (triangle.texture == "mirror"){ 
    print = true; 
    vec3 incident = rayDirection; 
    vec3 normal = getNormalOfTriangle(closest.intersectedTriangle); 
    vec3 reflection = incident - (2 * dot(incident, normal) * normal); 
    reflection = normalize(reflection); 
    point = point + ((float)0.00001 * normal); // avoid self-intersection 
    colour = shootRay(point, reflection, depth + 1, currentIOR); 
    print = false; 
    return colour; 
  } 
  else if (triangle.texture == "glass"){
    colour = glass(rayDirection, closest, depth);
    return colour;
  }
  else if (triangle.texture == "texture") {
    colour = Colour(255, 255, 255);
    return colour;
  }

  // else we use Phong shading to get the colour
  else {
    float diffuseIntensity = intensityDropOff(point) * angleOfIncidence(closest); 
    Kd = Kd * diffuseIntensity; 
    // specular light 
    vec3 normal = closest.normal; // this is the interpolated normal for Phong shading (it is previously calculated and stored in the RayTriangleIntersection object) 
    float specularIntensity = calculateSpecularLight(point, rayDirection, normal); 
    Ks = Ks * specularIntensity; 
    colour =  getFinalColour(colour, Ka, Kd, Ks);
  }
 
  /*
  // CODE FOR HARD SHADOWS
  // are we in shadow? only colour the pixel if we are not 
  bool inShadow = checkForShadows(closest.intersectionPoint); 
  if (inShadow){ 
    Ka = Ka/2; 
    Kd = 0; 
    Ks = 0; 
    Colour cc = getFinalColour(colour, Ka, Kd, Ks); 
    return cc; 
  } 
  return colour;
  */

 
  // CODE FOR SOFT SHADOWS
  bool inShadow = checkForShadows(closest.intersectionPoint);
  if (inShadow){
    float shadowFraction = softShadows(closest);
    // mix shadow and normal colour
    Ka = Ka/2; 
    Kd = 0; 
    Ks = 0; 
    Colour shadowColour = getFinalColour(colour, Ka, Kd, Ks);

    Colour output;
    output.red = (shadowFraction * shadowColour.red) + ((1 - shadowFraction) * colour.red);
    output.green = (shadowFraction * shadowColour.green) + ((1 - shadowFraction) * colour.green);
    output.blue = (shadowFraction * shadowColour.blue) + ((1 - shadowFraction) * colour.blue);
    return output;
  }
  return colour;
} 
 
 
Colour getFinalColour(Colour colour, float Ka, float Kd, float Ks){ 
  // this takes the ambient, diffuse and specular constants and gets the output colour 
  // note that we do not multiply the specular light by the colour (it is white hence the 255) 
  colour.red = ((Ka + Kd) * colour.red) + (Ks * 255); 
  colour.green = ((Ka + Kd) * colour.green) + (Ks * 255); 
  colour.blue = ((Ka + Kd) * colour.blue) + (Ks * 255); 
 
  // clipping the colour for if it goes above 255 
  colour.red = glm::min(colour.red, 255); 
  colour.green = glm::min(colour.green, 255); 
  colour.blue = glm::min(colour.blue, 255); 
 
  return colour; 
} 
 
 
// given a point in the scene, this function calculates the intensity of the light 
float intensityDropOff(vec3 point){ 
  float distance = distanceVec3(point, lightPosition); 
  float intensity = lightIntensity / (2 * 3.1416 * distance * distance); 
  return intensity; 
} 
 
 
vec3 getNormalOfTriangle(ModelTriangle triangle){ 
  vec3 v0 = triangle.vertices[0]; 
  vec3 v1 = triangle.vertices[1]; 
  vec3 v2 = triangle.vertices[2]; 
  vec3 e0 = (v1 - v0); 
  vec3 e1 = (v2 - v0); 
  vec3 normal = glm::cross(e0, e1); 
  normal = normalize(normal); 
  return normal; 
} 
 
 
 
// this function takes an intersection point and calculates the angle of incidence and 
// outputs an intensity value between 0 and 1 
float angleOfIncidence(RayTriangleIntersection intersection){ 
  ModelTriangle triangle = intersection.intersectedTriangle; 
  vec3 normal = getNormalOfTriangle(triangle); 
  vec3 vectorToLight = lightPosition - intersection.intersectionPoint; 
  vectorToLight = normalize(vectorToLight); 
  float intensity = dot(normal, vectorToLight); 
  // the dot product returns 1 if they are parallel 
  // 0 if perpendicular 
  // <0 if the normal faces the other way 
  if (intensity < 0){ 
    intensity = 0; 
  } 
  return intensity; 
} 
 
 
 
float distanceVec3(vec3 from, vec3 to){ 
  vec3 d = from - to; 
  float a = d[0] * d[0]; 
  float b = d[1] * d[1]; 
  float c = d[2] * d[2]; 
  return sqrt(a + b + c); 
} 
 
 
 
// this returns a true or false depending on if we are in shadow or not 
bool checkForShadows(vec3 point){ 
  vec3 shadowRayDirection = lightPosition - point; 
  float distance = distanceVec3(lightPosition, point); 
  shadowRayDirection = normalize(shadowRayDirection); 
 
  // for each face, send a 'shadow ray' from the point to the light and check for intersections 
  int n = faces.size(); 
  for (int i = 0 ; i < n ; i++){ 
    ModelTriangle triangle = faces[i]; 
     
    // got the following code from the worksheet 
    vec3 e0 = triangle.vertices[1] - triangle.vertices[0]; 
    vec3 e1 = triangle.vertices[2] - triangle.vertices[0]; 
    vec3 SPVector = point - triangle.vertices[0]; 
    mat3 DEMatrix(-shadowRayDirection, e0, e1); 
    vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector; // this is a 1x3 vector of (t,u,v) (look at notes) 
    float t = possibleSolution[0]; 
    float u = possibleSolution[1]; 
    float v = possibleSolution[2]; 
     
    // if it is actually a solution 
    bool bool1 = (t > 0.0001); // this is not 0 to avoid self-intersection 
    bool bool2 = (0 <= u) && (u <= 1); 
    bool bool3 = (0 <= v) && (v <= 1); 
    bool bool4 = (u + v) <= 1; 
    bool bool5 = (t < distance); // an intersection beyond the light doesn't matter 
    // if we have an intersection then we can stop checking the other faces 
    // it is 0.000001 to avoid self intersection 
    if (bool1 && bool2 && bool3 && bool4 && bool5){ 
      return true; 
    } 
  } 
  return false; 
} 
 
 
float calculateSpecularLight(vec3 point, vec3 rayDirection, vec3 normal){ 
  // got the method from the lecture slides 
  vec3 lightDirection = point - lightPosition; 
  vec3 incident = normalize(lightDirection); 
  normal = normalize(normal); 
  // equation form online 
  vec3 reflection = incident - (2 * dot(incident, normal) * normal); 
  reflection = normalize(reflection); 
  vec3 viewerDirection = -rayDirection; 
  float intensity = dot(viewerDirection, reflection); 
  // CAN CHANGE THE SPECULAR DROP OFF HERE 
  intensity = pow(intensity, 20); 
  if (intensity < 0){ 
    intensity = 0; 
  } 
  return intensity; 
} 
 
 
 
/* 
 
void phongBRDF(vec3 point, vec3 normal){ 
  ///////////////////////////// 
  // PARAMETERS 
  ///////////////////////////// 
  float Ka = 0.2; // ambient constant 
  float Kd = 0.4; // diffuse constant 
  float Ks = 0.4; // specular constant 
  ///////////////////////////// 
   
  vec3 viewerDirection = cameraPosition - point; 
  vec3 lightDirection = point - lightPosition; 
  vec3 incident = normalize(lightDirection); 
  normal = normalize(normal); 
  // equation from online 
  vec3 reflection = incident - (2 * dot(incident, normal) * normal); 
  reflection = normalize(reflection); 
 
  // equation got from wikipedia 
  float value1 = dot(-lightDirection, normal); 
  float value2 = dot(reflection, viewerDirection); 
  float intensity = (Ka * lightIntensity) + (Kd * value1); 
 
} 
*/ 
 
 

float softShadows(RayTriangleIntersection intersection){
  ///////////////////
  // PARAMETERS
  ///////////////////
  float gradientConstant = 0.5; // how big the gradient should be
  int numberOfSteps = 50; // how smooth it should be
  ///////////////////


  ModelTriangle triangle = intersection.intersectedTriangle;
  vec3 normal = getNormalOfTriangle(triangle);
  vec3 point = intersection.intersectionPoint;
  vec3 up = gradientConstant * normal;
  //vec3 down = -gradientConstant * normal;
  vec3 pointAbove = point + up;
  vec3 pointBelow = point;//point + down;
  bool above = checkForShadows(pointAbove);
  bool below = checkForShadows(pointBelow);
  float shadowFraction = 0;
  // we are in total light 
  if ((above == 0) && (below == 0)){
    shadowFraction = 0;
  } 
  // we are in total shadow 
  else if ((above == 1) && (below == 1)){
    shadowFraction = 1;
  } 
  // we are in-between, and need to calculate the gradient 
  else {
    vec3 upVector = pointAbove - pointBelow;
    for (int i = 1 ; i <= numberOfSteps ; i++){
      vec3 point = pointBelow + ((i / (float)numberOfSteps) * upVector);
      bool inShadow = checkForShadows(point);
      // the first point will definitely be in shadow and as we move up we find how in shadow it should be
      if (not(inShadow)){
        shadowFraction = (i / (float)numberOfSteps);
        return shadowFraction;
      }
    }
  }
  return shadowFraction;
} 
 
 

 
 
void gouraudShading() { 
} 
 
 
void averageVertexNormals(){ 
  textures(); 
  int currentFaceIndex = 0; // stores the number of how many faces we have gone through yet 
  vector<vec3> faceVertices; // stores the indices of which vertices make up each face 
   
  // Attempt to open OBJFile. 
  ifstream myfile(objFileName); 
 
  // if we cannot open the file, print an error 
  if (myfile.is_open() == 0) cout << "Unable to open file" << "\n"; 
 
  string line; //used as buffer for ifstream. 
 
  // go through and figure out how many vertices we have 
  int numberOfVertices = 0; 
  while (getline(myfile, line)){ 
    if (line.find('v')==0){ 
      numberOfVertices = numberOfVertices + 1; 
    } 
  } 
 
  // open and close the file again 
  myfile.close(); 
  ifstream myfile2(objFileName); 
 
  // make a vector to store the normals for each vertex (so each element in the vector will be another 
  // vector containing all the normals of the faces in which that vertex is a part of) 
  vector<vector<vec3>> vertexNormals(numberOfVertices); 
  vector<vec3> averagedNormals(numberOfVertices); // once all normals for each vertex have been found, this stores the average of them 
 
  // take the OBJ file again and go through each face, get the normal and then store that normal 
  // in the vector for all 3 vertices 
 
   
  while (getline(myfile2, line)){ 
    if (line.find('f') == 0){   
      // if this line is a face, then get the normal of it (we have pre-stored the faces so can do this) 
      ModelTriangle face = faces[currentFaceIndex]; 
      vec3 normal = getNormalOfTriangle(face); 
      currentFaceIndex = currentFaceIndex + 1; 
 
      vector<string> faceLine = separateLine(line); // this turns 'f 1/1 2/2 3/3' into ['f','1/1','2/2','3/3'] 
 
      vec3 verticesIndex (0,0,0); 
 
      // go through the face and for each vertex, store the normal in the corresponding space in the vector 
      for (int i = 1 ; i < 4 ; i++){ 
        string element = faceLine[i]; // this will be '1/1' for example 
        int slashIndex = element.find('/'); 
        string number = element.substr(0, slashIndex); 
        int index = stoi(number) - 1;// -1 as the vertices are numbered from 1, but c++ indexes from 0 
        verticesIndex[i-1] = index; 
        vertexNormals[index].push_back(normal); 
      } 
      faceVertices.push_back(verticesIndex); 
    } 
  } 
 
  myfile2.close(); 
 
  // for each vertex, go through and average each of the normals 
  for (int i = 0 ; i < numberOfVertices ; i++){ 
    vector<vec3> normals = vertexNormals[i]; 
    int n = normals.size(); 
    vec3 sum (0,0,0); 
    for (int j = 0 ; j < n ; j++){ 
      sum = sum + normals[j]; 
    } 
    sum = sum / (float)n; 
    averagedNormals[i] = sum; 
  } 
 
  // go through the faces again and store the average normal in the ModelTriangle object 
  for (int i = 0 ; i < faces.size() ; i++){ 
    vec3 v = faceVertices[i]; 
    // for each vertex 
    for (int j = 0 ; j < 3 ; j++){ 
      int index = v[j]; 
      vec3 normal = averagedNormals[index]; 
      faces[i].normals[j] = normal; 
    } 
  } 
} 
 

Colour glass(vec3 rayDirection, RayTriangleIntersection closest, int depth){
  vec3 point = closest.intersectionPoint;
  ModelTriangle triangle = closest.intersectedTriangle;
  vec3 normal = getNormalOfTriangle(triangle);

  // send the reflection ray
  vec3 incident = rayDirection; 
  vec3 reflection = incident - (2 * dot(incident, normal) * normal); 
  reflection = normalize(reflection); 
  vec3 newPoint = point + ((float)0.00001 * normal); // avoid self-intersection 
  Colour reflectionColour = shootRay(newPoint, reflection, depth + 1, 1); // IOR back to 1 as moving in air
  
  // send the refraction ray
  float refractiveIndex = 1.3;
  vec4 refraction = refract(incident, normal, refractiveIndex);
  int direction = refraction[3];
  vec3 refracted;
  refracted[0] = refraction[0];
  refracted[1] = refraction[1];
  refracted[2] = refraction[2];
  if (refracted == vec3 (0,0,0)){
    return reflectionColour;
  }

  // we need to adjust the point to avoid self-intersection but this depends on if we are going through the face or reflecting from it
  if (direction == -1){
    // we are entering a new material
    newPoint = point - ((float)0.0001 * normal);
  }
  else {
    // we are leaving the material
    newPoint = point + ((float)0.0001 * normal);
  }
  Colour refractionColour = shootRay(newPoint, refracted, depth + 1, 1.5); // IOR is 1.5 as now we are travelling in glass


  // mix them together using Fresnel equation
  float reflectiveConstant = fresnel(rayDirection, normal, refractiveIndex);
  float refractiveConstant = 1 - reflectiveConstant;

  Colour output;
  output.red = (reflectiveConstant * reflectionColour.red) + (refractiveConstant * refractionColour.red);
  output.green = (reflectiveConstant * reflectionColour.green) + (refractiveConstant * refractionColour.green);
  output.blue = (reflectiveConstant * reflectionColour.blue) + (refractiveConstant * refractionColour.blue);
  return output;
}


// calculates the direction of the refraction
// returns a vec4 (first 3 values are the direction - the last displays whether we are entering (-1) or leaving (1) the material)
vec4 refract(vec3 incident, vec3 normal, float ior) { 
  // are we hitting the surface from inside or outside the material?
  float direction = dot(incident, normal); // negative when entering the material, positive when leaving
  float n1 = 1;
  float n2 = ior;
  float ratio = n1 / n2;
  // if we are leaving the material
  if (direction > 0){
    ratio = 1 / ratio;
    normal = -normal;
  }
  float cosTheta = abs(direction);

  // check: do we have total internal reflection (this depends on the incident ray and the critical angle)
  float k = 1 - ratio * ratio * (1 - cosTheta * cosTheta);
  if (k < 0){
    // total internal reflection
    return vec4 (0,0,0,0);
  }

  vec3 refracted = ratio * incident + (ratio * cosTheta - sqrtf(k)) * normal;
  refracted = normalize(refracted);
  vec4 output;
  output[0] = refracted[0];
  output[1] = refracted[1];
  output[2] = refracted[2];
  output[3] = sign(direction);
  return output;
}


float fresnel(vec3 incident, vec3 normal, float ior) {
    float cosi = dot(incident, normal); 
    float etai = 1;
    float etat = ior; 
    if (cosi > 0){ 
      std::swap(etai, etat); 
    } 
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi)); 
    // Total internal reflection
    float kr;
    if (sint >= 1) { 
        kr = 1; 
    } 
    else { 
        float cost = sqrtf(std::max(0.f, 1 - sint * sint)); 
        cosi = fabsf(cosi); 
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        kr = (Rs * Rs + Rp * Rp) / 2; 
    } 
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
    return kr;
} 



////////////////////
// CULLING CODE
////////////////////


// we cull the faces in the scene that face away from the camera
// we do this by taking vectors from the camera to the centre of each face and dotting it with the normal
void backfaceCulling(vec3 cameraPosition){
  int n = faces.size();
  // for each face
  for (int i = 0 ; i < n ; i++){
    ModelTriangle face = faces[i];
    // get the normal of the face
    vec3 normal = getNormalOfTriangle(face);
    // get the centre of the face by averaging the vertices
    vec3 faceCentre = face.vertices[0] + face.vertices[1] + face.vertices[2];
    faceCentre = faceCentre * float(1/3);
    vec3 cameraToFace = faceCentre - cameraPosition;

    // if the face faces away
    if (dot(cameraToFace, normal) > 0){
      face.culled = true;
    }
  }
}


// for a set of vertices (an object maybe), create a bounding box
vector<ModelTriangle> boundingBox(vector<ModelTriangle> inputFaces){
  // we want to find the minimum and maximum values of x,y,z in all the vertices
  // first set the min to be (inf, inf, inf)
  // set max to be (-inf, -inf, -inf)
  vec3 min (numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity());
  vec3 max = -min;
  printVec3(min);
  printVec3(max);
  int n = inputFaces.size();
  // for each face
  for (int i = 0 ; i < n ; i++){
    ModelTriangle triangle = inputFaces[i];
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = triangle.vertices[j];
      // check for all the x,y,z values separately to see if it is a new min or maximum
      for (int k = 0 ; k < 3 ; k++){
        float value = vertex[k];
        if (value < min[k]){
          min[k] = value;
        }
        if (value > max[k]){
          max[k] = value;
        }
      }
    }
  }

  printVec3(min);
  printVec3(max);

  // create the box object
  vector<ModelTriangle> boxFaces; // this is a cube so will have 12 faces
  vector<vec3> vertices; // these must be in a particular order so the faces can be constructed facing the right way and also the right faces are constructed
  vertices.push_back( vec3 (min[0], min[1], min[2]) ); // bottom-left-forward   v1
  vertices.push_back( vec3 (max[0], min[1], min[2]) ); // bottom-right-forward  v2 
  vertices.push_back( vec3 (min[0], max[1], min[2]) ); // top-left-forward      v3
  vertices.push_back( vec3 (max[0], max[1], min[2]) ); // top-right-forward     v4
  vertices.push_back( vec3 (min[0], min[1], max[2]) ); // bottom-left-back      v5
  vertices.push_back( vec3 (max[0], min[1], max[2]) ); // bottom-right-back     v6
  vertices.push_back( vec3 (min[0], max[1], max[2]) ); // top-left-back         v7
  vertices.push_back( vec3 (max[0], max[1], max[2]) ); // top-right-back        v8

  cout << "n: " << vertices.size() << endl;

  /*
  front face:
  1 2 3
  2 4 3
  top face:
  3 4 7
  4 8 7
  back face:
  7 8 5
  8 6 5
  bottom face:
  5 6 1
  6 2 1
  left face:
  5 1 7
  1 3 7
  right face:
  2 6 4
  6 8 4
  */

 vector<vec3> vertexIndices;
 vertexIndices.push_back(vec3 (1,2,3));
 vertexIndices.push_back(vec3 (2,4,3));
 vertexIndices.push_back(vec3 (3,4,7));
 vertexIndices.push_back(vec3 (4,8,7));
 vertexIndices.push_back(vec3 (7,8,5));
 vertexIndices.push_back(vec3 (8,6,5));
 vertexIndices.push_back(vec3 (5,6,1));
 vertexIndices.push_back(vec3 (6,2,1));
 vertexIndices.push_back(vec3 (5,1,7));
 vertexIndices.push_back(vec3 (1,3,7));
 vertexIndices.push_back(vec3 (2,6,4));
 vertexIndices.push_back(vec3 (6,8,4));


  // make the vertices into faces
  for (int i = 0 ; i < 12 ; i++){
    vec3 indices = vertexIndices[i];
    ModelTriangle triangle;
    triangle.colour = Colour (255,255,255); // arbitrarily set colour to white (for testing)
    for (int j = 0 ; j < 3 ; j ++){
      int index = indices[j] - 1;
      triangle.vertices[j] = vertices[index];
      printVec3(vertices[index]);
    }
    boxFaces.push_back(triangle);
  }
  return boxFaces;
}

