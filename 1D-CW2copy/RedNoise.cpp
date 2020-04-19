// Note that in this code we use a left-handed coordinate system 
 

// Call Headers [ Safe from double includes ]
#include "OBJ.h"
#include "PPM.h"
#include "Interpolate.h"

#include <Utils.h> 
#include <RayTriangleIntersection.h> 
 
using namespace std; 
using namespace glm;

enum MOVEMENT {UP, DOWN, LEFT, RIGHT, ROLL_LEFT, ROLL_RIGHT, PAN_LEFT, PAN_RIGHT, TILT_UP, TILT_DOWN};
enum RENDERTYPE {WIREFRAME, RASTERIZE, RAYTRACE};

// Press '1' for Wireframe 
// Press '2' for Rasterized 
// Press '3' for Raytraced  


//––---------------------------------//
/* Things You Can Change Are Here */
//––---------------------------------//

RENDERTYPE currentRender = RASTERIZE; //Set default RenderType here. 
std::string defaultPPMFileName = "snapshot.ppm";

#define W 600 //Set desired screen width here. 
#define H 600 //Set desired screen height here.

const int AA = 1; //Set Anti-Aliasing Multiplier here, applied to both x and y so expect ~AA^2 time [eg. 800x800x1 5.43s, 800x800x4 88.7s ~16.4x]

bool displayRenderTime = false;

//Scene we want to render.
string objFileName = "cornell-box.obj"; 
string mtlFileName = "cornell-box.mtl"; 
string texFileName = "texture.ppm";


//––---------------------------------//
/* End Of Things You Can Change Here */
//––---------------------------------//

const int WIDTH = W * AA;
const int HEIGHT = H * AA;

vector<uint32_t> pixelBuffer; 
vector<float> depthMap;
  
void update();
void handleEvent(SDL_Event event);
void playOrPause();
void render(); 
void clear(); 
void drawLine(CanvasPoint start, CanvasPoint end, Colour colour); 
void drawStrokedTriangle(CanvasTriangle triangle); 
void drawFilledTriangle(CanvasTriangle triangle); 
vector<Object> createObjects(vector<ModelTriangle> inputFaces);
vector<string> separateLine(string inputLine); 
void rasterize(); 
void updateView (MOVEMENT movement);
/* FUNCTION Declarations */ 
ImageFile readImage(string fileName); 
void renderImageFile(ImageFile imageFile); 
void lookAt(vec3 point); 
vec3 findCentreOfScene(); 
void raytracer(); 
Colour solveLight(RayTriangleIntersection closest, vec3 rayDirection, float Ka, float Kd, float Ks); 
vec3 createRay(const int i, const int j); 
vector<vector<vec4>> checkForIntersections(vec3 point, vec3 rayDirection);
vector<vec4> faceIntersections(vector<ModelTriangle> inputFaces, vec3 point, vec3 rayDirection);
RayTriangleIntersection closestIntersection(vector<vector<vec4>> solutions, vec3 rayPoint); 
Colour shootRay(vec3 rayPoint, vec3 rayDirection, int depth, float currentIOR); 
Colour getFinalColour(Colour colour, float Ka, float Kd, float Ks); 
float intensityDropOff(const vec3 point); 
float angleOfIncidence(RayTriangleIntersection intersection); 
float distanceVec3(vec3 from, vec3 to); 
bool InShadow(vec3 point); 
float calculateSpecularLight(vec3 point, vec3 rayDirection, vec3 normal);
float softShadows(RayTriangleIntersection intersection);
Colour mirror(RayTriangleIntersection intersection, vec3 incident);
Colour glass(vec3 rayDirection, RayTriangleIntersection closest, int depth);
vec4 refract(vec3 I, vec3 N, float ior);
float fresnel(vec3 incident, vec3 normal, float ior);
void backfaceCulling(vec3 rayDirection);
vector<ModelTriangle> boundingBox(vector<ModelTriangle> inputFaces);
void spin(vec3 point, float angle, float distance);
void spinAround(float angle, int stepNumber, bool clockwise, int zoom);
void translateVertices(int objectIndex, vec3 direction, float distance);
void jump(int objectIndex, float height);
void squash(int objectIndex, float squashFactor);
void pixarJump(int objectIndex, float height, bool rotate, float maxSquashFactor);
void jumpSquash(int objectIndex, float maxSquashFactor);
void bounce(int objectIndex, float height, int numberOfBounces);
vec3 findObjectCentre(Object object);
void rotateObject(int objectIndex, float theta, vec3 point);
 
DrawingWindow window;
 
// this stores the faces split up into separate objects
vector<Object> objects;
 
ImageFile textureFile;

bool animate = false;
 
// initial camera parameters 
vec3 cameraPosition (0,2,3.5);//(0,-2,-3.5); 
vec3 cameraRight (1,0,0); 
vec3 cameraUp (0,1,0);//(0,-1,0); 
vec3 cameraForward (0,0,1);//(0,0,-1); // this is actually backwards 
mat3 cameraOrientation (cameraRight, cameraUp, cameraForward); // this creates a matrix with each entry as separate columns 

float focalLength = 1; 
float imageWidth = 2; // WIDTH 
float imageHeight = imageWidth * (HEIGHT / float(WIDTH)); // HEIGHT 

// light parameters 
vec3 lightPosition (-0.234011, 5, -3); // this is roughly the centre of the white light box 
float lightIntensity = 100; 

void initialise() {
  if (!(AA>=1)) exit(1);
  //1) Create Drawing Window.
  window = DrawingWindow(W, H, false); 
  window.clearPixels();
  
  //2) Initialise Depth Map.
  for (int i=0; i< WIDTH*HEIGHT; i++) {
    depthMap.push_back(std::numeric_limits<float>::infinity());
    if (AA > 1) pixelBuffer.push_back(0);
  }
}

int main(int argc, char* argv[]) { 
  // 1) Initialise.
  initialise();

  // 2) Read In OBJ.
  objects = readGroupedOBJ(objFileName, mtlFileName, 1);
  //objects = readGroupedOBJ("logo.obj", "logo.mtl", 0.07);
  
  cout << "Number Of Objects: " << objects.size() << "\n";
  // 3) Read In Texture.
  textureFile = importPPM(texFileName);

  // 4) Create textures

  // || Mirrored floor ||
  //faces[6].texture = "mirror"; 
  //faces[7].texture = "mirror";  

  //for (int i=8; i<10; i++) {
  //  faces[i].texture = "texture";
  //}

  // || Glass Red Box ||
  //for (int i = 12 ; i < 22 ; i++){
  //  faces[i].texture = "glass";
  //}
  
  //objects[0].faces[10].texture = "texture";
  //objects[0].faces[10].vertices_textures[0] = vec2(0.4, 0.2);
  //objects[0].faces[10].vertices_textures[1] = vec2(0.2, 0.4);
  //objects[0].faces[10].vertices_textures[2] = vec2(0.6, 0.4);
  
  render();

  SDL_Event event;
  while(true) { 
    // We MUST poll for events - otherwise the window will freeze ! 
    if(window.pollForInputEvents(&event)) handleEvent(event);
      if (animate){
        update();
        animate = false;
    }
 
    // Need to render the frame at the end, or nothing actually gets shown on the screen ! 
    window.renderFrame(); 
  } 
} 
 
 
 
void update() {
  // Function for performing animation (shifting artifacts or moving the camera)
  const float pi = 4 * atan(1);
  spinAround(pi, 100, true, -1);
  spinAround(pi, 100, true, 1);
} 

// this function starts and stops the animation in the window (press p)
void playOrPause(){
  if (animate){
    animate = false;
  }
  if (not(animate)){
    animate = true;
  }
}

