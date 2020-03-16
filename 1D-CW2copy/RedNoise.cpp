// Note that in this code we use a left-handed coordinate system

/* Already included from OBJStream.cpp! */

#include "OBJStream.cpp"
#include "TriangleDrawing.cpp"
#include <RayTriangleIntersection.h>
#include <assert.h>



using namespace std;
using namespace glm;

#define WIDTH 600  //640
#define HEIGHT 600 //480



void update();
void handleEvent(SDL_Event event);
void render();
void clear();

void drawLine(CanvasPoint start, CanvasPoint end, Colour colour);
void drawStrokedTriangle(CanvasTriangle triangle);
void drawFilledTriangle(CanvasTriangle triangle);
void test(string objFileName, string mtlFileName);


enum RENDERTYPE { WIREFRAME, RASTERIZE, RAYTRACE };

RENDERTYPE renderType = RASTERIZE;

void rasterize();
void initializeDepthMap();
void updateView(string input);

/* FUNCTION Declarations */

void rotateView(string inputString);
void lookAt(vec3 point);
vec3 findCentreOfScene();
void test2();
void raytracer();
Colour solveLight(RayTriangleIntersection closest, vec3 rayDirection);
vec3 createRay(int i, int j);


void rayTest(string objFileName, string mtlFileName);


vector<vec3> checkForIntersections(vec3 rayDirection);
RayTriangleIntersection closestIntersection(vector<vec3> solutions);
float intensityDropOff(vec3 point);
vec3 getNormalOfTriangle(ModelTriangle triangle);
float angleOfIncidence(RayTriangleIntersection intersection);
float distanceVec3(vec3 from, vec3 to);
bool checkForShadows(vec3 point);
float calculateSpecularLight(vec3 point, vec3 rayDirection, vec3 normal);




DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);


// this is where we will store the faces of the OBJ file
vector<ModelTriangle> faces;


// create a global depth map
float depthMap [WIDTH*HEIGHT];




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
  
  initializeDepthMap();

  // the files (scene) which we want to render
  //string objFileName = "cornell-box.obj";
  //string mtlFileName = "cornell-box.mtl";

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

void handleEvent(SDL_Event event) {
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
    else if(event.key.keysym.sym == SDLK_a){
      cout << "PAN RIGHT" << endl;
      rotateView("panRight");
    }
    else if(event.key.keysym.sym == SDLK_s){
      cout << "PAN LEFT" << endl;
      rotateView("panLeft");
    }
    else if(event.key.keysym.sym == SDLK_w){
      cout << "TILT DOWN" << endl;
      rotateView("tiltDown");
    }
    else if(event.key.keysym.sym == SDLK_z){
      cout << "TILT UP" << endl;
      rotateView("tiltUp");
    }
    else if(event.key.keysym.sym == SDLK_t) test("cornell-box.obj", "cornell-box.mtl");
    else if(event.key.keysym.sym == SDLK_y) test2();
    else if(event.key.keysym.sym == SDLK_r) rayTest("cornell-box.obj", "cornell-box.mtl" );
    else if(event.key.keysym.sym == SDLK_c) clear();
    // pressing 1 changes to wireframe mode
    else if(event.key.keysym.sym == SDLK_1){
      clear();
      renderType = WIREFRAME;
      render();
    }
    // pressing 2 changes to rasterize mode
    else if(event.key.keysym.sym == SDLK_2){
      clear();
      renderType = RASTERIZE;
      render();
    }
    // pressing 3 changes to raytrace mode
    else if(event.key.keysym.sym == SDLK_3){
      clear();
      renderType = RAYTRACE;
      render();
    }
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}



// this function renders the scene, depending on what the value of STATE is (so whether we use wireframe, rasterize or raytrace)
void render(){
  if (renderType == RAYTRACE) raytracer();
  else rasterize();
  
}

void clear(){
  window.clearPixels();
  initializeDepthMap();
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

void rasterize(){
  cout << "\n Forward: ";
  printVec3(cameraForward);
  cout << "\n Up: ";
  printVec3(cameraUp);
  cout << "\n Right: ";
  printVec3(cameraRight);
  // for each face
  for (int i = 0 ; i < faces.size() ; i++){
    ModelTriangle triangle = faces[i];
    CanvasTriangle canvasTriangle;
    canvasTriangle.colour = triangle.colour;
    
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = triangle.vertices[j];
      
      // change from world coordinates to camera
      vec3 vertexCSpace = cameraPosition - (cameraOrientation * vertex); // CSpace means camera space
      
      // save the depth of this point for later
      float depth = vertexCSpace[2];

      // calculating the projection onto the 2D image plane by using interpolation and the z depth
      // only worth doing if the vertex is in front of camera
      if (depth > 0){
        float proportion = focalLength / depth;
        vec3 vertexProjected = vertexCSpace * proportion; // the coordinates of the 3D point (in camera space) projected onto the image plane
        float x = -vertexProjected[0];
        float y = vertexProjected[1];

        float xNormalised = (x / imageWidth) + 0.5;
        float yNormalised = (y / imageHeight) + 0.5;
        float xPixel = xNormalised * WIDTH;
        float yPixel = yNormalised * HEIGHT;
        // store the pixel values as a Canvas Point and save it for this triangle
        canvasTriangle.vertices[j] = CanvasPoint(xPixel, yPixel, depth); // we save the depth of the 2D point too

      }
    }

    if (renderType == WIREFRAME) drawStrokedTriangle(canvasTriangle);
    else drawFilledTriangle(canvasTriangle);
  }
}

