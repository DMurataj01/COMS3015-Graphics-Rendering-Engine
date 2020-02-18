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
uint32_t getColour(vec3 colour);
void printVec3(vec3 input);
void drawLine(vec2 start, vec2 end, vec3 colour);
void drawStrokedTriangle(vec2 point1, vec2 point2, vec2 point3, vec3 colour);
void drawRandomTriangle();
void drawFilledTriangle(vec2 point1, vec2 point2, vec2 point3, vec3 colour);
void drawRandomFilledTriangle();
void a();
void test();
void test2();
vector<vector<string>> readOBJMTL(string filename);
vector<ModelTriangle> readOBJ(string filename);

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


// draws an interpolation between colours in the 4 corners of the window (creates a rainbow effect)
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
    else if(event.key.keysym.sym == SDLK_t) readOBJ("example.txt");
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
uint32_t getColour(vec3 colour){
  int red = int(colour[0]);
  int green = int(colour[1]);
  int blue = int(colour[2]);
  uint32_t col = (255<<24) + (red<<16) + (green<<8) + blue;
  return col;
}

void printVec3(vec3 input){
  cout << '[' << input[0] << ", " << input[1] << ", " << input[2] << ']';
}


// draws a 2D line from start to end (colour is in (r,g,b) format with 255 as max)
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


// draws an unfilled triangle with the input points as vertices
void drawStrokedTriangle(vec2 point1, vec2 point2, vec2 point3, vec3 colour){
  drawLine(point1,point2,colour);
  drawLine(point2,point3,colour);
  drawLine(point3,point1,colour);
}


// draws a random unfilled triangle (this can be activated by pressing 'u')
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

void test(){

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
      uint32_t a = getColour(vec3(red,green,blue));
      cout << a << "\n";
    }

    myfile.close();
  }

  else cout << "Unable to open file"; 
}

// this function reads in an OBJ material file and stores it in a vector of vectors, so an output may look like:
// [['Red','1','0','0'] , ['Green','0','1','0']]
vector<vector<string>> readOBJMTL(string filename){
  // store the colours in a vector of vectors - one element would look like ['Red',255,0,0]
  vector<vector<string>> colours;
  // open the file, we then go through each line storing it in a string
  string line;
  ifstream myfile(filename);
  
  if (myfile.is_open()){
    while (getline(myfile,line)){
      cout << line;
      
      // if we have a new material, store it
      if (line.find("newmtl") == 0){
        // getting the name of the colour
        int n = line.length();
        string name = line.substr(7,n);
        // store it in a vector
        vector<string> element;
        element.push_back(name);
        cout << name << "\n";
        
        // getting the colour - get the next line and then iterate through to find the spaces, and then get the numbers in between
        getline(myfile,line);
        string lineCopy = line;
        for (int i = 0 ; i < 4 ; i++){
          // find where the space is, then take a substring of the end of the line to then find the next space
          int spaceIndex = lineCopy.find(' ');
          int n = lineCopy.length();
          // once we have found the first 2 spaces we can then start storing the colours
          if (i >= 1){
            string value = lineCopy.substr(0, spaceIndex);
            cout << value << "\n";
            element.push_back(value);
          }
          lineCopy = lineCopy.substr(spaceIndex + 1, n);
        }
        // push the element into the colours array
        colours.push_back(element);
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
        cout << "VERTEX: \n";
        vec3 vertex;
        
        // iterate through finding where the spaces are and then storing the vertex in a vec3
        string lineCopy = line;
        for (int i = 0 ; i < 4 ; i++){
          int spaceIndex = lineCopy.find(' ');
          int n = lineCopy.length();
          // once we have gone past the first space we then have the numbers and can store them
          if (i >= 1){
            string numberString = lineCopy.substr(0, spaceIndex);
            float number = stof(numberString); // make the string a float
            vertex[i-1] = number;
            cout << number << "\n";
          }
          lineCopy = lineCopy.substr(spaceIndex+1, n);

          vertices.push_back(vertex);
        }
      }
      // if the line starts with an f, then we have a face stating which vertices are connected
      if (line.find('f') == 0){
        cout << "FACE: \n";
        vec3 face;

        // iterate through finding where the spaces are and then storing the values inbetween these spaces in a vec3 object
        string lineCopy = line;
        for (int i = 0 ; i < 3 ; i++){
          // we only need to worry about the first number (as a face has format 'f 1/1 2/2 3/3')
          int spaceIndex = lineCopy.find(' ');
          face[i] = stof(lineCopy.substr(spaceIndex + 1,spaceIndex + 2));
          cout << face[i] << "\n";
          int n = lineCopy.length();
          lineCopy = lineCopy.substr(spaceIndex + 1, n);
        }

        //taking the face (so the numbers of which vertices make up the face and storing them in a ModelTriangle object)
        vector<vec3> faceVertices;
        for (int i = 0 ; i < 3 ; i++){
          int index = face[i]; // which number vertex it is
          faceVertices[i] = vertices[index]; // store all 3 vertices in a vector
        }
        }

        // store this as a ModelTriangle object and add it to the output
        ModelTriangle triangle (faceVertices[0], faceVertices[1], faceVertices[2], Colour (1,0,0));
        output.push_back(triangle);
    }
  }

  else cout << "Unable to open file";

  return output;
}
