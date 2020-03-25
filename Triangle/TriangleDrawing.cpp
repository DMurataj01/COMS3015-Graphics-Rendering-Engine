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
vector<float> interpolate(float from, float to, int numberOfValues) {
    vector<float> vec_list;
    float increment = (to - from) / (numberOfValues-1);
    
    for (int i = 0; i<numberOfValues; i++)
        vec_list.push_back(from + (i*increment));
    
    return vec_list;
}

vector<vec3> interpolate(vec3 from, vec3 to, float numberOfValues) {
  vector<vec3> out; //The output vector of numbers
  out.push_back(from); //Add the first number in
  vec3 stepValue = (to - from) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted
  vec3 previous = from;

  //For each step
  for (int i = 1; i < numberOfValues ; i++) {
    vec3 input = previous + stepValue;
    out.push_back(input);
    previous = input;
  }

  return out;
}

vector<CanvasPoint> interpolate(CanvasPoint from, CanvasPoint to, float numberOfValues) {
  vector<CanvasPoint> vecInterpVectors;

  //Add the first number in.
  vecInterpVectors.push_back(from);
  vec3 castedFrom = vec3(from.x, from.y, from.depth);
  vec3 castedTo = vec3(to.x, to.y, to.depth);
  vec3 stepValue = (castedTo - castedFrom) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted
  vec3 previous = castedFrom;

  for (int i = 1; i < numberOfValues ; i++) {
    vec3 input = previous + stepValue;
    vecInterpVectors.push_back(CanvasPoint(input.x, input.y, input.z));
    previous = input;
  }
  return vecInterpVectors;
}

vector<TexturePoint> interpolate(TexturePoint from, TexturePoint to, float numberOfValues) {
  vector<TexturePoint> vecInterpVectors;

  //Add the first number in.
  vecInterpVectors.push_back(from);
  vec2 castedFrom = vec2(from.x, from.y);
  vec2 castedTo = vec2(to.x, to.y);
  vec2 stepValue = (castedTo - castedFrom) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted
  vec2 previous = castedFrom;

  for (int i = 1; i < numberOfValues ; i++) {
    vec2 input = previous + stepValue;
    vecInterpVectors.push_back(TexturePoint(input.x, input.y));
    previous = input;
  }
  return vecInterpVectors;
}



// given a colour in (R,G,B) format, puts it into binary form in a single string (ASCII)
uint32_t getColour(Colour colour) {
  uint32_t col = (255<<24) + (colour.red<<16) + (colour.green<<8) + colour.blue;
  return col;
}
uint32_t getColour(Colour colour, float percentage){
  return (255<<24) + (int(colour.red*percentage)<<16) + (int(colour.green*percentage)<<8) + (int(colour.blue*percentage));
}

void print(vec3 name){
  cout << "Vec3: [" << name[0] << ", " << name[1] << ", " << name[2] << "]\n";
}
void print(CanvasPoint vertex) {
  cout << "CanvasPoint: [" << vertex.x << ", " << vertex.y << ", " << vertex.depth << "]\n";
}

void print(TexturePoint vertex) {
  cout << "TexturePoint: [" << vertex.x << ", " << vertex.y << "]\n";
}
void print(CanvasTriangle triangle) {
  cout << "Triangle: [" << triangle.vertices[0].x << ", " << triangle.vertices[0].y << ", " << triangle.vertices[0].depth << "] ";
  cout << "[" << triangle.vertices[1].x << ", " << triangle.vertices[1].y << ", " << triangle.vertices[1].depth << "] ";
  cout << "[" << triangle.vertices[2].x << ", " << triangle.vertices[2].y << ", " << triangle.vertices[2].depth << "]\n";
}

////////////////////////////////////////////////////////////////////////////////
float fpart(float x) {
	return x - floor(x);
}
////////////////////////////////////////////////////////////////////////////////
float rfpart(float x) {
	return 1 - fpart(x);
}
////////////////////////////////////////////////////////////////////////////////
