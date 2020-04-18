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

#ifndef GLM_H
#define GLM_H
#include <glm/glm.hpp> 
#endif

// given a start and an end value, step from the start to the end with the right number of steps 
std::vector<glm::vec3> interpolate(glm::vec3 from, glm::vec3 to, float numberOfValues) { 
  std::vector<glm::vec3> out; //The output vector of numbers 
  out.push_back(from); //Add the first number in 
  glm::vec3 stepValue = (to - from) / (numberOfValues - 1); //numberOfValues - 1 as the first number is already counted 
  glm::vec3 previous = from; 
 
  //For each step 
  for (int i = 1; i < numberOfValues ; i++) { 
    glm::vec3 input = previous + stepValue; 
    out.push_back(input); 
    previous = input; 
  } 
 
  return out; 
} 

std::vector<CanvasPoint> interpolateWithDepth(CanvasPoint from, CanvasPoint to, float numberOfValues) {
  std::vector<CanvasPoint> vecInterpVectors;

  //Add the first number in.
  //vecInterpVectors.push_back(from);
  glm::vec3 castedFrom = glm::vec3(from.x, from.y, from.depth);
  glm::vec3 castedTo = glm::vec3(to.x, to.y, to.depth);
  glm::vec3 stepValue = (castedTo - castedFrom) / (numberOfValues); //numberOfValues - 1 as the first number is already counted
  glm::vec3 previous = castedFrom;

  for (int i = 0; i <= numberOfValues; i++) {
    float proportion = (i) / (numberOfValues);
    
    float inverseDepth = ((1 - proportion) * (1 / from.depth)) + (proportion * (1 / to.depth));
    float depthPoint1 = 1 / inverseDepth;
    glm::vec3 input = previous + stepValue;
    vecInterpVectors.push_back(CanvasPoint(input.x, input.y, depthPoint1));
    previous = input;
  }
  return vecInterpVectors;
}

#endif
