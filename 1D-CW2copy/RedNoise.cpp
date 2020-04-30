// Note that in this code we use a left-handed coordinate system.

// Call Headers [ Safe from double includes ]
#include "OBJ.h"
#include "PPM.h"
#include "Materials.h"
#include "Interpolate.h"

#include <Utils.h> 
#include <RayTriangleIntersection.h> 
 
using namespace std; 
using namespace glm;

enum MOVEMENT {UP, DOWN, LEFT, RIGHT, ROLL_LEFT, ROLL_RIGHT, PAN_LEFT, PAN_RIGHT, TILT_UP, TILT_DOWN};
enum RENDERTYPE {WIREFRAME, RASTERIZE, RAYTRACE};
enum SHADOW {NO=0, YES=1, REFLECTIVE=2};

// Press '1' for Wireframe 
// Press '2' for Rasterized 
// Press '3' for Raytraced  

// Press '8' for Wireframe Animation.
// Press '9' for Rasterize Animation.
// Press '9' for Raytraced Animation.

//––---------------------------------//
/* Things You Can Change Are Here */
//––---------------------------------//

RENDERTYPE currentRender = WIREFRAME; //Set default RenderType here. 
std::string defaultPPMFileName = "render/snapshot";

const int maximumNumberOfReflections = 7;

#define W 176 //Set desired screen width here. 
#define H 200 //Set desired screen height here.

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
  
void handleEvent(SDL_Event event);
void render(); 
void clear(); 
void drawLine(CanvasPoint start, CanvasPoint end, Colour colour); 
void drawStrokedTriangle(CanvasTriangle triangle); 
void drawFilledTriangle(CanvasTriangle triangle); 
void rasterize(); 
void updateView (MOVEMENT movement);
/* FUNCTION Declarations */ 
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
SHADOW InShadow(vec3 point); 
float calculateSpecularLight(vec3 point, vec3 rayDirection, vec3 normal);
float softShadows(RayTriangleIntersection intersection);
Colour mirror(RayTriangleIntersection intersection, vec3 incident);
Colour glass(vec3 rayDirection, RayTriangleIntersection closest, int depth);
vec4 refract(vec3 I, vec3 N, float ior);
float fresnel(vec3 incident, vec3 normal, float ior);
void backfaceCulling(vec3 rayDirection);
void spin(vec3 point, float angle, float distance);
void spinAround(float angle, int stepNumber, bool clockwise, int zoom);
void spinAroundAndSpinSubObject(float angle, int stepNumber, bool clockwise, int zoom, int subObjectIndex, float subObjectRotation);
void jump(int objectIndex, float height);
void squash(int objectIndex, float squashFactor);
void pixarJump(int objectIndex, float height, bool rotate, float maxSquashFactor);
void pixarJump(vector<int> objectIndices, float height, float rotateAngle, float maxSquashFactor, vec3 rotateCentre, bool firstJump, float scaleDownFactor);
void cubeJumps(bool firstJump);
void jumpSquash(int objectIndex, float maxSquashFactor);
void bounce(int objectIndex, float height, int numberOfBounces);

DrawingWindow window;
 
// this stores the faces split up into separate objects
vector<Object> objects;
 
ImageFile textureFile;

int currentFrame = 0;

bool recording = false;
 
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

const float pi = 3.14159265358979323846;

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

glm::vec3 GetSceneXCentre() { 
  glm::vec3 sceneCentre; 
  const int n = objects.size(); 
   
  for (int o=0; o<n; o++) { 
    sceneCentre += objects.at(o).GetCentre(); 
  } 
   
  return sceneCentre / glm::vec3(n, n, n); 
} 
 