// OPTIMISED - this function sets the screen pixel buffer. (if AA multiplier == 1 -> buffer is ignored.)
void SetBufferColour(int x, int y, uint32_t col) {
  if (AA == 1) {
    //No need to use buffer.
    window.setPixelColour(x, y, col);
  } else {
    //Store to Buffer
    const int i = x + (y*WIDTH);
    pixelBuffer[i] = col;
  }
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
  clear();

  //Initialise Timer.
  std::clock_t start = std::clock();

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

  /* pixelBuffer ---> Display, iif AAMultiplier is greater than 1.*/
  if (AA > 1) {
    for (int j=0; j<HEIGHT; j+=AA) {
      for (int i=0; i<WIDTH; i+=AA) {
        /* average [ i j ||  i+1 j ||  i j+1 || i+1 j+1 ] per channel. */
        
        //AA * AA Block.
        int R=0, G=0, B=0;
        for (int jj=0; jj<AA; jj++){
          for (int ii=0; ii<AA; ii++){
            int index = (i + ii) + (WIDTH*(j + jj)); //index = x + (WIDTH * y)
            R += getRedValueFromColor(pixelBuffer[index]);
            G += getGreenValueFromColor(pixelBuffer[index]);
            B += getBlueValueFromColor(pixelBuffer[index]);
          }
        }
        R /= (AA*AA);
        G /= (AA*AA);
        B /= (AA*AA);
        uint32_t colour = (255<<24) + (int(R)<<16) + (int(G)<<8) + int(B); 
        window.setPixelColour(i/AA, j/AA, colour);
      }
    }
  }

  double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  if (displayRenderTime) cout << "Time Taken To Render: " << duration << "\n";
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
    else if(event.key.keysym.sym == SDLK_c)     bounce(1, 1.5, 5);
    else if(event.key.keysym.sym == SDLK_p)     playOrPause();

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
    else if(event.key.keysym.sym == SDLK_n) {
      ImageFile displaySnapShot = CreateImageFileFromWindow(window, WIDTH, HEIGHT);
      exportToPPM(defaultPPMFileName, displaySnapShot); 
    }
    else if(event.key.keysym.sym == SDLK_m) {
      ImageFile imageFile = importPPM("texture.ppm");
      renderImageFile(imageFile);
    }
  } 
} 
 
void clear(){ 
  window.clearPixels(); 
  for (int i = 0; i < (HEIGHT*WIDTH); i++) 
    depthMap[i] = numeric_limits<float>::infinity(); 
  
  if (AA > 1) 
    for (int i = 0 ; i < (HEIGHT*WIDTH); i++) 
      pixelBuffer[i] = 0;
} 
 

void setDepthPixelColour(int x, int y, double z, uint32_t clr) { 
  const int index = (WIDTH*(y)) + x; 
  if ((x > 0) && (y > 0) && (x < WIDTH) && (y< HEIGHT) && (z < depthMap.at(index))) {
    SetBufferColour(x,y, clr);
    depthMap.at(index) = z;
  }
};

// use this function to split the faces into separate objects (an 'Object' object has been created)
// if you want, you can store a bounding box with the object too
vector<Object> createObjects(vector<ModelTriangle> inputFaces) {
  // how many objects do you want?
  vector<ModelTriangle> objectFaces1;
  vector<ModelTriangle> objectFaces2;


  // for each face, split it up into objects
  for (int i = 0 ; i < inputFaces.size() ; i++){
    ModelTriangle face = inputFaces[i];

    // red box
    if ((i > 11) && (i < 22)){
      objectFaces2.push_back(inputFaces[i]);
    }
    // the rest of the faces
    else {
      objectFaces1.push_back(inputFaces[i]);
    }
  }

  // create the objects
  Object object1 (objectFaces1);
  Object object2 (objectFaces2);
  
  vector<Object> outputVec;
  outputVec.push_back(object1);
  outputVec.push_back(object2);


  // for each object, sort out the face indices
  for (int i = 0 ; i < outputVec.size() ; i++){
    Object object = outputVec[i];
    for (int j = 0 ; j < object.faces.size() ; j++){
      outputVec[i].faces[j].faceIndex = j;
    }
  }

  return outputVec;
}
   
void updateView (MOVEMENT movement) {
  vec3 col1, col2, col3;

  bool move = false;
  bool rotate = false;

  float theta = -0.5;

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
      col1 = vec3 (cos(theta), sin(theta), 0); 
      col2 = vec3 (-sin(theta), cos(theta), 0); 
      col3 = vec3 (0, 0, 1);
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;
    case ROLL_RIGHT:
      col1 = vec3 (cos(-theta), sin(-theta), 0); 
      col2 = vec3 (-sin(-theta), cos(-theta), 0); 
      col3 = vec3 (0, 0, 1);
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;
    case PAN_LEFT:
      col1 = vec3 (cos(-theta), 0, -sin(-theta)); 
      col2 = vec3 (0, 1, 0); 
      col3 = vec3 (sin(-theta), 0, cos(-theta));
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;
    case PAN_RIGHT:
      col1 = vec3 (cos(theta), 0, -sin(theta)); 
      col2 = vec3 (0, 1, 0); 
      col3 = vec3 (sin(theta), 0, cos(theta));
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;      
    case TILT_UP:
      col1 = vec3 (1, 0, 0); 
      col2 = vec3 (0, cos(-theta), sin(-theta)); 
      col3 = vec3 (0, -sin(-theta), cos(-theta));
      rotate = true;
      //cameraOrientation *= mat3(col1, col2, col3); 
      break;
    case TILT_DOWN:
      col1 = vec3 (1, 0, 0); 
      col2 = vec3 (0, cos(theta), sin(theta)); 
      col3 = vec3 (0, -sin(theta), cos(theta));
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

  /*
  // if the camera position has changed we need to redo the culling of faces
  if (move){
    for (int o = 0; o< objects.size(); o++)
      for (int i = 0 ; i < objects[o].faces.size(); i++)
        objects[o].faces[i].culled = false;
    
    // cull them
    backfaceCulling(cameraPosition);
  }
  */

  render();
} 
 
void lookAt(vec3 point){
  vec3 direction = cameraPosition - point;
  direction = normalize(direction);
  cameraForward = direction; 
  vec3 randomVector (0,1,0); 
  cameraRight = glm::cross(randomVector, cameraForward); 
  cameraUp = glm::cross(cameraForward, cameraRight); 
  

  cameraForward = normalize(cameraForward); 
  cameraRight = normalize(cameraRight); 
  cameraUp = normalize(cameraUp); 
   
  cameraOrientation = mat3 (cameraRight, cameraUp, cameraForward); 
 
  render(); 
} 
 
// this function averages all the vertices in the scene to find the centre of the scene 
vec3 findCentreOfScene(){ 
  int n=0; //overall faces count.

  vec3 sum (0,0,0); 
  for (int o=0; o<objects.size(); o++){
    for (int i = 0 ; i < objects[o].faces.size(); i++){ 
      ModelTriangle face = objects[o].faces[i]; 
      // for each vertex 
      for (int j = 0 ; j < 3 ; j++){ 
        sum += face.vertices[j]; 
      } 
    }
    n += objects[o].faces.size(); 
  }
  
  sum /= (float)(n*3); 
  return (sum); 
} 
  
void renderImageFile(ImageFile imageFile){  
  clear();
  for (int i=0; i<imageFile.vecPixelList.size(); i++){ 
    int row = int(i/imageFile.width); 
    int col = i - (row*imageFile.width); 
    SetBufferColour(col, row, imageFile.vecPixelList.at(i).toUINT32_t()); 
  } 
} 
 
//////////////////////////////////////////////////////// 
// RASTERIZING CODE 
//////////////////////////////////////////////////////// 
// draws a 2D line from start to end (colour is in (r,g,b) format with 255 as max) 

void drawLine(CanvasPoint start, CanvasPoint end, Colour colour) { 
  const float diffX = end.x - start.x; 
  const float diffY = end.y - start.y; 
  const float numberOfSteps = glm::max(abs(diffX),abs(diffY)); 
 
  const uint32_t col = colour.toUINT32_t(); 
 
  // if we are starting and ending on the same pixel 
  if (numberOfSteps == 0) setDepthPixelColour(start.x, start.y, glm::min(start.depth, end.depth), col); 
  else { 
    const float stepSizeX = diffX / numberOfSteps; 
    const float stepSizeY = diffY / numberOfSteps; 
 
    // for each pixel across 
    for (int i = 0; i <= numberOfSteps; i++){        
      int x = round(start.x + (i * stepSizeX)); 
      int y = round(start.y + (i * stepSizeY)); 

      // interpolate to find the current depth of the line 
      const float proportion = i / numberOfSteps; 
      const float inverseDepth = ((1 - proportion) * (1 / start.depth)) + (proportion * (1 / end.depth)); // got this equation from notes 
      const float depth = 1 / inverseDepth; 
      setDepthPixelColour(x, y, depth, col);
    } 
  } 
} 

