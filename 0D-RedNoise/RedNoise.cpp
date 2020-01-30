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

void draw();
void update();
void handleEvent(SDL_Event event);
vector<float> interpolate(float from, float to, int numberOfValues);
vector<vec3> interpolate(vec3 from, vec3 to, int numberOfValues);

void draw_interp_grayscale();
void draw_interp_colour();

DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[]){
  SDL_Event event;
  while(true)
  {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();
    draw();
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

void draw(){
  window.clearPixels();
  draw_interp_colour();
}

void draw_interp_colour(){
  vector<float> vec_height_list = interpolate(0, 255, HEIGHT);

  for(int y=0; y<window.height; y++) {
    vector<float> vec_width_list = interpolate(255, 0, WIDTH);

    for(int x=0; x<window.width ;x++) {
      float red = vec_width_list[x];
      float green = vec_height_list[y];
      float blue = 255 - vec_width_list[x];
      uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
      window.setPixelColour(x, y, colour);
    }
  }
}


void draw_interp_grayscale(){
  for(int y=0; y<window.height ;y++) {
    vector<float> vec_list = interpolate(255, 0, WIDTH);
    for(int x=0; x<window.width ;x++) {
      float red = vec_list[x];
      float green = vec_list[x];
      float blue = vec_list[x];
      uint32_t colour = (255<<24) + (int(red)<<16) + (int(green)<<8) + int(blue);
      window.setPixelColour(x, y, colour);
    }
  }
}

vector<float> interpolate(float from, float to, int numberOfValues)
{
    vector<float> vec_list;
    
    float increment = (to - from) / (numberOfValues-1);
    
    for (int i = 0; i<numberOfValues; i++){
        vec_list.push_back(from + (i*increment));
    }
    
    return vec_list;
}

vector<vec3> interpolate(vec3 from, vec3 to, int numberOfValues)
{
    vector<vec3> vec_list;
    
    vec3 increment = (to - from) / vec3(numberOfValues-1, numberOfValues-1, numberOfValues-1);
    
    for (int i = 0; i<numberOfValues; i++){
        vec_list.push_back(from + (float(i)*increment));
    }
    
    return vec_list;
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
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