void resetToOriginalScene() {
  textureFile = importPPM(texFileName);
  objects = readGroupedOBJ(objFileName, mtlFileName, 1);
  objects.at(4).ApplyMaterial(MIRROR); // Mirrored floor
  objects.at(6).ApplyMaterial(GLASS);  // Mirrored Red Box.
  cameraPosition[0] = GetSceneXCentre()[0]; 
}
int main(int argc, char* argv[]) { 
  // 1) Initialise.
  initialise();

  resetToOriginalScene();
  render();

  SDL_Event event;
  while(true) { 
    // We MUST poll for events - otherwise the window will freeze ! 
    if(window.pollForInputEvents(&event)) handleEvent(event);
 
    // Need to render the frame at the end, or nothing actually gets shown on the screen ! 
    window.renderFrame(); 
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
  window.renderFrame(); 
  double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  if (displayRenderTime) cout << "Time Taken To Render: " << duration << "\n";
  if (recording) {
    exportToPPM(defaultPPMFileName + std::to_string(currentFrame) + ".ppm", CreateImageFileFromWindow(window, W, H)); 
    currentFrame++;
  }
} 



void jumpSquash(vector<int> objectIndices, float maxSquashFactor){
  // save the object before the transformations so we can go back to it
  vector<Object> objectCopies;
  for (int o = 0 ; o < objectIndices.size() ; o++){
    objectCopies.push_back(objects[objectIndices[o]]);
  }

  // before the jump, squash the object down
  // use a quadratic step in the squash factor so it squashes quickly at first and then slows down as it gets to the maximum squash
  // it should also go back to normal after it has been squashed (it should speed up as it is preparing to jump)
  // if we use a quadratic function to get the squash factors then we should get the desired acceleration

  // how many steps do we want (how quickly should it squash)
  int steps = 10;
  vector<float> squashSteps;
  for (float t = 0 ; t <= steps ; t++){
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
    // for each object
    for (int o = 0 ; o < objectIndices.size() ; o++){
      squash(objectIndices[o], squashSteps[i]);
    }
    render();
    window.renderFrame();
    // put the objects back to it's original
    for (int o = 0 ; o < objectIndices.size() ; o++){
      objects[objectIndices[o]] = objectCopies[o];
    }
  }
}



void bounce(vector<int> objectIndices, float height, int numberOfBounces, float rotateAngle, vec3 rotateCentre, float scaleDownFactor){
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
  jumpSquash(objectIndices, 0.5);
  // for each bounce, jump but decrease the height
  for (int n = 0 ; n < numberOfBounces ; n++){
    float bounceHeight = (a*n*n) + (b*n) + c;
    float squashFactor = 0.5 * (bounceHeight / height);
    float rotate = rotateAngle * (bounceHeight / height);
    float scaleDown = (bounceHeight / height);
    scaleDown = scaleDownFactor * scaleDown;
    pixarJump(objectIndices, bounceHeight, rotate, squashFactor, rotateCentre, false, scaleDown);
  }
}

void bounce(vector<int> objectIndices, float height, int numberOfBounces, float rotateAngle, vec3 rotateCentre, float scaleDownFactor, bool second){
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
  // for each bounce, jump but decrease the height
  for (int n = 0 ; n < numberOfBounces ; n++){
    float bounceHeight = (a*n*n) + (b*n) + c;
    float squashFactor = 0.5 * (bounceHeight / height);
    float rotate = rotateAngle * (bounceHeight / height);
    float scaleDown = (bounceHeight / height);
    scaleDown = scaleDownFactor * scaleDown;
    pixarJump(objectIndices, bounceHeight, rotate, squashFactor, rotateCentre, false, scaleDown);
  }
}


void ReRenderWait(int n) {
  for (int i = 0 ; i < n ; i++){
    render();
  }
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

    else if(event.key.keysym.sym == SDLK_8)     {
      currentRender = WIREFRAME;
      resetToOriginalScene();
      // 0) Read in Hackspace logo, scale and append to object list.
      vector<Object> hackspaceLogo = readGroupedOBJ("logo.obj", "logo.mtl", 0.06);
      hackspaceLogo.at(0).ApplyColour(Colour(0,0, 255), true);
      hackspaceLogo.at(0).Move(vec3(-1,0,0), 0.7);
      hackspaceLogo.at(0).Move(vec3(0,0,-1), 1.7);
      hackspaceLogo.at(0).Move(vec3(0,1, 0), 1.5);
      hackspaceLogo.at(0).RotateXZ(pi/8);
      objects.push_back(hackspaceLogo.at(0));
      
      /* Note - We're rendering at 60FPS! */ 
      /* 15 Frames == 0.25s */
      /* 30 Frames == 0.50s */
      /* 45 Frames == 0.75s */
      /* 60 Frames == 1.00s */

      // 1) Spin around.
      spinAround(pi, 100, true, -1);
      spinAround(pi, 100, true, 1);

      // 2) Bounce Hackspace logo.
      bounce(9, 1, 3);
      
      ReRenderWait(31);

      // 3) Funky spins and stuff.
      objects.at(9).RotateXZ(-pi/20);
    
      cameraPosition.x = objects.at(9).GetCentre().x;
    
      // Remove all the objects from the scene aside from the hackspace logo.
      objects.erase(objects.begin(), objects.begin() + 9);
      
      for (int i=0; i<60; i++) render();

      // Do R G B Christmas Tree.      
      for (int i=0; i<9; i++) {  
        if ((i % 3) == 0) {
          //Hold this 15 times. 
          for (int t=0; t<15; t++) {
            objects.at(0).ApplyColour(Colour(255,0, 0), true);
            objects.at(0).RotateZY(pi/15);
            render();
          } 
        }
        else if ((i % 3) == 1) {
          //Hold this 15 times.
          for (int t=0; t<15; t++) {
            objects.at(0).ApplyColour(Colour(0,255, 0), true);
            render();
          } 
        }
        else {
          //Hold this 15 times.
          for (int t=0; t<15; t++) {
            objects.at(0).ApplyColour(Colour(0, 0, 255), true);
            render();
          } 
        }
      }

      for (int t=0; t<15; t++) {
        objects.at(0).RotateZY(pi/15);
        render();
      }

      // Change Colour To Orange...
      objects.at(0).ApplyColour(Colour(255, 131, 0), true);
      
      //Hinge light to Bottom of the Hackspace Logo.
      lightPosition.y = objects.at(0).getLowestYValue();
      
      ReRenderWait(60);
      
      objects.at(0).Scale(vec3(5, 5, 5));
      cameraPosition.x = objects.at(0).GetCentre().x;
      objects.at(0).RotateXZ(-pi/14);
      objects.at(0).Move(vec3(0, 1, 0), 1);
      render();
    }
    else if(event.key.keysym.sym == SDLK_9)     {
      currentRender = RASTERIZE;
      resetToOriginalScene();
      vector<int> objectIndices;
      objectIndices.push_back(6);
      objectIndices.push_back(7);
      bounce(objectIndices, 1, 3, 0, vec3 (0,0,0), 0.3);
      ReRenderWait(20);
      cubeJumps(true); cubeJumps(false); cubeJumps(false); cubeJumps(false);
      ReRenderWait(20);
      bounce(objectIndices, 1, 3, 0, vec3 (0,0,0), 0.3, true);
      ReRenderWait(20);
      cubeJumps(false); cubeJumps(false); cubeJumps(false); cubeJumps(false);
    }
    else if(event.key.keysym.sym == SDLK_0)     {
      currentRender = RAYTRACE;
      resetToOriginalScene();
      vector<Object> hackspaceLogo = readGroupedOBJ("logo.obj", "logo.mtl", 0.06);
      hackspaceLogo.at(0).Move(vec3(0,0,-1), 0.7);
      hackspaceLogo.at(0).Move(vec3(-1,0,0), 2.5);
      hackspaceLogo.at(0).SnapToY0();
      hackspaceLogo.at(0).ApplyMaterial(TEXTURE);
      objects.push_back(hackspaceLogo.at(0));

      cameraPosition[0] = GetSceneXCentre()[0]; 

      // spin me right round.
      spinAroundAndSpinSubObject(pi, 100, true, -1, 9, pi/10);
      spinAroundAndSpinSubObject(pi, 100, true,  1, 9, pi/10);

      // delete old hsLogo.
      objects.erase(objects.begin() + 9);
      render();

      for (int i=0; i<15; i++) render();

      vector<Object> hsLogo = readGroupedOBJ("logo.obj", "logo.mtl", 0.06);
      hsLogo.at(0).ApplyMaterial(GLASS);
      hsLogo.at(0).Move(vec3(-1,0,0), 0.7);
      hsLogo.at(0).Move(vec3(0,0,-1), 1.7);
      hsLogo.at(0).Move(vec3(0,1, 0), 1.5);
      hsLogo.at(0).RotateXZ(pi/8);
      objects.push_back(hsLogo.at(0));
      render();

      // Hold me for around 0.3s.
      for (int i=0; i<25; i++) render();

      const int n = 40;
      for (int i=0; i<120; i++) {
        objects.at(9).RotateYX(pi/n);
        render();
      }

      for (int i=0; i<n; i++) {
        objects.at(9).RotateYX(pi/n);
        objects.at(9).Scale_Locked_YMin(vec3(1.025, 1.025, 1));
        objects.at(9).Move(vec3(0, 0, cameraPosition.z), 0.06);
        objects.at(9).Move(vec3(0, -1, 0), 0.03);
        objects.at(9).Move(vec3(-1,  0, 0), 0.027);
        render();
      }

      objects.erase(objects.begin(), objects.begin() + 9);
      objects.at(0).ApplyMaterial(NONE);
      objects.at(0).RotateXZ(-pi/20);
      cameraPosition.x = objects.at(0).GetCentre().x;
      lightIntensity = 0;
      lightPosition = cameraPosition;
      render();
      
      lightIntensity = 20; 
      
      //Beautiful orange.
      objects.at(0).ApplyColour(Colour(255, 131, 0), true);

      //Hinge light to Bottom of the Hackspace Logo.
      lightPosition.y = objects.at(0).getLowestYValue();
      
      // Slide for 60 frames --- Light Intensity Slider ( from 20 -> 100 )
      for (int i=0; i<60; i++) {
        lightIntensity += 1.25;
        render();
      }

      //Move from RGB(255, 170, 0) to (255, 130, 0) [ The Perfect Hackspace Orange ].
      lightIntensity = 120;
      render();
    }

    else if(event.key.keysym.sym == SDLK_m) {
      ImageFile imageFile = importPPM("texture.ppm");
      renderImageFile(imageFile);
    }
    else if(event.key.keysym.sym == SDLK_n) {
      ImageFile displaySnapShot = CreateImageFileFromWindow(window, WIDTH, HEIGHT);
      exportToPPM(defaultPPMFileName + std::to_string(currentFrame) + ".ppm", displaySnapShot); 
      currentFrame++;
    }

    else if(event.key.keysym.sym == SDLK_r) {
      recording = !recording;
      if (recording) cout << "Recording Started\n";
      else cout << "Recording Stopped\n";
    }
  } 
} 
 
 void cubeJumps(bool firstJump) {
  // first get the centre of the two objects
  // for the 1st object
  vec3 sum (0,0,0);
  Object object1 = objects.at(6);
  int n1 = object1.faces.size();
  for (int i = 0 ; i < n1 ; i++){
    for (int j = 0 ; j < 3 ; j++){
      sum += object1.faces[i].vertices[j];
    }
  }
  // for the 2nd object
  Object object2 = objects.at(7);
  int n2 = object2.faces.size();
  for (int i = 0 ; i < n2 ; i++){
    for (int j = 0 ; j < 3 ; j++){
      sum += object2.faces[i].vertices[j];
    }
  }
  vec3 centre = sum / float((n1*3) + (n2*3));
  vector<int> objectIndices;
  objectIndices.push_back(6);
  objectIndices.push_back(7);

  pixarJump(objectIndices, 1.5, 3.14159/2, 0.5, centre, firstJump, 0);

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

  render();
} 
 
