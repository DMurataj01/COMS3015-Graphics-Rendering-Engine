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
vector<string> separateLine(string inputLine);
vec3 getVertex(string inputLine);
ModelTriangle getFace(string inputLine, vector<vec3> vertices, Colour colour, float scalingFactor);
vector<ModelTriangle> readOBJ(string objFileName, string mtlFileName, float scalingFactor);
void rasterize(string objFileName, string mtlFileName);


DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);


int main(int argc, char* argv[])
{
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
    if(event.key.keysym.sym == SDLK_LEFT) cout << "LEFT" << endl;
    else if(event.key.keysym.sym == SDLK_RIGHT) cout << "RIGHT" << endl;
    else if(event.key.keysym.sym == SDLK_UP) cout << "UP" << endl;
    else if(event.key.keysym.sym == SDLK_DOWN) cout << "DOWN" << endl;
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
  float stepSizeX = diffX / numberOfSteps;
  float stepSizeY = diffY / numberOfSteps;
  
  // getting the colour in a single string
  uint32_t col = getColour(colour);

  for (int i = 0 ; i < numberOfSteps ; i++){
    int x = round(start.x + (i * stepSizeX));
    int y = round(start.y + (i * stepSizeY));
    window.setPixelColour(x,y,col);
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
  float yProportion = yDistance / maxYDistance;
  float xDistance = maxPoint.x + (yProportion * maxXDistance);
  CanvasPoint cutterPoint(xDistance, maxPoint.y + yDistance);

  // the upper triangle
  // for each row, fill it in
  float steps = middlePoint.y - maxPoint.y; // how many rows
  for (int i = 0 ; i < steps + 1 ; i++){
    // find the two points which intersect this row
    float yDiff = i / steps;
    float maxXDiff1 = maxPoint.x - middlePoint.x;
    float maxXDiff2 = maxPoint.x - cutterPoint.x;
    float xStart = round(maxPoint.x - (yDiff * maxXDiff1));
    float xEnd = round(maxPoint.x - (yDiff * maxXDiff2));
    CanvasPoint start(xStart, maxPoint.y + i);
    CanvasPoint end(xEnd, maxPoint.y + i);
    drawLine(start,end,colour);
  }
  
  // the lower triangle
  // for each row, fill it in
  float steps2 = minPoint.y - middlePoint.y; // how many rows
  for (int i = steps2 ; i > 0 ; i--){
    // find the two points which intersect this row
    float yDiff = 1 - (i / steps2);
    float maxXDiff1 = minPoint.x - middlePoint.x;
    float maxXDiff2 = minPoint.x - cutterPoint.x;
    float xStart = round(minPoint.x - (yDiff * maxXDiff1));
    float xEnd = round(minPoint.x - (yDiff * maxXDiff2));
    CanvasPoint start(xStart, cutterPoint.y + i);
    CanvasPoint end(xEnd, cutterPoint.y + i);
    drawLine(start,end,colour);
  }
  /*
  // this code draws the outlien of the triangle ontop of the filled triangle to make sure it is correct
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
  cout << "Calling function readOBJMTL:\n";
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
        cout << colour << "\n";
      }
    }
  }
  else cout << "Unable to open file";

  return colours;
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



// given a vector in form ['v','12','5','4'] it returns (12,5,4)
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



vector<ModelTriangle> readOBJ(string objFileName, string mtlFileName, float scalingFactor){
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
return faces;
}


void rasterize(string objFileName, string mtlFileName){
  // camera parameters
  vec3 cameraPosition (0,0,-1);
  vec3 cameraRight (1,0,0);
  vec3 cameraUp (0,1,0);
  vec3 cameraForward (0,0,-1);
  mat3 cameraOrientation (cameraRight, cameraUp, cameraForward);
  float focalLength = 1;
  // image plane parameters
  float imagePlaneHeight = 2;
  float imagePlaneWidth = 2;

  // read in the files and get the vector of faces/triangles
  vector<ModelTriangle> faces = readOBJ(objFileName, mtlFileName, 1);

  // for each face
  for (int i = 0 ; i < faces.size() ; i++){
    ModelTriangle triangle = faces[i];
    CanvasTriangle canvasTriangle;
    canvasTriangle.colour = triangle.colour;
    int onScreen = 0; // this states how many of the points can be seen on the screen (we only draw the face if all 3 points can be seen)
    
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = triangle.vertices[j];
      
      // change from world coordinates to camera
      vec3 vertexCSpace = (cameraOrientation * vertex) - cameraPosition; // CSpace means camera space
      
      // calculating the projection onto the 2D image plane by using interpolation and the z depth
      float depth = vertexCSpace[2];
      
      // only worth doing if the vertex is in front of camera
      if (depth > 0){
        float proportion = focalLength / depth;
        vec3 vertexProjected = vertexCSpace * proportion; // the coordinates of the 3D point (in camera space) projected onto the image plane
        float x = vertexProjected[0];
        float y = vertexProjected[1];

        // converting this 2D point into pixel x and y values
        // this point is only visible if the absolute value of x and y are less than half the value of the image plane heigh/width
        if ((abs(x) < (imagePlaneWidth / 2)) && (abs(y) < (imagePlaneHeight / 2))){
          onScreen = onScreen + 1;
          float xNormalised = (x / imagePlaneWidth) + 0.5;
          float yNormalised = (y / imagePlaneHeight) + 0.5;
          float xPixel = xNormalised * WIDTH;
          float yPixel = yNormalised * HEIGHT;
          // store the pixel values as a Canvas Point and save it for this triangle
          canvasTriangle.vertices[j] = CanvasPoint (xPixel,yPixel);
        }
      }
    }

    // now we have all 3 vertices as CanvasPoint's, draw the triangle only if all the face can be seen
    if (onScreen == 3){
      drawStrokedTriangle(canvasTriangle);
    }

  }
}



void test(){
  rasterize("cornell-box.obj","cornell-box.mtl");
  /*
  vec3 cameraPosition (0,0,-1);
  vec3 cameraUp (0,1,0);
  vec3 cameraRight (-1,0,0);
  vec3 cameraForward (0,0,-1);
  mat3 cameraOrientation (cameraRight, cameraUp, cameraForward);
  vec3 vertex (1,0,0);
  vec3 newPoint = (cameraOrientation * vertex) - cameraPosition;
  cout << "[" << newPoint[0] << ", " << newPoint[1] << ", " << newPoint[2] << "]\n";
  */
}



























// OLD FUNCTIONS


/*
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

        float red;
        float green;
        float blue;
        string name;

        // getting the name of the colour
        int n = line.length();
        name = line.substr(7,n);

        
        // getting the colour - get the next line and then iterate through to find the spaces, and then get the numbers in between
        getline(myfile,line);
        string lineCopy = line;

        for (int i = 0 ; i < 4 ; i++){
          // find where the space is, then take a substring of the end of the line to then find the next space
          int spaceIndex = lineCopy.find(' ');
          int n = lineCopy.length();
          string value = lineCopy.substr(0,spaceIndex); // this stores the string inetween 2 spaces

          // once we have found the first 2 spaces we can then start storing the colours
          // MTL files have a max value of 0, a Colour object uses 0-255 range
          if (i == 1){
            red = stof(value)*255;
          }
          else if (i == 2){
            green = stof(value)*255;
          }
          else if (i == 3){
            blue = stof(value)*255;
          }

          // update the current lineCopy so we then check the next part of the string for a space
          lineCopy = lineCopy.substr(spaceIndex + 1, n);
        }
        // create the colour object and push the element into the colours array
        Colour colour (name, red, green, blue);
        colours.push_back(colour);
      }
    }
  }
  else cout << "Unable to open file";

  return colours;
}



vector<ModelTriangle> readOBJ(string filename){
  vector<vec3> vertices; // storing the vertices
  vector<ModelTriangle> output; // storing the faces in terms of the vertices and the colour of each

  // open the file, we then go through each line storing it in a string
  string line;
  ifstream myfile(filename);

  if (myfile.is_open()){
    while (getline(myfile,line)){
      
      // if the line starts with a v, then we have a vertex
      if (line.find('v') == 0){
        vec3 vertex;
        
        // iterate through finding where the spaces are and then storing the points in a vec3
        string lineCopy = line;
        for (int i = 0 ; i < 4 ; i++){
          int spaceIndex = lineCopy.find(' ');
          int n = lineCopy.length();
          // once we have gone past the first space we then have the numbers and can store them
          if (i >= 1){
            string numberString = lineCopy.substr(0, spaceIndex);
            float number = stof(numberString); // make the string a float
            vertex[i-1] = number;
          }
          lineCopy = lineCopy.substr(spaceIndex+1, n);
        }

        vertices.push_back(vertex);
        cout << "Vertex: [" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << "]\n";
      }
      
      // if the line starts with an f, then we have a face stating which vertices are connected
      else if (line.find('f') == 0){
        vec3 face; // stores the indices of which vertices are used in this face

        // iterate through finding where the spaces are and then storing the values inbetween these spaces in a vec3 object
        string lineCopy = line;
        for (int i = 0 ; i < 3 ; i++){
          // we only need to worry about the first number (as a face has format 'f 1/1 2/2 3/3')
          int spaceIndex = lineCopy.find(' ');
          face[i] = stof(lineCopy.substr(spaceIndex + 1,spaceIndex + 2));
          int n = lineCopy.length();
          lineCopy = lineCopy.substr(spaceIndex + 1, n);
        }

        // taking the face (so the numbers of which vertices make up the face) and storing them in a ModelTriangle object
        vector<vec3> faceVertices;
        for (int i = 0 ; i < 3 ; i++){
          int index = face[i] - 1; // which number vertex it is (starting from 0)
          faceVertices.push_back(vertices[index]); // store all 3 vertices in a vector
        }
      
        // store this as a ModelTriangle object and add it to the output
        ModelTriangle triangle (faceVertices[0], faceVertices[1], faceVertices[2], Colour (1,0,0));
        cout << "triangle: \n" << triangle << "\n";
        output.push_back(triangle);
      }
    }
  }

  else cout << "Unable to open file";

  return output;
}

*/