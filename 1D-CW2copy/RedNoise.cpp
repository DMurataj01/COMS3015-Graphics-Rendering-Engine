#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;
using namespace glm;

#define WIDTH 640//640
#define HEIGHT 480//480


/* STRUCTURE - ImageFile */
struct ImageFile {
  vector<Colour> vecPixelList;
  int width;
  int height;  
};


void update();
void handleEvent(SDL_Event event);
vector<vec3> interpolate(vec3 from, vec3 to, float numberOfValues);
uint32_t getColour(Colour colour);
void printVec3(vec3 input);
void drawLine(CanvasPoint start, CanvasPoint end, Colour colour);
void drawStrokedTriangle(CanvasTriangle triangle);
void drawRandomTriangle();
void drawFilledTriangle(CanvasTriangle triangle);
void drawRandomFilledTriangle();
void a();
void readPPM();
void test();
vector<Colour> readOBJMTL(string filename);
string removeLeadingWhitespace(string s);
ImageFile readImage(string fileName);
vector<string> separateLine(string inputLine);
vec3 getVertex(string inputLine);
ModelTriangle getFace(string inputLine, vector<vec3> vertices, Colour colour, float scalingFactor);
vector<ModelTriangle> readOBJ(float scalingFactor);
void rasterize(vector<ModelTriangle> faces);
void initializeDepthMap();
void updateView(string input);
/* FUNCTION Declarations */
ImageFile readImage(string fileName);
void renderImage(ImageFile imageFile);




DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);


// the files (scene) which we want to render
string objFileName = "cornell-box.obj";
string mtlFileName = "cornell-box.mtl";

// this is where we will store the faces of the OBJ file
vector<ModelTriangle> faces;


// create a global depth map
float depthMap [WIDTH*HEIGHT];


// initial camera parameters
vec3 cameraPosition (0,-2,-3.5);
vec3 cameraRight (1,0,0);
vec3 cameraUp (0,-1,0);
vec3 cameraForward (0,0,-1);
mat3 cameraOrientation (cameraRight, cameraUp, cameraForward);
float focalLength = WIDTH / 2;



int main(int argc, char* argv[])
{
  initializeDepthMap();

  window.clearPixels();

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



void handleEvent(SDL_Event event)
{
  if(event.type == SDL_KEYDOWN) {
    if(event.key.keysym.sym == SDLK_LEFT){
      cout << "LEFT" << endl;
      updateView("left");
    }
    else if(event.key.keysym.sym == SDLK_RIGHT){
      cout << "RIGHT" << endl;
      updateView("right");
    }
    else if(event.key.keysym.sym == SDLK_UP){
      cout << "UP" << endl;
      updateView("up");
    }
    else if(event.key.keysym.sym == SDLK_DOWN){
      cout << "DOWN" << endl;
      updateView("down");
    }
    else if(event.key.keysym.sym == SDLK_u) drawRandomTriangle();
    else if(event.key.keysym.sym == SDLK_f) drawRandomFilledTriangle();
    else if(event.key.keysym.sym == SDLK_t) test();
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
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
    //window.setPixelColour(start.x, start.y, col);
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
            window.setPixelColour(x,y,col);
            depthMap[index] = depth;
          }
        }
      }
      
    }
  }
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


// draws a random unfilled triangle (this can be activated by pressing 'u')
void drawRandomTriangle(){
  // getting random points
  int x1 = round(rand()%WIDTH);
  int x2 = round(rand()%WIDTH);
  int x3 = round(rand()%WIDTH);
  int y1 = round(rand()%HEIGHT);
  int y2 = round(rand()%HEIGHT);
  int y3 = round(rand()%HEIGHT);

  CanvasPoint point1 (x1, y1);
  CanvasPoint point2 (x2, y2);
  CanvasPoint point3 (x3, y3);

  // getting random colour
  int red = round(rand()%255);
  int green = round(rand()%255);
  int blue = round(rand()%255);

  Colour colour (red, green, blue);
  CanvasTriangle triangle (point1, point2, point3, colour);

  drawStrokedTriangle(triangle);
}