void initializeDepthMap(){
  for (int i = 0 ; i < (HEIGHT*WIDTH) ; i++)
    depthMap[i] = numeric_limits<float>::infinity();
}


void updateView(string input){

  initializeDepthMap();
  window.clearPixels();
  vec3 direction (0, 0, 0);

  if (input == "up"){
    direction = cameraUp;
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
  cameraPosition = cameraPosition + direction;
  rasterize();
}

void rotateView(string inputString){

  initializeDepthMap();
  window.clearPixels();

  vec3 col1 (0,0,0);
  vec3 col2 (0,0,0);
  vec3 col3 (0,0,0);

  if (inputString == "rollRight"){
    float alpha = -0.5;
    col1 = vec3 (cos(alpha), sin(alpha), 0);
    col2 = vec3 (-sin(alpha), cos(alpha), 0);
    col3 = vec3 (0, 0, 1);
  }

  else if (inputString == "rollLeft"){
    float alpha = 0.5;
    col1 = vec3 (cos(alpha), sin(alpha), 0);
    col2 = vec3 (-sin(alpha), cos(alpha), 0);
    col3 = vec3 (0, 0, 1);
  }

  else if (inputString == "panLeft"){
    float beta = 0.5;
    col1 = vec3 (cos(beta), 0, -sin(beta));
    col2 = vec3 (0, 1, 0);
    col3 = vec3 (sin(beta), 0, cos(beta));
  }

  else if (inputString == "panRight"){
    float beta = -0.5;
    col1 = vec3 (cos(beta), 0, -sin(beta));
    col2 = vec3 (0, 1, 0);
    col3 = vec3 (sin(beta), 0, cos(beta));
  }
  else if (inputString == "tiltDown"){
    float gamma = 0.5;
    col1 = vec3 (1, 0, 0);
    col2 = vec3 (0, cos(gamma), sin(gamma));
    col3 = vec3 (0, -sin(gamma), cos(gamma));
  }

  else if (inputString == "tiltUp"){
    float gamma = -0.5;
    col1 = vec3 (1, 0, 0);
    col2 = vec3 (0, cos(gamma), sin(gamma));
    col3 = vec3 (0, -sin(gamma), cos(gamma));
  }
  
  mat3 rotation (col1, col2, col3);
  cameraOrientation = cameraOrientation * rotation;
  rasterize();
}


void lookAt(vec3 point){
  initializeDepthMap();
  window.clearPixels();

  vec3 direction = point - cameraPosition;
  cameraForward = direction;
  vec3 randomVector (0,1,0);
  cameraRight = glm::cross(randomVector, cameraForward);
  cameraUp = glm::cross(cameraForward, cameraRight);
  
  cameraForward = -normalize(cameraForward);
  cameraRight = -normalize(cameraRight);
  cameraUp = normalize(cameraUp);

  cout << "1: " << glm::dot(cameraForward,cameraUp)<< "\n";
  cout << "2: " << glm::dot(cameraForward, cameraRight)<<"\n";
  
  cameraOrientation = mat3 (cameraRight, cameraUp, cameraForward);

  rasterize();
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

void test(string objFileName, string mtlFileName){
  faces = readOBJ(objFileName, mtlFileName, 1);
  render();
}

void test2(){
  vec3 centre = findCentreOfScene();
  printVec3(centre);
  lookAt(centre);
}

////////////////////////////////////////////////////////
// RAYTRACING CODE
////////////////////////////////////////////////////////


// this runs when you press the letter 'r' - used for testing functions
void rayTest(string objFileName, string mtlFileName){
  faces = readOBJ(objFileName, mtlFileName, 1);
  raytracer();
}

// this will be the main function for the raytracer
void raytracer(){
  // for each pixel
  for (int i = 0 ; i < WIDTH ; i++){
    for (int j = 0 ; j < HEIGHT ; j++){

      // send a ray
      //vec3 point = cameraPosition;
      vec3 rayDirection = createRay(i,j);

      // does this ray intersect any of the faces?
      vector<vec3> solutions = checkForIntersections(rayDirection);
      
      RayTriangleIntersection closest = closestIntersection(solutions);

      // if this ray actually intersects any faces
      if (closest.distanceFromCamera >= 0){
        
        // are we in shadow? only colour the pixel if we are not
        bool inShadow = checkForShadows(closest.intersectionPoint);
        if (not(inShadow)){
          
          Colour colour = solveLight(closest, rayDirection);
        
          window.setPixelColour(i, j, getColour(colour));
        }

        // if we are in shadow add a little bit of ambient light
        if (inShadow){
          Colour colour = closest.intersectedTriangle.colour;
          colour.red = colour.red * 0.05;
          colour.green = colour.green * 0.05;
          colour.blue = colour.blue * 0.05;
          window.setPixelColour(i, j, getColour(colour));
        }
      }
    }
  }
}



Colour solveLight(RayTriangleIntersection closest, vec3 rayDirection){
  /////////////////////////////
  // PARAMETERS
  /////////////////////////////
  float Kd = 0.5; // diffuse constant
  float Ks = 0.5; // specular constant
  /////////////////////////////

  Colour colour = closest.intersectedTriangle.colour;
  vec3 point = closest.intersectionPoint;
  
  // diffuse light
  float diffuseIntensity = intensityDropOff(point) * angleOfIncidence(closest);
  
  // specular light
  vec3 normal = getNormalOfTriangle(closest.intersectedTriangle);
  float specularIntensity = calculateSpecularLight(point, rayDirection, normal);
  
  // average the diffuse and specular intensities
  float intensity = (diffuseIntensity + specularIntensity) / 2;
  if (intensity < 0.1){
    intensity = 0.1;
  }

  // average the diffuse and specular
  // we don't multiply the specular Intensity by a colour because it should be white
  colour.red = (Kd * colour.red * diffuseIntensity) + (Ks * specularIntensity);
  colour.green = (Kd * colour.green * diffuseIntensity) + (Ks * specularIntensity);
  colour.blue = (Kd * colour.blue * diffuseIntensity) + (Ks * specularIntensity);
        
  // clipping the colour for if it goes above 255
  colour.red = glm::min(colour.red, 255);
  colour.green = glm::min(colour.green, 255);
  colour.blue = glm::min(colour.blue, 255);

  return colour;
}



// given the pixel coordinates, this function calculates the direction fot he ray (in vector format)
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
vector<vec3> checkForIntersections(vec3 rayDirection){
  // this is the output vector, for every possible face it stores a possibleSolution
  vector<vec3> solutions;
  // for each face
  int n = faces.size();
  for (int i = 0 ; i < n ; i++){
    ModelTriangle triangle = faces[i];
    // got the following code from the worksheet
    vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
    vec3 SPVector = cameraPosition - triangle.vertices[0];
    mat3 DEMatrix(-rayDirection, e0, e1);
    vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
    solutions.push_back(possibleSolution);
  }
  return solutions;
}


// this function gives back the index of the closest face for a particular ray
RayTriangleIntersection closestIntersection(vector<vec3> solutions){
  RayTriangleIntersection closest; // this is where we store the closest triangle of intersection, the point it intersects and the distance
  closest.distanceFromCamera = -1; // initialize this so we can tell when there are no intersections
  float closestT = numeric_limits<float>::infinity();
  int closestIndex = -1;
  // for each possible solution / for each face
  int n = solutions.size();
  for (int i = 0 ; i < n ; i++){
    
    ModelTriangle triangle = faces[i];
    vec3 possibleSolution = solutions[i];
    float t = possibleSolution[0];
    float u = possibleSolution[1];
    float v = possibleSolution[2];
    
    // if it is actually a solution
    bool bool1 = (t > 0);
    bool bool2 = (0 <= u) && (u <= 1);
    bool bool3 = (0 <= v) && (v <= 1);
    bool bool4 = (u + v) <= 1;
    if (bool1 && bool2 && bool3 && bool4){
      // is it closer than what we currently have?
      if (t < closestT){
        closestT = t;
        closestIndex = i;
        
        // calculating the point of intersection
        vec3 p0 = triangle.vertices[0];
        vec3 p1 = triangle.vertices[1];
        vec3 p2 = triangle.vertices[2];
        vec3 point = p0 + (u * (p1 - p0)) + (v * (p2 - p0));
        vec3 d = point - cameraPosition;
        float distance = sqrt( (d[0]*d[0]) + (d[1] * d[1]) + (d[2] * d[2]));
        closest.intersectionPoint = point;
        closest.distanceFromCamera = distance;
        closest.intersectedTriangle = triangle;
      }
    }
  }
  return closest;
}


////////////////////////////////////////////////////////
// LIGHTING
////////////////////////////////////////////////////////


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


void phongBRDF(vec3 point, vec3 normal){

  /*
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
  */
}
