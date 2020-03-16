#ifndef MODELTRIANGLE_H
  #define MODELTRIANGLE_H
  #include <ModelTriangle.h>
#endif

#ifndef CANVASTRIANGLE_H
  #define CANVASTRIANGLE_H
  #include <CanvasTriangle.h>
#endif
#ifndef DRAWINGWINDOW_H
  #define DRAWINGWINDOW_H
  #include <DrawingWindow.h>
#endif

#ifndef UTILS_H
  #define UTILS_H
  #include <Utils.h>
#endif

#ifndef GLM_H
  #define GLM_H
  #include <glm/glm.hpp>
#endif

#ifndef FSTREAM_H
  #define FSTREAM_H
  #include <fstream>
#endif

#ifndef SSTREAM_H
  #define SSTREAM_H
  #include <sstream>
#endif
#ifndef VECTOR_H
  #define VECTOR_H
  #include <vector>
#endif

using namespace std;
using namespace glm;

// given a start and an end value, step from the start to the end with the right number of steps
vector<vec3> interpolate(vec3 from, vec3 to, float numberOfValues) {
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
uint32_t getColour(Colour colour) {
  uint32_t col = (255<<24) + (colour.red<<16) + (colour.green<<8) + colour.blue;
  return col;
}

void printVec3(vec3 name){
  cout << "[" << name[0] << ", " << name[1] << ", " << name[2] << "]\n";
}