void drawLine(CanvasPoint start, CanvasPoint end, vector<TexturePoint> texturePoints) { 
  const float diffX = end.x - start.x; 
  const float diffY = end.y - start.y; 
  const float numberOfSteps = glm::max(abs(diffX),abs(diffY)); 
 
  // if we are starting and ending on the same pixel 
  if (numberOfSteps == 0) setDepthPixelColour(start.x, start.y, glm::min(start.depth, end.depth), getPixelColour(&textureFile, texturePoints.at(0).x, texturePoints.at(0).y).toUINT32_t()); 
  else { 
    const float stepSizeX = diffX / numberOfSteps; 
    const float stepSizeY = diffY / numberOfSteps; 
 
    // for each pixel across 
    for (int i = 0; i <= numberOfSteps ; i++){        
      int x = round(start.x + (i * stepSizeX)); 
      int y = round(start.y + (i * stepSizeY)); 

      // interpolate to find the current depth of the line 
      const float proportion = i / numberOfSteps; 
      const float inverseDepth = ((1 - proportion) * (1 / start.depth)) + (proportion * (1 / end.depth)); // got this equation from notes 
      const float depth = 1 / inverseDepth; 
      //cout << "Requesting [x,y, i]" << i << "\n";
      setDepthPixelColour(x, y, depth, getPixelColour(&textureFile, texturePoints.at(i).x, texturePoints.at(i).y).toUINT32_t());
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
  if (steps == 0) drawLine(middlePoint, maxPoint, triangle.colour); 
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
  if (steps2 == 0) drawLine(minPoint, middlePoint, triangle.colour); 
  else { 
    for (int i = steps2; i >= 0; i--){ 
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
 
void drawTexturedTriangle(ImageFile *imageFile, CanvasTriangle triangle) {
  // sort vertices.
  triangle = sortTriangleVertices(triangle);

  CanvasPoint topPoint = triangle.vertices[0];
  CanvasPoint middlePoint = triangle.vertices[1];
  CanvasPoint lowestPoint = triangle.vertices[2];
  // the vertical distance between the highest point and the middle point
  float yDistance = middlePoint.y - topPoint.y;
  // interpolating to find the cutting point
  float maxYDistance = lowestPoint.y - topPoint.y;
  float maxXDistance = lowestPoint.x - topPoint.x;
  float yProportion = yDistance / maxYDistance; // the proportion of how far along in the y the point should be
  float xDistance = topPoint.x + (yProportion * maxXDistance);
  
  // interpolate to find the right depth too
  // because of perspective projection we interpolate using 1/depth of everything
  float inverseDepth = ((1 - yProportion) * (1 / topPoint.depth)) + (yProportion * (1 / lowestPoint.depth)); // equation from notes
  float cutterDepth = (1 / inverseDepth);
  CanvasPoint cutterPoint(xDistance, topPoint.y + yDistance, cutterDepth);

  //Change in X from lowestPoint to TopPoint.
  const float bigDX = lowestPoint.x - topPoint.x;
  //Change in Y from lowestPoint to TopPoint.
  const float bigDY = lowestPoint.y - topPoint.y;

  //Length of line from topPoint to lowestPoint.
  const float bigHypo = sqrtf( ((bigDX * bigDX) + (bigDY * bigDY)) );

  if (bigHypo == 0) {
    cout << "bigHypo = 0\n";
    return;
  }

  //Length of line from topPoint to cutterPoint.
  const float smallHypo = sqrtf(( pow(cutterPoint.x - topPoint.x, 2) + pow(cutterPoint.y - topPoint.y, 2) ));
  
  //Calculate proportion [ how far down the line cutterPoint is ]
  const float p = smallHypo/bigHypo;

  //cout << "\np: " << p << "\n";

  //Get texture cut point using proportion & SOHCAHTOA.

  //Change in X from lowestPoint texture to TopPoint texture.
  const float t_bigDX = lowestPoint.texturePoint.x - topPoint.texturePoint.x;
  //Change in Y from lowestPoint texture to TopPoint texture.
  const float t_bigDY = lowestPoint.texturePoint.y - topPoint.texturePoint.y;
  //Length of line from topPoint texture to lowestPoint texture.
  const float t_bigHypo = sqrtf( ((t_bigDX * t_bigDX) + (t_bigDY * t_bigDY)) );
  //Get length of line from topPoint texture to cutterPoint texture.
  const float t_smallHypo = p * t_bigHypo;

  const float t_smallDX = abs( t_smallHypo * (t_bigDX / t_bigHypo) ); //t_smallHypo * sin alpha (of bigger triangle)
  const float t_smallDY = abs( t_smallHypo * (t_bigDY / t_bigHypo) ); //t_smallHypo * cos alpha (of bigger triangle)

  float cutTextureX = (topPoint.texturePoint.x <= lowestPoint.texturePoint.x) ? (topPoint.texturePoint.x + t_smallDX) : (topPoint.texturePoint.x - t_smallDX);
  float cutTextureY = (topPoint.texturePoint.y <= lowestPoint.texturePoint.y) ? (topPoint.texturePoint.y + t_smallDY) : (topPoint.texturePoint.y - t_smallDY);

  cutterPoint.texturePoint = TexturePoint(cutTextureX, cutTextureY);

  CanvasPoint leftPoint = (middlePoint.x < cutterPoint.x) ? middlePoint : cutterPoint;
  CanvasPoint rightPoint = (middlePoint.x < cutterPoint.x) ? cutterPoint : middlePoint;

  //cout << "topPoint   : ";  print(topPoint);    print(topPoint.texturePoint);    cout << "\n";
  //cout << "leftPoint  : ";  print(leftPoint);   print(leftPoint.texturePoint);   cout << "\n";
  //cout << "rightPoint : ";  print(rightPoint);  print(rightPoint.texturePoint);  cout << "\n";
  //cout << "lowestPoint: ";  print(lowestPoint); print(lowestPoint.texturePoint); cout << "\n";

  vector<CanvasPoint> pointList;
  pointList.push_back(topPoint);
  pointList.push_back(leftPoint);
  pointList.push_back(lowestPoint);

  //** 4.1. Fill the Top Flat Bottom Triangle.
  //textureFlatBottomTriangle(imageFile, CanvasTriangle(topPoint, leftPoint, rightPoint), closestPoint, furthestPoint);
  

  float steps = glm::abs(middlePoint.y - topPoint.y); // how many rows 
  vector<TexturePoint> t_interpLeft = interpolate(topPoint.texturePoint, leftPoint.texturePoint, steps);
  vector<TexturePoint> t_interpRight = interpolate(topPoint.texturePoint, rightPoint.texturePoint, steps);

  // if the two vertices are on the same y line, then just draw a line between the two 
  if (steps == 0) 
    cout << "Line880\n"; //drawLine(middlePoint, topPoint, triangle.colour); 
  else { 
    for (int i = 0 ; i < steps ; i++){ 
      // find the two points which intersect this row 
      float proportion = i / steps; 
      float maxXDiff1 = topPoint.x - middlePoint.x; 
      float maxXDiff2 = topPoint.x - cutterPoint.x; 
      float xStart = round(topPoint.x - (proportion * maxXDiff1)); 
      float xEnd = round(topPoint.x - (proportion * maxXDiff2)); 
       
      // interpolating to find the right depth of both of the points 
      // point1: 
      float inverseDepthPoint1 = ((1 - proportion) * (1 / topPoint.depth)) + (proportion * (1 / middlePoint.depth)); 
      float depthPoint1 = 1 / inverseDepthPoint1; 
      // point2: 
      float inverseDepthPoint2 = ((1 - proportion) * (1 / topPoint.depth)) + (proportion * (1 / cutterPoint.depth)); 
      float depthPoint2 = 1 / inverseDepthPoint2; 
      CanvasPoint start(xStart, topPoint.y + i, depthPoint1); 
      CanvasPoint end(xEnd, topPoint.y + i, depthPoint2);
      
      int noOfSteps = glm::abs(end.x - start.x) + 1;
      vector<TexturePoint> interpTexLine = ((end.x - start.x) >= 0) ? interpolate(t_interpLeft[i], t_interpRight[i], noOfSteps) : interpolate(t_interpRight[i], t_interpLeft[i], noOfSteps); 
      drawLine(start, end, interpTexLine); 

    } 
  } 
  
  
  
  //** 4.2. Fill the Bottom Flat Top Triangle.
  //textureFlatTopTriangle(imageFile, CanvasTriangle(leftPoint, rightPoint, lowestPoint), closestPoint, furthestPoint);


  // the lower triangle 
  // for each row, fill it in 
  float steps2 = glm::abs(lowestPoint.y - leftPoint.y);
  t_interpLeft = interpolate(leftPoint.texturePoint, lowestPoint.texturePoint, steps2);
  t_interpRight = interpolate(rightPoint.texturePoint, lowestPoint.texturePoint, steps2);

  if (steps2 == 0) 
    cout << "Line 942\n"; //drawLine(minPoint, middlePoint, triangle.colour); 
  else { 
    for (int i = steps2; i >= 0; i--){ 
      // find the two points which intersect this row 
      float proportion = 1 - (i / steps2); // proportion going upwards from the min point to the middle 
      float maxXDiff1 = lowestPoint.x - middlePoint.x; 
      float maxXDiff2 = lowestPoint.x - cutterPoint.x; 
      float xStart = round(lowestPoint.x - (proportion * maxXDiff1)); 
      float xEnd = round(lowestPoint.x - (proportion * maxXDiff2)); 
       
      // interpolating to find the right depth 
      // point1: 
      float inverseDepthPoint1 = ((1 - proportion) * (1 / lowestPoint.depth)) + (proportion * (1 / middlePoint.depth)); 
      float depthPoint1 = 1 / inverseDepthPoint1; 
      // point2: 
      float inverseDepthPoint2 = ((1 - proportion) * (1 / lowestPoint.depth)) + (proportion * (1 / cutterPoint.depth)); 
      float depthPoint2 = 1 / inverseDepthPoint2; 
      CanvasPoint start(xStart, cutterPoint.y + i, depthPoint1); 
      CanvasPoint end(xEnd, cutterPoint.y + i, depthPoint2); 
      
      int noOfSteps = glm::abs(end.x - start.x) + 1;
      vector<TexturePoint> interpTexLine = ((end.x - start.x) >= 0) ? interpolate(t_interpLeft[i], t_interpRight[i], noOfSteps) : interpolate(t_interpRight[i], t_interpLeft[i], noOfSteps); 
      drawLine(start, end, interpTexLine); 
    } 
  } 

}

void rasterize(){  
  //for each object.
  for (int o = 0; o < objects.size(); o++){
    // for each face 
    for (int i = 0 ; i < objects[o].faces.size() ; i++){ 
      ModelTriangle triangle = objects[o].faces[i]; 
      CanvasTriangle canvasTriangle; 
      canvasTriangle.colour = triangle.colour;
      canvasTriangle.textured = (triangle.texture == "texture");

      // for each vertex 
      for (int j = 0 ; j < 3 ; j++){ 
        vec3 vertex = triangle.vertices[j]; 
        
        // change from world coordinates to camera
        vertex = vertex - cameraPosition;
        mat3 rotationMatrix = glm::inverse(mat3(cameraRight, cameraUp, cameraForward));
        vec3 vertexCSpace = rotationMatrix * vertex;
        vertexCSpace[2] = -vertexCSpace[2];

        // save the depth of this point for later 
        float depth = vertexCSpace[2]; 
  
        // calculating the projection onto the 2D image plane by using interpolation and the z depth 
        // only worth doing if the vertex is in front of camera 
        if ((depth < 0) || (depth > 0)){ 
          float proportion = focalLength / depth; 
          vec3 vertexProjected = vertexCSpace * proportion; // the coordinates of the 3D point (in camera space) projected onto the image plane 

          // converting to get the pixel numbers
          // finding position of top left corner of image plane in camera space
          vec3 topLeft = vec3 (-imageWidth/2, imageHeight/2, focalLength);//(focalLength * cameraForward) + ((imageHeight/2) * cameraUp) - ((imageWidth/2) * cameraRight);
          vec3 topLeftToPoint = vertexProjected - topLeft;
          float xPixel = (topLeftToPoint[0] / imageWidth) * WIDTH;
          float yPixel = (-topLeftToPoint[1] / imageHeight) * HEIGHT;
    
          // store the pixel values as a Canvas Point and save it for this triangle 
          canvasTriangle.vertices[j] = CanvasPoint(xPixel, yPixel, depth); // we save the depth of the 2D point too 

          //if the triangle is textured, get the texture uv's and multiply them by the texture WIDTH, HEIGHT to get texture X,Y.
          if (canvasTriangle.textured) canvasTriangle.vertices[j].texturePoint = TexturePoint(triangle.vertices_textures[j].x * textureFile.width, triangle.vertices_textures[j].y * textureFile.height);
        } 
      } 


      if (currentRender == WIREFRAME) drawStrokedTriangle(canvasTriangle); 
      else {
        if (canvasTriangle.textured) drawTexturedTriangle(&textureFile, canvasTriangle);
        else drawFilledTriangle(canvasTriangle); 
        //drawFilledTriangle(canvasTriangle); 
      }
    } 
  }
}  

//////////////////////////////////////////////////////// 
// RAYTRACING CODE 
//////////////////////////////////////////////////////// 
  

// this will be the main function for the raytracer 
void raytracer(){
  // for each pixel 
  for (int i = 0 ; i < WIDTH ; i++){ 
    //#pragma clang loop vectorize_width(8) interleave_count(8)
    for (int j = 0 ; j < HEIGHT ; j++){
      // create a ray 
      vec3 rayDirection = createRay(i,j);
      // shoot the ray and check for intersections 
      Colour colour = shootRay(cameraPosition, rayDirection, 0, 1); // depth starts at 0, IOR is 1 as travelling in air
      // colour the pixel accordingly 
      SetBufferColour(i, j, colour.toUINT32_t()); 
    } 
  } 
} 
 
Colour solveLight(RayTriangleIntersection closest, vec3 rayDirection, float Ka, float Kd, float Ks) { 
  // diffuse light 
  Kd *= (intensityDropOff(closest.intersectionPoint) * angleOfIncidence(closest));
  // specular light 
  //closest.normal -  this is the interpolated noral for Phong shading (it is previously calculated and stored in the RayTriangleIntersection object) 
  Ks *= calculateSpecularLight(closest.intersectionPoint, rayDirection, closest.normal); 
  return getFinalColour(closest.intersectedTriangle.colour, Ka, Kd, Ks); 
} 
 


//OPTIMISED - Compute only once outside the loop. 
//const vec2 pixelSize(imageWidth/WIDTH, imageHeight/HEIGHT)
const float pixelSizeX = imageWidth/WIDTH;
const float pixelSizeY = imageHeight/HEIGHT;

// given the pixel coordinates, this function calculates the direction of the ray (in vector format) 
vec3 createRay(const int i, const int j) {
  //vec2 pixel(float(i)+0.5,float(j)+0.5); //pixel = pixel + vec2(0.5,0.5); 
  //vec2 distanceFromCentre = pixel - vec2(WIDTH/2, HEIGHT/2); // distance from the pixel to the centre of the image plane in terms of number of pixels 
  const float horizontalDistance = pixelSizeX * (float(i) + 0.5 - (WIDTH/2)); //pixel[0] * distanceFromCentre[0]
  const float verticalDistance = pixelSizeY * (float(j) + 0.5 - (HEIGHT/2));  //pixel[1] * distanceFromCentre[1]
  const vec3 imagePlaneCentre = cameraPosition - (focalLength * cameraForward); // this is the 3D coordinate of the centre of the image plane 
  const vec3 point = imagePlaneCentre + (horizontalDistance * cameraRight) + (verticalDistance * (-cameraUp)); 
  return normalize(point - cameraPosition); //Return direction.
} 
 
 
// this function takes the ray and checks with all the faces to see which ones it intersects 
// it outputs a vector of vectors - a (t,u,v) vector for each face 
// if t>0, then that means that for that particular face the ray intersects it with u and v being local coordinates for the triangle 
vector<vector<vec4>> checkForIntersections(vec3 point, vec3 rayDirection){ 
  // this is the output vector, for every possible face it stores a possibleSolution 
  // the 4th value in this evctor is the index of the face if there is an intersection
  // there is an element in solutions ofr every object
  // each element in solutions is a vector of possible solutions for all the faces in that object
  vector<vector<vec4>> solutions;

  // for each object, does it have a bounding box?
  // if so, does the ray intersect it?
  // if so, which of the faces in that object does the ray intersect?
  for (int i = 0 ; i < objects.size(); i++) {
    vector<vec4> objectSolutions; // to store the solutions if any for this object
    objectSolutions.push_back(vec4 (-1,0,0,-1));
    if (objects[i].hasBoundingBox){
      vector<vec4> boxSolutions = faceIntersections(objects[i].boxFaces, point, rayDirection);
      // do we intersect the bounding box?
      for (int j = 0; j < boxSolutions.size(); j++){
        // if we have an intersection, then check for intersections with all the faces
        if (boxSolutions[j][0] > 0){
          solutions.push_back(faceIntersections(objects[i].faces, point, rayDirection));
          break;
        }
      }
    }
    // if this object doesn't have a bounding box, then just check each face as normal
    else {
      solutions.push_back(faceIntersections(objects[i].faces, point, rayDirection));
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
      //add possiblesolution to solution list.
      solutions.push_back( vec4(possibleSolution[0], possibleSolution[1], possibleSolution[2], triangle.faceIndex)); 
    }
    // if it has been culled then return a fake solution
    else {
      solutions.push_back(vec4 (-1,0,0,-1));
    } 
  } 
  return solutions; 
}

// this function gives back the index of the closest face for a particular ray 
RayTriangleIntersection closestIntersection(vector<vector<vec4>> objectSolutions, vec3 rayPoint) { 
  RayTriangleIntersection closest; // this is where we store the closest triangle of intersection, the point it intersects and the distance 
  closest.distanceFromCamera = -1; // initialize this so we can tell when there are no intersections 
  float closestT = numeric_limits<float>::infinity(); 
  int closestIndex = -1; 
  // for each possible solution / for each face 
  int n = objectSolutions.size(); 
  // for each object
  for (int o=0; o<n; o++) {
    vector<vec4> solutions = objectSolutions[o];
    for (int i = 0 ; i < objects[o].faces.size() ; i++){ 
      
      vec4 possibleSolution = solutions[i]; 
      float t = possibleSolution[0]; 
      float u = possibleSolution[1]; 
      float v = possibleSolution[2];
      int index = possibleSolution[3];
      
      // if it is actually a solution 
      bool bool1 = (t > 0); 
      bool bool2 = (0 <= u) && (u <= 1) && (0 <= v) && (v <= 1) ; 
      bool bool3 = (u + v) <= 1; 
      if (bool1 && bool2 && bool3){ 
        ModelTriangle triangle = objects[o].faces[index]; 
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
  // cull the faces
  backfaceCulling(rayDirection);
  // stop recursing if our reflections get too much
  if (depth == 7) return Colour(255,255,255);  

  // does this ray intersect any of the faces?
  vector<vector<vec4>> solutions = checkForIntersections(rayPoint, rayDirection); 
  // find the closest intersection
  RayTriangleIntersection closest = closestIntersection(solutions, rayPoint); 
  Colour colour = closest.intersectedTriangle.colour; 
  vec3 point = closest.intersectionPoint; 
 
  // if this ray doesn't intersect anything, then we return the colour black 
  if (closest.distanceFromCamera <= 0) return Colour(0,0,0); 
  
 
  // the ambient, diffuse and specular light constants 
  float Ka = 0.2, Kd = 0.4, Ks = 0.4; 
 
  ModelTriangle triangle = closest.intersectedTriangle;

  // if this face is a mirror, create a reflected ray and shoot this ray (recurse this function) 
  if (triangle.texture == "mirror"){ 
    vec3 incident = rayDirection; 
    vec3 normal = closest.intersectedTriangle.getNormal(); 
    vec3 reflection = normalize(incident - (2 * dot(incident, normal) * normal));
    // avoid self-intersection 
    return shootRay(point + ((float)0.00001 * normal), reflection, depth + 1, currentIOR);
  } 
  else if (triangle.texture == "glass"){
    return glass(rayDirection, closest, depth);
  }
  else if (triangle.texture == "texture") {
    bool mydebug = true; //(closest.intersectUV[0] > 0.2 && closest.intersectUV[1] > 0.2);

    if (mydebug) {
      cout << "How Far Along X: " << closest.intersectUV[0] << "\n";
      cout << "How Far Along Y: " << closest.intersectUV[1] << "\n";
     
      cout << "Texture [0]: " << closest.intersectedTriangle.vertices_textures[0].x << ", " << closest.intersectedTriangle.vertices_textures[0].y << "\n";
      cout << "Texture [1]: " << closest.intersectedTriangle.vertices_textures[1].x << ", " << closest.intersectedTriangle.vertices_textures[1].y << "\n";
      cout << "Texture [2]: " << closest.intersectedTriangle.vertices_textures[2].x << ", " << closest.intersectedTriangle.vertices_textures[2].y << "\n";

      cout << "V [0]: " << closest.intersectedTriangle.vertices[0].x << ", " << closest.intersectedTriangle.vertices[0].y << "\n";
      cout << "V [1]: " << closest.intersectedTriangle.vertices[1].x << ", " << closest.intersectedTriangle.vertices[1].y << "\n";
      cout << "V [2]: " << closest.intersectedTriangle.vertices[2].x << ", " << closest.intersectedTriangle.vertices[2].y << "\n";
    }

    int minIndexX = min_index(closest.intersectedTriangle.vertices[0].x, closest.intersectedTriangle.vertices[1].x, closest.intersectedTriangle.vertices[2].x);
    int minIndexY = min_index(closest.intersectedTriangle.vertices[0].y, closest.intersectedTriangle.vertices[1].y, closest.intersectedTriangle.vertices[2].y);
    int maxIndexX = max_index(closest.intersectedTriangle.vertices[0].x, closest.intersectedTriangle.vertices[1].x, closest.intersectedTriangle.vertices[2].x);
    int maxIndexY = max_index(closest.intersectedTriangle.vertices[0].y, closest.intersectedTriangle.vertices[1].y, closest.intersectedTriangle.vertices[2].y);

    if (mydebug) cout << "|MinX " << minIndexX << "|MaxX " << maxIndexX << "|MinY " << minIndexY << "|Max Y " << maxIndexY << "\n";

    // texX : from minIndex closest.intersectedTriangle.vertices_textures[minIndex].x
    // texX :   to maxIndex closest.intersectedTriangle.vertices_textures[maxIndex].x 
    // texY : from minIndex closest.intersectedTriangle.vertices_textures[minIndex].y
    // texY :   to maxIndex closest.intersectedTriangle.vertices_textures[maxIndex].y 
    //cout << "Intersect UV: " << closest.intersectUV[0] << ", " << closest.intersectUV[1] << "\n";
    int tX = textureFile.width;
    int tY = textureFile.height;
    
    float texX;
    float texY;
    if (mydebug) {
      texX = getValueBetweenNumbers(true, tX * closest.intersectedTriangle.vertices_textures[minIndexX].x, tX* closest.intersectedTriangle.vertices_textures[maxIndexX].x, closest.intersectUV[0]);
      texY = getValueBetweenNumbers(true, tY * closest.intersectedTriangle.vertices_textures[minIndexY].y, tY*closest.intersectedTriangle.vertices_textures[maxIndexY].y, closest.intersectUV[1]);
    } else {
      texX = getValueBetweenNumbers(false, tX * closest.intersectedTriangle.vertices_textures[minIndexX].x, tX * closest.intersectedTriangle.vertices_textures[maxIndexX].x, closest.intersectUV[0]);
      texY = getValueBetweenNumbers(false, tY * closest.intersectedTriangle.vertices_textures[minIndexY].y, tY *closest.intersectedTriangle.vertices_textures[maxIndexY].y, closest.intersectUV[1]);
    }

    //colour = getPixelColour(&textureFile, texX, texY);

  colour = getPixelColour(&textureFile, closest.intersectUV[0] * textureFile.width, closest.intersectUV[1] * textureFile.height);
    //cout << "Tex X, Tex Y: " << texX << ", " << texY << "\n";

    return colour;

  }

  // else we use Phong shading to get the colour
  else {
    Kd *= (intensityDropOff(point) * angleOfIncidence(closest)); // multiply Kd by the diffuse intensity.
    // specular light 
    // closest.normal - this is the interpolated normal for Phong shading (it is previously calculated and stored in the RayTriangleIntersection object) 
    Ks *= calculateSpecularLight(point, rayDirection, closest.normal); // multiply Ks by the specular intensity.
    colour = getFinalColour(colour, Ka, Kd, Ks);
  }
 
  /*
  // CODE FOR HARD SHADOWS
  // are we in shadow? only colour the pixel if we are not 
  bool inShadow = InShadow(closest.intersectionPoint); 
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
  if (InShadow(closest.intersectionPoint)){
    const float shadowFraction = softShadows(closest);
    // mix shadow and normal colour
    Colour shadowColour = getFinalColour(colour, Ka/2, 0, 0); //getFinalColour(Colour, Ka, Kd, Ks)

    const int r = (shadowFraction * shadowColour.red) + ((1 - shadowFraction) * colour.red);
    const int g = (shadowFraction * shadowColour.green) + ((1 - shadowFraction) * colour.green);
    const int b = (shadowFraction * shadowColour.blue) + ((1 - shadowFraction) * colour.blue);
    return Colour(r,g,b);
  }
  return colour;
} 
 
Colour getFinalColour(Colour colour, float Ka, float Kd, float Ks){ 
  // this takes the ambient, diffuse and specular constants and gets the output colour 
  // note that we do not multiply the specular light by the colour (it is white hence the 255) 
  colour.SetRed(((Ka + Kd) * colour.red) + (Ks * 255));
  colour.SetGreen(((Ka + Kd) * colour.green) + (Ks * 255)); 
  colour.SetBlue(((Ka + Kd) * colour.blue) + (Ks * 255));
  return colour; 
} 
 
// given a point in the scene, this function calculates the intensity of the light 
float intensityDropOff(const vec3 point){ 
  const float distance = distanceVec3(point, lightPosition); 
  return lightIntensity / (2 * 3.1416 * distance * distance); //return intensity 
} 

// this function takes an intersection point and calculates the angle of incidence and 
// outputs an intensity value between 0 and 1 
float angleOfIncidence(RayTriangleIntersection intersection){ 
  ModelTriangle triangle = intersection.intersectedTriangle; 
  vec3 normal = triangle.getNormal(); 
  vec3 vectorToLight = lightPosition - intersection.intersectionPoint; 
  vectorToLight = normalize(vectorToLight); 
  float intensity = dot(normal, vectorToLight); 
  // the dot product returns 1 if they are parallel 
  // 0 if perpendicular 
  // <0 if the normal faces the other way 
  if (intensity < 0) return 0;
  return intensity; 
} 
 
float distanceVec3(const vec3 from, const vec3 to){ 
  const vec3 d = from - to; 
  const float a = d[0] * d[0]; 
  const float b = d[1] * d[1]; 
  const float c = d[2] * d[2]; 
  return sqrtf(a + b + c); 
} 
 
// this returns a true or false depending on if we are in shadow or not 
bool InShadow(vec3 point){ 
  vec3 shadowRayDirection = lightPosition - point; 
  float distance = distanceVec3(lightPosition, point); 
  shadowRayDirection = normalize(shadowRayDirection); 
  for (int o=0; o<objects.size(); o++) {
    // for each face, send a 'shadow ray' from the point to the light and check for intersections 
    for (int i = 0 ; i < objects[o].faces.size(); i++){ 
      ModelTriangle triangle = objects[o].faces[i]; 
        
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
      bool bool2 = (0 <= u) && (u <= 1) && (0 <= v) && (v <= 1) && ((u + v) <= 1); 
      bool bool3 = (t < distance); // an intersection beyond the light doesn't matter 
      // if we have an intersection then we can stop checking the other faces 
      // it is 0.000001 to avoid self intersection 
      if (bool1 && bool2 && bool3) return true; 
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
  vec3 reflection = normalize(incident - (2 * dot(incident, normal) * normal)); 
  vec3 viewerDirection = -rayDirection; 
  float intensity = dot(viewerDirection, reflection); 
  // CAN CHANGE THE SPECULAR DROP OFF HERE 
  intensity = pow(intensity, 20); 
  if (intensity < 0) return 0;
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
  vec3 normal = triangle.getNormal();
  vec3 point = intersection.intersectionPoint;
  vec3 up = gradientConstant * normal;
  //vec3 down = -gradientConstant * normal;
  vec3 pointAbove = point + up;
  vec3 pointBelow = point;//point + down;
  const bool above = InShadow(pointAbove);
  const bool below = InShadow(pointBelow);
  //float shadowFraction = 0;
  // we are in total light 
  if ((above == 0) && (below == 0)) return 0; //shadowFraction = 0
  // we are in total shadow 
  else if ((above == 1) && (below == 1)) return 1; //shadowFraction = 1
  // we are in-between, and need to calculate the gradient 
  else {
    vec3 upVector = pointAbove - pointBelow;
    for (int i = 1 ; i <= numberOfSteps ; i++){
      vec3 point = pointBelow + ((i / (float)numberOfSteps) * upVector);
      bool inShadow = InShadow(point);
      // the first point will definitely be in shadow and as we move up we find how in shadow it should be
      if (!inShadow) return (i / (float)numberOfSteps); //shadowFraction = (i / (float)numberOfSteps)
    }
  }
  return 0; //return shadowFraction
} 

void gouraudShading() { 
} 

Colour glass(vec3 rayDirection, RayTriangleIntersection closest, int depth){
  vec3 point = closest.intersectionPoint;
  ModelTriangle triangle = closest.intersectedTriangle;
  vec3 normal = triangle.getNormal();

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
  
  vec3 refracted (refraction[0], refraction[1], refraction[2]);
  if (refracted == vec3 (0,0,0)) return reflectionColour;
  

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
  output.SetRed ((reflectiveConstant * reflectionColour.red) + (refractiveConstant * refractionColour.red));
  output.SetGreen ((reflectiveConstant * reflectionColour.green) + (refractiveConstant * refractionColour.green));
  output.SetBlue ((reflectiveConstant * reflectionColour.blue) + (refractiveConstant * refractionColour.blue));
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
  const float cosTheta = abs(direction);

  // check: do we have total internal reflection (this depends on the incident ray and the critical angle)
  const float k = 1 - ratio * ratio * (1 - cosTheta * cosTheta);
  
  if (k < 0) return vec4 (0,0,0,0); // total internal reflection
  
  const vec3 refracted = normalize(ratio * incident + (ratio * cosTheta - sqrtf(k)) * normal);
  return vec4(refracted[0], refracted[1], refracted[2], sign(direction));
}

float fresnel(vec3 incident, vec3 normal, float ior) {
    float cosi = dot(incident, normal); 
    float etai = 1;
    float etat = ior;

    if (cosi > 0) std::swap(etai, etat); 
    
    // Compute sini using Snell's law
    const float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi)); 
    // Total internal reflection
    if (sint >= 1) return 1; //kr =1
    else { 
        float cost = sqrtf(std::max(0.f, 1 - sint * sint)); 
        cosi = fabsf(cosi); 
        const float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        const float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        return (Rs * Rs + Rp * Rp)*0.5f; //kr = (Rs * Rs + Rp * Rp)/2 
    } 
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
    return 0; //return kR
} 



////////////////////
// CULLING CODE
////////////////////


// we cull the faces in the scene that face away from the camera
// we do this by taking vectors from the camera to the centre of each face and dotting it with the normal
void backfaceCulling(vec3 rayDirection){
  // for each object
  for (int o = 0 ; o < objects.size() ; o++){
    // for each face
    for (int i = 0 ; i < objects[o].faces.size() ; i++){
      //ModelTriangle face = objects[o].faces[i];
      const vec3 normal = objects[o].faces[i].getNormal();
      //True if faces face the other way && !glass.
      objects[o].faces[i].culled = ((dot(normal, rayDirection) > 0) && (objects[o].faces[i].texture != "glass"));
    }
  }
}


// for a set of vertices (an object maybe), create a bounding box
vector<ModelTriangle> boundingBox(vector<ModelTriangle> inputFaces) {
  // we want to find the minimum and maximum values of x,y,z in all the vertices
  // first set the min to be (inf, inf, inf)
  // set max to be (-inf, -inf, -inf)
  vec3 minBound(numeric_limits<float>::infinity(), numeric_limits<float>::infinity(), numeric_limits<float>::infinity());
  vec3 maxBound = -minBound;
  print(minBound);
  print(maxBound);
  int n = inputFaces.size();
  // for each face
  for (int i = 0 ; i < n ; i++){
    ModelTriangle triangle = inputFaces[i];
    // get each component
  
    float tempMinX = min(triangle.vertices[0][0], triangle.vertices[1][0], triangle.vertices[1][0]);
    float tempMinY = min(triangle.vertices[0][1], triangle.vertices[1][1], triangle.vertices[1][1]);
    float tempMinZ = min(triangle.vertices[0][2], triangle.vertices[1][2], triangle.vertices[1][2]);

    float tempMaxX = max(triangle.vertices[0][0], triangle.vertices[1][0], triangle.vertices[1][0]);
    float tempMaxY = max(triangle.vertices[0][1], triangle.vertices[1][1], triangle.vertices[1][1]);
    float tempMaxZ = max(triangle.vertices[0][2], triangle.vertices[1][2], triangle.vertices[1][2]);

    // check for all the x,y,z values separately to see if it is a new min or maximum
    if (tempMinX < minBound[0]) minBound[0] = tempMinX;
    if (tempMinY < minBound[1]) minBound[1] = tempMinX;
    if (tempMinZ < minBound[2]) minBound[2] = tempMinX;
    
    if (tempMaxX > maxBound[0]) maxBound[0] = tempMaxX;
    if (tempMaxY > maxBound[1]) maxBound[1] = tempMaxX;
    if (tempMaxZ > maxBound[2]) maxBound[2] = tempMaxX;      
  }

  print(minBound);
  print(maxBound);

  // create the box object
  vector<ModelTriangle> boxFaces; // this is a cube so will have 12 faces
  vector<vec3> vertices; // these must be in a particular order so the faces can be constructed facing the right way and also the right faces are constructed
  vertices.push_back( vec3(minBound[0], minBound[1], minBound[2]) ); // bottom-left-forward   v1
  vertices.push_back( vec3(maxBound[0], minBound[1], minBound[2]) ); // bottom-right-forward  v2 
  vertices.push_back( vec3(minBound[0], maxBound[1], minBound[2]) ); // top-left-forward      v3
  vertices.push_back( vec3(maxBound[0], maxBound[1], minBound[2]) ); // top-right-forward     v4
  vertices.push_back( vec3(minBound[0], minBound[1], maxBound[2]) ); // bottom-left-back      v5
  vertices.push_back( vec3(maxBound[0], minBound[1], maxBound[2]) ); // bottom-right-back     v6
  vertices.push_back( vec3(minBound[0], maxBound[1], maxBound[2]) ); // top-left-back         v7
  vertices.push_back( vec3(maxBound[0], maxBound[1], maxBound[2]) ); // top-right-back        v8

  cout << "n: " << vertices.size() << endl;

  vector<vec3> vertexIndices;
  vertexIndices.push_back(vec3 (1,2,3)); // front face
  vertexIndices.push_back(vec3 (2,4,3)); // front face
  vertexIndices.push_back(vec3 (3,4,7)); //   top face
  vertexIndices.push_back(vec3 (4,8,7)); //   top face
  vertexIndices.push_back(vec3 (7,8,5)); //  back face
  vertexIndices.push_back(vec3 (8,6,5)); //  back face
  vertexIndices.push_back(vec3 (5,6,1)); //bottom face
  vertexIndices.push_back(vec3 (6,2,1)); //bottom face
  vertexIndices.push_back(vec3 (5,1,7)); //  left face
  vertexIndices.push_back(vec3 (1,3,7)); //  left face
  vertexIndices.push_back(vec3 (2,6,4)); // right face
  vertexIndices.push_back(vec3 (6,8,4)); // right face


  // make the vertices into faces
  for (int i = 0 ; i < 12 ; i++){
    const vec3 indices = vertexIndices[i];
    ModelTriangle triangle;
    triangle.colour = Colour (255,255,255); // arbitrarily set colour to white (for testing)
    for (int j = 0 ; j < 3 ; j ++){
      const int index = indices[j] - 1;
      triangle.vertices[j] = vertices[index];
      print(vertices[index]);
    }
    boxFaces.push_back(triangle);
  }
  return boxFaces;
}

////////////////////////////////
// ANIMATION CODE
////////////////////////////////

void spin(vec3 point, float theta, float distance){
  // we spin round by starting at the centre point looking at the camera, then spin around a set amount and work out the new camera position
  vec3 pointToCamera = cameraPosition - point;
  pointToCamera = normalize(pointToCamera);
  // rotate the vector by the angle
  vec3 col1 = vec3 (cos(theta), 0, -sin(theta)); 
  vec3 col2 = vec3 (0, 1, 0); 
  vec3 col3 = vec3 (sin(theta), 0, cos(theta));
  mat3 rotationMatrix (col1, col2, col3);
  vec3 vec = rotationMatrix * pointToCamera;
  vec = normalize(vec);
  cameraPosition = point + (distance * vec);
  lookAt(point);
  render();
  window.renderFrame();
}

void spinAround(float angle, int stepNumber, bool clockwise, int zoom){
  vec3 point = findCentreOfScene();
  float startDistance = distanceVec3(point, cameraPosition);
  float endDistance;
  
  if (zoom == 1) endDistance = startDistance / 1.5;
  else if (zoom == -1) endDistance = startDistance * 1.5;
  else  endDistance = startDistance;

  const float distanceStep = (endDistance - startDistance) / stepNumber;
  
  float angleStep = angle / stepNumber;
  if (clockwise) angleStep = -angleStep;
  

  // for each step we move the camera the right amount
  for (int i = 0 ; i < stepNumber ; i++){
      float distance = startDistance + (i * distanceStep);
      spin(point, angleStep, distance);
  }
}


void translateVertices(int objectIndex, vec3 direction, float distance){
  direction = normalize(direction);
  // for each face
  for (int i = 0 ; i < objects[objectIndex].faces.size() ; i++){
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = objects[objectIndex].faces[i].vertices[j];
      objects[objectIndex].faces[i].vertices[j] = vertex + (distance * direction);
    }
  }
}


// use equations of motion to animate a jump
void jump(int objectIndex, float height){
  // if we just define the height of the bounce, then we can calculate what the initial velocity must be and also how long it will take
  float a = -9.81; // acceleration

  // equations: 
  // v = u + at where u is initial velocity
  // maxHeight = u^2 / (-2a)
  // displacement = ut + (1/2)at^2
  // timeAtTopOfJump = u / -a;
  
  // initial velocity (m/s) using equations of motion
  float u = sqrt(height * (2*-a));
  // calculating how long the jump will take
  float totalTime = (u / -a) * 2;
  float timeStep = 0.02; // this will depend on how many frames we produce per second (normally about 24)

  // for each time frame, calculate the height of the object
  for (float t = 0 ; t < totalTime ; t += timeStep){
    // using the 2nd equations of motion (displacement one written above)
    float displacement = (u*t) + (0.5 * a * t * t);
    translateVertices(objectIndex, vec3 (0,1,0), displacement);
    render();
    window.renderFrame();
    // translate the vertices back to original
    translateVertices(objectIndex, vec3 (0,-1,0), displacement);
  }
}



void squash(int objectIndex, float squashFactor){
  // find the bottom of the object
  // also find the centre of the underside
  // (when we squash an object it squashes downwards)
  Object object = objects[objectIndex];
  float lowestPoint = numeric_limits<float>::infinity();
  vec3 averagedVertices (0,0,0);

  // for each face
  for (int i = 0 ; i < object.faces.size() ; i++){
    ModelTriangle face = object.faces[i];
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = face.vertices[j];
      averagedVertices = averagedVertices + vertex;
      if (vertex[1] < lowestPoint){
        lowestPoint = vertex[1];
      }
    }
  }
  averagedVertices /= float(object.faces.size() * 3);

  // we squash the object around the following point (the centre but on the under side of the object)
  vec3 squashCentre = averagedVertices;
  squashCentre[1] = lowestPoint;

  // for a squash we want to make the object flatter but also wider
  // make the y coordinates closer to the centre but the x and z coordinates further away from the centre
  // for each face
  for (int i = 0 ; i < object.faces.size() ; i++){
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = objects[objectIndex].faces[i].vertices[j];
      vec3 direction = vertex - squashCentre;
      //direction = normalize(direction);
      // increase the y and z, but decrease the x
      vec3 newPoint = vertex + (squashFactor * direction);
      newPoint[1] = vertex[1] - (squashFactor * direction[1]);
      objects[objectIndex].faces[i].vertices[j] = newPoint;
    }
  }
}


// same function as the jump except we include a squash and stretch transformation with the vertices
// before the jump we want a squash and also after it lands
// we want a stretch as it jumps in the air
void pixarJump(int objectIndex, float height, bool rotate, float maxSquashFactor){
  // save the object so we can go back to it
  Object objectCopy = objects[objectIndex];

  //jumpSquash(objectIndex);

  // these are equations to work out the height of the object at each time frame
  // if we just define the height of the bounce, then we can calculate what the initial velocity must be and also how long it will take
  float a = -50; // real life acceleratoin is a = -9.81, but this looked too slow in the render

  // equations: 
  // v = u + at where u is initial velocity
  // maxHeight = u^2 / (-2a)
  // displacement = ut + (1/2)at^2
  // timeAtTopOfJump = u / -a;
  
  // initial velocity (m/s) using equations of motion
  float u = sqrt(height * (2*-a));
  // calculating how long the jump will take
  float totalTime = (u / -a) * 2;
  float timeStep = 0.02; // this will depend on how many frames we produce per second (normally about 24)



  // for the jump we want the object to go from normal to stretch then down to normal again (squashFactor = 0)
  // we can stretch by using negative values in the squash function
  // quadratic motion again (quadratic speed with the stretching)
  // we will first get all the squash factors for each step
  // we start with a squashFactor of 0 and end with it too
  // again we will use a quadratic (not negative and the squash factor will decrease and then increase again)
  // y = at^2 + bt + c
  // at t = 0 we want y = 0, so we know c = 0
  // dy/dt = 2at + b
  // we want the squash factor to be at the lowest when the object is at the peak of its jump (so half way through)
  // t = totalTime / 2, dy/dt = 0
  // 0 = totalTime*a + b
  // b = -totalTime*a
  // So we have:
  // y = a(t^2) - (totalTime*a)t
  // when t = totalTime/2, we want to have y = -maxSquashFactor (this is our stretch)
  // -maxSquashFactor = (totalTime^2)(a/4) - (totalTime^2)(a/2)
  // maxSquashFactor = (totalTime^2)(a/4);
  // can now work out a and b
  float aQuad = (maxSquashFactor * 4) / (totalTime * totalTime);
  float bQuad = -totalTime * aQuad;

  // we can also make the object rotate as it jumps
  float fullTurn = 3.1415926;
  
  // number of steps
  float numberOfSteps = int(totalTime / timeStep);
  float stepAngle = fullTurn / numberOfSteps;
  
  // for each time frame, calculate the height of the object
  for (int i = 0 ; i < numberOfSteps ; i++){
    float t = i*timeStep;
    // using the 2nd equations of motion (displacement one written above)
    float displacement = (u*t) + (0.5 * a * t * t);
    // translate
    translateVertices(objectIndex, vec3 (0,1,0), displacement);
    // squash
    float squashFactor = (aQuad*t*t) + (bQuad*t);
    squash(objectIndex, squashFactor);
    // rotate
    if (rotate) {
      vec3 centre = findObjectCentre(objects[objectIndex]);
      rotateObject(objectIndex, stepAngle*i, centre);
    }
    // render
    render();
    window.renderFrame();
    // translate the vertices back to original and undo the squash
    objects[objectIndex] = objectCopy;
  }

  // after the jump we want the object to go from normal to squashed to normal again
  // can do the same as we did before the jump
  jumpSquash(objectIndex, maxSquashFactor);
}


void jumpSquash(int objectIndex, float maxSquashFactor){
  // save the object before the transformations so we can go back to it
  Object objectCopy = objects[objectIndex];

  // before the jump, squash the object down
  // use a quadratic step in the squash factor so it squashes quickly at first and then slows down as it gets to the maximum squash
  // it should also go back to normal after it has been squashed (it should speed up as it is preparing to jump)
  // if we use a quadratic function to get the squash factors then we should get the desired acceleration

  // how many steps do we want (how quickly should it squash)
  int steps = 10;
  vector<float> squashSteps;
  for (float t = 0 ; t < steps ; t++){
    // use y = -at^2 + bt + c
    // at t = 0, we want squashFactor = 0 and at t = steps we want squashFactor = 0
    // half way through (so t = steps/2) we want squashFactor = maxSquashFactor
    // we also want it to peak at t = steps/2, so we need to look at derivative
    // dy/dt = -2ax + b
    // we want dy/dt = 0 at time = steps/2
    // 0 = -(a*steps) + b
    // b = a*steps
    // the first equation (t = 0, y = 0) gives c = 0
    // so we now have:
    //   y = -a(t^2) + (steps*a)t;
    // we also want (t = steps/2, y = maxSquashFactor)
    //   maxSquashFactor = -(steps^2)(a/4) + (2*steps^2)(a/2)
    //   maxSquashFactor = steps^2 * (a/4);
    // can now get a and b
    float a = (4 * maxSquashFactor) / (steps * steps);
    float b = a * steps;
    float squashFactor = -(a*t*t) + (b*t);
    squashSteps.push_back(squashFactor);
  }

  //vector<float> squashSteps {0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4};
  for (int i = 0 ; i < squashSteps.size() ; i++){
    squash(objectIndex, squashSteps[i]);
    render();
    window.renderFrame();
    // put the object back to it's original
    objects[objectIndex] = objectCopy;
  }
}


void bounce(int objectIndex, float height, int numberOfBounces){
  // use a quadratic to get the heights of the bounces
  // y = an^2 + bn + c
  // n is the bounce number
  // at n = 0, y = height so therefore c = height
  // we want the minima to be on the last bounce
  // dy/dn = 2an + b
  // dy/dn = 0 when n = numberOfBounces
  // 0 = (2*numberOfBounces)a + b
  // b = -(2*numberOfBounces)a
  // so therefore:
  // y = an^2 + (-2*numberOfBounces*a)n + height
  // at n = numberOfBounces, y = 0
  // 0 = (numberOfBounces^2)a + (-2*numberOfBounces^2)a + height;
  // 0 = -(numberOfBounces^2)a + height;
  // we can now get a and b

  float a = height / (numberOfBounces * numberOfBounces);
  float b = -(2 * numberOfBounces * a);
  float c = height;

  // squash before the jump
  jumpSquash(objectIndex, 0.5);
  // for each bounce, jump but decrease the height
  for (int n = 0 ; n < numberOfBounces ; n++){
    float bounceHeight = (a*n*n) + (b*n) + c;
    float squashFactor = 0.5 * (bounceHeight / height);
    pixarJump(objectIndex, bounceHeight, false, squashFactor);
  }
}


vec3 findObjectCentre(Object object){
  vec3 averagedVertex (0,0,0);
  int n = object.faces.size();
  // for each face
  for (int i = 0 ; i < n ; i++){
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = object.faces[i].vertices[j];
      averagedVertex = averagedVertex + vertex;
    }
  }
  averagedVertex = averagedVertex / float(n*3);
  return averagedVertex;
}


// this function rotates an object in the secen by the specified angle
// it can rotate around a certain point (this is normally set at the objects centre)
void rotateObject(int objectIndex, float theta, vec3 point) {
  // create the rotation matrix
  vec3 col1 = vec3 (cos(theta), 0, -sin(theta)); 
  vec3 col2 = vec3 (0, 1, 0); 
  vec3 col3 = vec3 (sin(theta), 0, cos(theta));
  mat3 rotationMatrix (col1, col2, col3);

  Object object = objects[objectIndex];
  // for each face
  for (int i = 0 ; i < object.faces.size() ; i++){
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      vec3 vertex = object.faces[i].vertices[j];
      vec3 centreToVertex = vertex - point;
      // rotate this by the angle
      vec3 newVertex = point + rotationMatrix * centreToVertex;
      objects[objectIndex].faces[i].vertices[j] = newVertex;
    }
  }
}