void lookAt(vec3 point){
  vec3 direction = normalize(cameraPosition - point);
  cameraForward = direction; 
  // vec3(0,1,0) is random vector from slides.
  cameraRight = glm::cross(vec3(0,1,0), cameraForward); 
  cameraUp = glm::cross(cameraForward, cameraRight); 
  
  cameraForward = normalize(cameraForward); 
  cameraRight = normalize(cameraRight); 
  cameraUp = normalize(cameraUp); 
   
  cameraOrientation = mat3 (cameraRight, cameraUp, cameraForward); 
 
  render(); 
} 
 
// this function averages all the vertices in the scene to find the centre of the scene 
vec3 findCentreOfScene(){ 
  vec3 sum(0,0,0); 
  for (int o=0; o<objects.size(); o++){
    sum += objects.at(o).GetCentre(); 
  }
  sum /= (float)(objects.size()); 
  return sum; 
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
  if (numberOfSteps == 0) setDepthPixelColour(start.x, start.y, glm::min(start.depth, end.depth), getImageFilePixelColour(&textureFile, texturePoints.at(0).x, texturePoints.at(0).y).toUINT32_t()); 
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
      setDepthPixelColour(x, y, depth, getImageFilePixelColour(&textureFile, texturePoints.at(i).x, texturePoints.at(i).y).toUINT32_t());
    } 
  } 
} 