void drawFilledTriangle(CanvasTriangle triangle){
  // separating the CanvasTriangle object and storing the vertices and colour separately
  CanvasPoint point1 = triangle.vertices[0];
  CanvasPoint point2 = triangle.vertices[1];
  CanvasPoint point3 = triangle.vertices[2];
  Colour colour = triangle.colour;
  
  // putting all the points in a vector so they can be retrieved by index
  vector<CanvasPoint> points;
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);

  // sort the points by the y value
  for (int i = 0 ; i < 2 ; i++){
    CanvasPoint pointOne = points[i];
    CanvasPoint pointTwo = points[i+1];
    // if the next point has a lower y value then swap them
    if (pointOne.y > pointTwo.y){
      points[i+1] = pointOne;
      points[i] = pointTwo;
    }
  }
  // still sorting : check the first two again
  CanvasPoint pointOne = points[0];
  CanvasPoint pointTwo = points[1];
  if (pointOne.y > pointTwo.y){
    points[0] = pointTwo;
    points[1] = pointOne;
  }

  CanvasPoint maxPoint = points[0];
  CanvasPoint middlePoint = points[1];
  CanvasPoint minPoint = points[2];
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
    drawLine(middlePoint, maxPoint, colour);
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
      drawLine(start,end,colour);
    }
  }
  
  // the lower triangle
  // for each row, fill it in
  float steps2 = minPoint.y - middlePoint.y; // how many rows
  if (steps2 == 0) {
    drawLine(minPoint, middlePoint, colour);
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
      drawLine(start,end,colour);
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



void drawRandomFilledTriangle(){
  int x1 = round(rand()%WIDTH);
  int x2 = round(rand()%WIDTH);
  int x3 = round(rand()%WIDTH);
  int y1 = round(rand()%HEIGHT);
  int y2 = round(rand()%HEIGHT);
  int y3 = round(rand()%HEIGHT);

  CanvasPoint point1 (x1, y1);
  CanvasPoint point2 (x2, y2);
  CanvasPoint point3 (x3, y3);

  int red = round(rand()%255);
  int green = round(rand()%255);
  int blue = round(rand()%255);

  Colour colour (red, green, blue);
  CanvasTriangle triangle (point1, point2, point3, colour);

  cout << "Point 1: " << x1 << ", " << y1 << "\n";
  cout << "Point 2: " << x2 << ", " << y2 << "\n";
  cout << "Point 3: " << x3 << ", " << y3 << "\n\n";

  drawFilledTriangle(triangle);
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
      faces.push_back(triangle);
    }
  }
  //cout << "Printing the faces: \n";
  //for (int i = 0 ; i < faces.size() ; i++){
  //  cout << faces[i] << "\n";
  //}
  return faces;
}




void rasterize(vector<ModelTriangle> faces){
  cout << "AFTER_AFTER: [" << cameraPosition[0] << ", " << cameraPosition[1] << ", " << cameraPosition [2] << "]\n";

  // for each face
  for (int i = 0 ; i < faces.size() ; i++){
    ModelTriangle triangle = faces[i];
    CanvasTriangle canvasTriangle;
    canvasTriangle.colour = triangle.colour;
    
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = triangle.vertices[j];
      
      // change from world coordinates to camera
      vec3 vertexCSpace = (cameraOrientation * vertex) - cameraPosition; // CSpace means camera space
      
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

    drawFilledTriangle(canvasTriangle);

  }
}



void initializeDepthMap(){
  for (int i = 0 ; i < (HEIGHT*WIDTH) ; i++){
    depthMap[i] = numeric_limits<float>::infinity();
  }
}



void updateView(string input){

  initializeDepthMap();
  window.clearPixels();
  vec3 direction (0, 0, 0);

  if (input == "up"){
    direction = cameraUp;
    cout << "up" << "\n";
  }

  else if (input == "down"){
    direction = -cameraUp;
  }

  else if (input == "right"){
    direction = cameraRight;
  }

  else if (input == "left"){
    direction = -cameraRight;
  }
  
  //direction = normalize(direction);

  cout << "dir: " << direction[0] << direction [1] << direction [2] << "\n";
  cout << "BEFORE: [" << cameraPosition[0] << ", " << cameraPosition[1] << ", " << cameraPosition [2] << "]\n";
  cameraPosition = cameraPosition + direction;  
  cout << "AFTER: [" << cameraPosition[0] << ", " << cameraPosition[1] << ", " << cameraPosition [2] << "]\n";
  rasterize(faces);


}



void test(){
  faces = readOBJ(1);
  rasterize(faces);
}


void renderImage(ImageFile imageFile){

  for (int i=0; i<imageFile.vecPixelList.size(); i++){
    int red = imageFile.vecPixelList.at(i).red;
    int green = imageFile.vecPixelList.at(i).green;
    int blue = imageFile.vecPixelList.at(i).blue;
    uint32_t colour = (255<<24) + (red<<16) + (green<<8) + blue;
    int row = int(i/imageFile.width);
    int col = i - (row*imageFile.width);
    window.setPixelColour(col, row, colour);
  }
  cout << "\nImage render complete.\n";
}