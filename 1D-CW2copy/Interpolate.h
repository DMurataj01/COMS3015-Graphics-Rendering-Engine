#ifndef INTERPOLATE_H
#define INTERPOLATE_H

#ifndef VECTOR_H
#define VECTOR_H
#include <vector>
#endif

#ifndef CANVASPOINT_H
#define CANVASPOINT_H
#include "CanvasPoint.h"
#endif

#ifndef CANVASTRIANGLE_H
#define CANVASTRIANGLE_H
#include "CanvasTriangle.h"
#endif


#ifndef GLM_H
#define GLM_H
#include <glm/glm.hpp> 
#endif


float min(float a, float b, float c) {
  float min0 = glm::min(a, b);
  if (c < min0) return c;
  return min0;
}
int min_index(float a, float b, float c) {
  float min0 = glm::min(a, b);
  int i_min0 = (a == min0) ? 0 : 1;
  if (c < min0) return 2;
  return i_min0;
}
float max(float a, float b, float c) {
  float max0 = glm::max(a, b);
  if (c > max0) return c;
  return max0;
}
int max_index(float a, float b, float c) {
  float max0 = glm::max(a, b);
  int i_max0 = (a == max0) ? 0 : 1;
  if (c > max0) return 2;
  return i_max0;
}

// Returns the fractional part of a number.
float fpart(float x) {
	return x - floor(x);
}

float getValueBetweenNumbers(bool print, float a, float b, float percentage) {
  if (print) {
    std::cout << "In: " << a << "& " << b << ".. $" << percentage << "\n";
    std::cout << "Out: " << (a + (percentage * (b-a))) << "\n";
  }
  return (a + (percentage * (b-a)));
}

void print(glm::vec3 name){
  std::cout << "Vec3: [" << name[0] << ", " << name[1] << ", " << name[2] << "]\n";
}
void print(CanvasPoint vertex) {
  std::cout << "CanvasPoint: [" << vertex.x << ", " << vertex.y << ", " << vertex.depth << "]\n";
}
void print(TexturePoint vertex) {
  std::cout << "TexturePoint: [" << vertex.x << ", " << vertex.y << "]\n";
}
void print(CanvasTriangle triangle) {
  std::cout << "Triangle: [" << triangle.vertices[0].x << ", " << triangle.vertices[0].y << ", " << triangle.vertices[0].depth << "] ";
  std::cout << "[" << triangle.vertices[1].x << ", " << triangle.vertices[1].y << ", " << triangle.vertices[1].depth << "] ";
  std::cout << "[" << triangle.vertices[2].x << ", " << triangle.vertices[2].y << ", " << triangle.vertices[2].depth << "]\n";
}

// given a start and an end value, step from the start to the end with the right number of steps 
std::vector<glm::vec3> interpolate(glm::vec3 from, glm::vec3 to, float numberOfValues) { 
  std::vector<glm::vec3> vectorOut; //The output vector of numbers 
  vectorOut.push_back(from); //Add the first number in 
  glm::vec3 stepValue = (to - from) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted 
  glm::vec3 previous = from; 
 
  //For each step 
  for (int i = 1; i < numberOfValues ; i++) { 
    glm::vec3 input = previous + stepValue; 
    vectorOut.push_back(input); 
    previous = input; 
  } 
 
  return vectorOut; 
} 


std::vector<TexturePoint> interpolate(TexturePoint from, TexturePoint to, float numberOfValues) {
  std::vector<TexturePoint> vectorOut;

  //Add the first number in.
  vectorOut.push_back(from);
  glm::vec2 castedFrom(from.x, from.y);
  glm::vec2 castedTo(to.x, to.y);
  glm::vec2 stepValue = (castedTo - castedFrom) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted
  glm::vec2 previous = castedFrom;

  for (int i = 1; i < numberOfValues; i++) {
    glm::vec2 input = previous + stepValue;
    vectorOut.push_back(TexturePoint(input.x, input.y));
    previous = input;
  }
  return vectorOut;
}

CanvasPoint getClosestPoint(std::vector<CanvasPoint> pointList) {
  double minDepth = std::numeric_limits<float>::infinity();
  for (int i=0; i<pointList.size(); i++){
    if (pointList[i].depth < minDepth) minDepth = pointList[i].depth;
  }
  for (int i=0; i<pointList.size(); i++){
    if (pointList[i].depth == minDepth) return pointList[i];
  }
  return pointList[0];
}

CanvasPoint getFurthestPoint(std::vector<CanvasPoint> pointList) {
  double maxDepth = 0;
  for (int i=0; i<pointList.size(); i++){
    if (pointList[i].depth > maxDepth) maxDepth = pointList[i].depth;
  }
  for (int i=0; i<pointList.size(); i++){
    if (pointList[i].depth == maxDepth) return pointList[i];
  }
  return pointList[0];
}

#endif