void drawWuLine(CanvasPoint ptStart, CanvasPoint ptEnd, Colour ptClr) {
  float diffX = (ptEnd.x - ptStart.x);
  float diffY = (ptEnd.y - ptStart.y);

  int x1 = ptStart.x;
  int y1 = ptStart.y;
  int x2 = ptEnd.x;
  int y2 = ptEnd.y;
  double d1 = ptStart.depth;
  double d2 = ptEnd.depth;


  if ((abs(diffX) == 0.f) || (abs(diffY) == 0.f) || (abs(diffX) == abs(diffY)) ) {
    drawLine(ptStart, ptEnd, ptClr);
  }
  else if (abs(diffX) > abs(diffY)) {
    // Mostly Horizontal line.
    
    if(x2 < x1) { swap(x1, x2); swap(y1, y2); swap(d1, d2);}

    // Add the first endpoint.
    setDepthPixelColour( x1, y1    , d1, ptClr.toUINT32_t(0.5));
    setDepthPixelColour( x1, y1 + 1, d1, ptClr.toUINT32_t(0.5));

    // Add the second endpoint
    setDepthPixelColour( x2, y2    , d2, ptClr.toUINT32_t(0.5));
    setDepthPixelColour( x2, y2 + 1, d2, ptClr.toUINT32_t(0.5));
    
    // get dy/dx gradient.
    const float gradient = diffY / diffX;
    
    const float numberOfSteps = int((x2 - 1) - (x1 + 1));
    //printf("Number of Steps: %d\n", numberOfSteps);
    if (numberOfSteps > 0) {
      
      float intery = y1 + gradient;
      
      // Add all the points between the endpoints)
      for(int x = x1 + 1; x <= x2 - 1; x++) {
        int i = x - (x1+1);
        float proportion = i / numberOfSteps;

        const double inverseDepth = ((1 - proportion) * (1 / d1)) + (proportion * (1 / d2)); // got this equation from notes 
        const double depth = 1 / inverseDepth; 

        setDepthPixelColour(x, floor(intery)    , depth, ptClr.toUINT32_t( 1- fpart(intery)));
        setDepthPixelColour(x, floor(intery) + 1, depth, ptClr.toUINT32_t( fpart(intery)));

        intery += gradient;
      }
    }
    

  }   
  else {
    if(y2 < y1) { swap(x1, x2); swap(y1, y2); swap(d1, d2);}

    const float gradient = diffX / diffY;

    // Add the first endpoint
    setDepthPixelColour( x1, y1    , d1, ptClr.toUINT32_t(0.5) );
    setDepthPixelColour( x1, y1 + 1, d1, ptClr.toUINT32_t(0.5) );
    
    // Add the second endpoint
    setDepthPixelColour( x2, y2    , d2, ptClr.toUINT32_t(0.5) );
    setDepthPixelColour( x2, y2 + 1, d2, ptClr.toUINT32_t(0.5) );

    const float numberOfSteps = int((y2 - 1) - (y1 + 1));

    if (numberOfSteps > 0) {

      float interx = x1 + gradient;
      
      // Add all the points between the endpoints
      for(int y = y1 + 1; y <= y2 - 1; y++){
        int i_interx = floor(interx);

        const int i = y - (y1+1);
        const float proportion = i / numberOfSteps; 
        const double inverseDepth = ((1 - proportion) * (1 / d1)) + (proportion * (1 / d2)); // got this equation from notes 
        const double depth = 1 / inverseDepth; 

        setDepthPixelColour( i_interx    , y, depth, ptClr.toUINT32_t( 1 - fpart(interx)));
        setDepthPixelColour( i_interx + 1, y, depth, ptClr.toUINT32_t( fpart(interx)));
        interx += gradient;
      }  
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

// draws an unfilled triangle with the input points as vertices 
void drawStrokedTriangle(CanvasTriangle triangle){ 
  CanvasPoint point1 = triangle.vertices[0]; 
  CanvasPoint point2 = triangle.vertices[1]; 
  CanvasPoint point3 = triangle.vertices[2]; 
  Colour colour = triangle.colour; 
 
  drawWuLine(point1,point2,colour); 
  drawWuLine(point2,point3,colour); 
  drawWuLine(point3,point1,colour); 
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
  
  //this code draws the outline of the triangle ontop of the filled triangle.
  drawLine(maxPoint, middlePoint, triangle.colour); 
  drawLine(middlePoint, minPoint, triangle.colour); 
  drawLine(maxPoint, minPoint, triangle.colour); 
  
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

  //Length of line from topPoint to cutterPoint.
  const float smallHypo = sqrtf(( pow(cutterPoint.x - topPoint.x, 2) + pow(cutterPoint.y - topPoint.y, 2) ));
  
  //Calculate proportion [ how far down the line cutterPoint is ]
  const float p = smallHypo/bigHypo;

  /* Get texture cut point using proportion & SOHCAHTOA */

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

  //** 4.1. Fill the Top Flat Bottom Triangle.
  //textureFlatBottomTriangle(imageFile, CanvasTriangle(topPoint, leftPoint, rightPoint), closestPoint, furthestPoint);
  
  float steps = glm::abs(middlePoint.y - topPoint.y); // how many rows 
  vector<TexturePoint> t_interpLeft = interpolate(topPoint.texturePoint, leftPoint.texturePoint, steps);
  vector<TexturePoint> t_interpRight = interpolate(topPoint.texturePoint, rightPoint.texturePoint, steps);

  // if the two vertices are on the same y line, then just draw a line between the two 
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
  
  //** 4.2. Fill the Bottom Flat Top Triangle.
  //textureFlatTopTriangle(imageFile, CanvasTriangle(leftPoint, rightPoint, lowestPoint), closestPoint, furthestPoint);

  // the lower triangle 
  // for each row, fill it in 
  float steps2 = glm::abs(lowestPoint.y - leftPoint.y);
  t_interpLeft = interpolate(leftPoint.texturePoint, lowestPoint.texturePoint, steps2);
  t_interpRight = interpolate(rightPoint.texturePoint, lowestPoint.texturePoint, steps2);

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

void rasterize(){  
  //for each object.
  for (int o = 0; o < objects.size(); o++){
    if (!objects.at(o).hidden) {
      // for each face 
      for (int i = 0 ; i < objects[o].faces.size() ; i++) { 
        ModelTriangle triangle = objects[o].faces[i]; 
        CanvasTriangle canvasTriangle; 
        canvasTriangle.colour = triangle.colour;
        canvasTriangle.textured = (triangle.material == TEXTURE);

        // for each vertex 
        for (int j = 0 ; j < 3 ; j++) { 
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
            if ((currentRender == RASTERIZE) && canvasTriangle.textured) canvasTriangle.vertices[j].texturePoint = TexturePoint(triangle.vertices_textures[j].x * textureFile.width, triangle.vertices_textures[j].y * textureFile.height);
          } 
        } 

        if (currentRender == WIREFRAME) drawStrokedTriangle(canvasTriangle); 
        else if (canvasTriangle.textured) drawTexturedTriangle(&textureFile, canvasTriangle);
        else drawFilledTriangle(canvasTriangle); 
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
  // the 4th value in this vector is the index of the face if there is an intersection
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
  // for each object
  for (int o=0; o<objectSolutions.size(); o++) {
      vector<vec4> solutions = objectSolutions[o];
      for (int i = 0 ; i < objects[o].faces.size() ; i++){ 
        
        vec4 possibleSolution = solutions[i]; 
        const float t = possibleSolution[0]; 
        const float u = possibleSolution[1]; 
        const float v = possibleSolution[2];
        const int index = possibleSolution[3];
        
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
            const vec3 p0 = triangle.vertices[0]; 
            const vec3 p1 = triangle.vertices[1]; 
            const vec3 p2 = triangle.vertices[2]; 
            const vec3 intersection = p0 + (u * (p1 - p0)) + (v * (p2 - p0)); 
            closest.intersectionPoint = intersection; 

            // calculating the distance between the camera and intersection point 
            const vec3 d = intersection - rayPoint; 
            closest.distanceFromCamera = sqrt( (d[0]*d[0]) + (d[1] * d[1]) + (d[2] * d[2]));

            // calculating the normal of the intersection 
            const vec3 n0 = triangle.normals[0]; 
            const vec3 n1 = triangle.normals[1]; 
            const vec3 n2 = triangle.normals[2]; 
            closest.normal = n0 + (u * (n1 - n0)) + (v * (n2 - n0));

            closest.intersectUV = vec2(u, v);
            closest.intersectedTriangle = triangle; 
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
  if (depth == maximumNumberOfReflections) return Colour(255,255,255);  

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
  if (triangle.material == MIRROR){ 
    vec3 incident = rayDirection; 
    vec3 normal = closest.intersectedTriangle.getNormal(); 
    vec3 reflection = normalize(incident - (2 * dot(incident, normal) * normal));
    // avoid self-intersection 
    return shootRay(point + ((float)0.00001 * normal), reflection, depth + 1, currentIOR);
  } 
  else if (triangle.material == GLASS){
    return glass(rayDirection, closest, depth);
  }
  else if (triangle.material == TEXTURE) {
    const vec2 e0 = closest.intersectedTriangle.vertices_textures[1] - closest.intersectedTriangle.vertices_textures[0];
    const vec2 e1 = closest.intersectedTriangle.vertices_textures[2] - closest.intersectedTriangle.vertices_textures[0];

    const vec2 texture_point = closest.intersectedTriangle.vertices_textures[0] + (closest.intersectUV[0] * e0) + (closest.intersectUV[1] * e1);

    // std::cout << closest_point[0] << " , " << closest_point[1] << std::endl;
    const float x = texture_point.x * textureFile.width;
    const float y = texture_point.y * textureFile.height;

    return getImageFilePixelColour(&textureFile, x, y);
  }

  else if (triangle.material == BUMP) {

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

  if (InShadow(closest.intersectionPoint) == YES || InShadow(closest.intersectionPoint) == REFLECTIVE){
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
SHADOW InShadow(vec3 point){ 
  const vec3 shadowRayDirection = normalize(lightPosition - point);
  const float distance = distanceVec3(lightPosition, point); 
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
      const float t = possibleSolution[0]; 
      const float u = possibleSolution[1]; 
      const float v = possibleSolution[2]; 
        
      // if it is actually a solution 
      bool bool1 = (t > 0.0001); // this is not 0 to avoid self-intersection 
      bool bool2 = (0 <= u) && (u <= 1) && (0 <= v) && (v <= 1) && ((u + v) <= 1); 
      bool bool3 = (t < distance); // an intersection beyond the light doesn't matter 
      // if we have an intersection then we can stop checking the other faces 
      // it is 0.000001 to avoid self intersection 
      if (bool1 && bool2 && bool3) {
        if (triangle.material == GLASS) return REFLECTIVE;
        else return YES; 
      }
    } 
  }
  return NO; 
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
 
float softShadows(RayTriangleIntersection intersection){
  ///////////////////
  // PARAMETERS
  ///////////////////
  float gradientConstant = 0.5; // how big the gradient should be
  int numberOfSteps = 50; // how smooth it should be
  ///////////////////

  vec3 normal = intersection.intersectedTriangle.getNormal();
  vec3 point = intersection.intersectionPoint;
  vec3 up = gradientConstant * normal;
  vec3 pointAbove = point + up;
  vec3 pointBelow = point;//point + down;

  // Total Light.
  if ((InShadow(pointAbove) == NO) && (InShadow(pointBelow) == NO)) return 0;
  // Total Shadow.
  else if ((InShadow(pointAbove) == YES) && (InShadow(pointBelow) == YES)) return 1;
  // Reflective Surface.
  else if ((InShadow(pointAbove) == REFLECTIVE) || (InShadow(pointBelow) == REFLECTIVE)) return 0.08;
  // Somewhere in-between total light and total shadow.
  else {
    vec3 upVector = pointAbove - pointBelow;
    for (int i = 1 ; i <= numberOfSteps ; i++){
      vec3 point = pointBelow + ((i / (float)numberOfSteps) * upVector);
      // the first point will definitely be in shadow and as we move up we find how in shadow it should be
      if (InShadow(point) == NO) return (i / (float)numberOfSteps); //shadowFraction = (i / (float)numberOfSteps)
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
        objects[o].faces[i].culled = ((dot(normal, rayDirection) > 0) && (objects[o].faces[i].material != GLASS));
      }
  }
}





////////////////////////////////
// ANIMATION CODE
////////////////////////////////

void spin(vec3 point, float theta, float distance){
  // we spin round by starting at the centre point looking at the camera, then spin around a set amount and work out the new camera position
  vec3 pointToCamera = normalize(cameraPosition - point);
  // rotate the vector by the angle
  vec3 col1(cos(theta), 0, -sin(theta)); 
  vec3 col2(0, 1, 0); 
  vec3 col3(sin(theta), 0, cos(theta));
  mat3 rotationMatrix (col1, col2, col3);
  vec3 vec = normalize(rotationMatrix * pointToCamera);
  cameraPosition = point + (distance * vec);
  lookAt(point);
}

void spinAround(float angle, int stepNumber, bool clockwise, int zoom){
  vec3 point = findCentreOfScene();
  float startDistance = distanceVec3(point, cameraPosition);
  float endDistance = startDistance;
  
  if (zoom == 1) endDistance = startDistance / 1.5;
  else if (zoom == -1) endDistance = startDistance * 1.5;

  const float distanceStep = (endDistance - startDistance) / stepNumber;
  
  const float angleStep = (!clockwise) ? (angle/stepNumber) : (-angle/stepNumber);

  cout << "Step Number: " << angle << ", .... step: " << stepNumber << "\n";
  

  // for each step we move the camera the right amount
  for (int i = 0; i < stepNumber; i++){
      float distance = startDistance + (i * distanceStep);
      spin(point, angleStep, distance);
      render();
  }
}

void spinAroundAndSpinSubObject(float angle, int stepNumber, bool clockwise, int zoom, int subObjectIndex, float subObjectRotation){
  vec3 point = findCentreOfScene();
  float startDistance = distanceVec3(point, cameraPosition);
  float endDistance = startDistance;
  
  if (zoom == 1) endDistance = startDistance / 1.5;
  else if (zoom == -1) endDistance = startDistance * 1.5;

  const float distanceStep = (endDistance - startDistance) / stepNumber;
  
  const float angleStep = (!clockwise) ? (angle/stepNumber) : (-angle/stepNumber);

  // for each step we move the camera the right amount
  for (int i = 0; i < stepNumber; i++){
      float distance = startDistance + (i * distanceStep);
      spin(point, angleStep, distance);
      objects.at(subObjectIndex).RotateXZ(subObjectRotation);
      render();
  }
}

// use equations of motion to animate a jump
void jump(int objectIndex, float height){
  // if we just define the height of the bounce, then we can calculate what the initial velocity must be and also how long it will take
  float a = -50; // acceleration

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
  for (float t = 0; t < totalTime; t += timeStep){
    // using the 2nd equations of motion (displacement one written above)
    float displacement = (u*t) + (0.5 * a * t * t);
    objects[objectIndex].Move(vec3(0,1,0), displacement);
    render();
    // translate the vertices back to original
    objects[objectIndex].Move(vec3(0,-1,0), displacement);
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
  float a = -50; // real life acceleration is a = -9.81, but this looked too slow in the render

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
  
  // number of steps
  float numberOfSteps = int(totalTime / timeStep);
  float stepAngle = pi / numberOfSteps;
  
  // for each time frame, calculate the height of the object
  for (int i = 0 ; i < numberOfSteps ; i++){
    float t = i*timeStep;
    // using the 2nd equations of motion (displacement one written above)
    float displacement = (u*t) + (0.5 * a * t * t);
    // translate
    objects[objectIndex].Move(vec3(0,1,0), displacement);
    // squash
    float squashFactor = (aQuad*t*t) + (bQuad*t);
    squash(objectIndex, squashFactor);
    // rotate
    if (rotate) objects[objectIndex].RotateXZ(stepAngle*i);
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

// same function as the jump except we include a squash and stretch transformation with the vertices
// before the jump we want a squash and also after it lands
// we want a stretch as it jumps in the air
void pixarJump(vector<int> objectIndices, float height, float rotateAngle, float maxSquashFactor, vec3 rotateCentre, bool firstJump, float scaleDownFactor){
  // for each object save the original so we can go back to it
  vector<Object> objectCopies;
  for (int o = 0 ; o < objectIndices.size() ; o++){
    objectCopies.push_back(objects[objectIndices[o]]);
  }

  if (firstJump){
    jumpSquash(objectIndices, 0.5); 
  }

  // these are equations to work out the height of the object at each time frame
  // if we just define the height of the bounce, then we can calculate what the initial velocity must be and also how long it will take
  float a = -50; // real life acceleration is a = -9.81, but this looked too slow in the render

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

  
  // number of steps
  float numberOfSteps = int(totalTime / timeStep);
  // we can also make the object rotate as it jumps
  float stepAngle = rotateAngle / numberOfSteps;
  float scaleDownStep = scaleDownFactor / numberOfSteps;
  
  // for each time frame, calculate the height of the object
  for (int i = 0 ; i < numberOfSteps ; i++){
    float t = i*timeStep;
    // using the 2nd equations of motion (displacement one written above)
    float displacement = (u*t) + (0.5 * a * t * t);
    
    // for each object, do the transformations
    for (int o = 0 ; o < objectIndices.size() ; o++){
      int objectIndex = objectIndices[o];
      // translate
      objects[objectIndex].Move(vec3(0,1,0), displacement);
      // squash
      float squashFactor = (aQuad*t*t) + (bQuad*t);
      squash(objectIndex, squashFactor);
      // rotate
      if (rotateAngle != 0) objects[objectIndex].RotateXZ(stepAngle*i, rotateCentre);
      // scale down
      if (scaleDownFactor != 0){
        float scaleFactor = (i * scaleDownStep);
        vec3 scaleCentre = objects.at(objectIndex).getBottomCentreOfObject(); 
        objects.at(objectIndex).ScaleObject(scaleCentre, scaleFactor);
      }
    }

    render();
    // translate the vertices back to original and undo the squash
    for (int o = 0 ; o < objectIndices.size() ; o++){
      int objectIndex = objectIndices[o];
      objects[objectIndex] = objectCopies[o];
    }
  }
  // on the last step we want to keep the rotation and the scale the same once resetting the object back to normal, so rotate by full angle again
  for (int o = 0 ; o < objectIndices.size() ; o++){
    int objectIndex = objectIndices[o];
    vec3 scaleCentre = objects.at(objectIndex).getBottomCentreOfObject(); 
    objects[objectIndex].RotateXZ(rotateAngle, rotateCentre);
    objects.at(objectIndex).ScaleObject(scaleCentre, scaleDownFactor);
  }

  // after the jump we want the object to go from normal to squashed to normal again
  // can do the same as we did before the jump
  jumpSquash(objectIndices, maxSquashFactor);
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
    //objects.at(9).RotateXZ(-pi/25);
  }
}