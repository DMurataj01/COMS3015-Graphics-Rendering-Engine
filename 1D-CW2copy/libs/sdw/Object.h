#ifndef MODELTRIANGLE_H
  #define MODELTRIANGLE_H
  #include <ModelTriangle.h>
#endif

#ifndef VECTOR_H
  #define VECTOR_H
  #include <vector>
#endif

#include "Materials.h"

class Object {
  public:
    std::vector<ModelTriangle> faces; // stores the faces of the object
    bool hasBoundingBox; // true if a bounding box has been created for this object
    std::vector<ModelTriangle> boxFaces; // if a bounding box has been created, this stores the faces of it
    MATERIAL material;

    Object() {
      hasBoundingBox = false;
    }

    Object(std::vector<ModelTriangle> inputFaces) {
      faces = inputFaces;
      hasBoundingBox = false;
    }

    void Clear() {
      faces.clear();
      hasBoundingBox = false;
      boxFaces.clear();
    }

    void ApplyMaterial(MATERIAL mat) {
      for(int i= 0; i< faces.size(); i++) {
        faces.at(i).material = mat;
      }
      material = mat;
    }

    void ApplyColour(Colour colour, bool resetMaterial) {
      for(int i= 0; i< faces.size(); i++) {
        faces.at(i).colour = colour;
        if (resetMaterial) faces.at(i).material = NONE;
        material = NONE;
      }
    }

    glm::vec3 GetCentre() {
      glm::vec3 sum(0,0,0);
      for (int i=0; i< faces.size(); i++) {
        sum += ((faces.at(i).vertices[0] + faces.at(i).vertices[1] + faces.at(i).vertices[2])/(float)3);
      }
      return sum/(float) faces.size();
    }

    // Rotate about the centre in the XZ direction.
    void RotateXZ(float theta) {
      glm::vec3 col1 = glm::vec3 (cos(theta), 0, sin(theta)); 
      glm::vec3 col2 = glm::vec3 (0, 1, 0); 
      glm::vec3 col3 = glm::vec3 (-sin(theta), 0, cos(theta));
      glm::mat3 rotationMatrix (col1, col2, col3);
      
      const glm::vec3 centre = GetCentre();

      for (int i=0; i<faces.size(); i++) {
        faces.at(i).vertices[0] = centre + rotationMatrix * (faces.at(i).vertices[0] - centre);
        faces.at(i).vertices[1] = centre + rotationMatrix * (faces.at(i).vertices[1] - centre);
        faces.at(i).vertices[2] = centre + rotationMatrix * (faces.at(i).vertices[2] - centre);
      }
    }
    // Rotate about the point in the XZ direction.
    void RotateXZ(float theta, glm::vec3 point) {
      glm::vec3 col1 = glm::vec3 (cos(theta), 0, -sin(theta)); 
      glm::vec3 col2 = glm::vec3 (0, 1, 0); 
      glm::vec3 col3 = glm::vec3 (sin(theta), 0, cos(theta));
      glm::mat3 rotationMatrix (col1, col2, col3);
      
      for (int i=0; i<faces.size(); i++) {
        faces.at(i).vertices[0] = point + rotationMatrix * (faces.at(i).vertices[0] - point);
        faces.at(i).vertices[1] = point + rotationMatrix * (faces.at(i).vertices[1] - point);
        faces.at(i).vertices[2] = point + rotationMatrix * (faces.at(i).vertices[2] - point);
      }
    }
    // Rotate about the centre in the ZY direction.
    void RotateZY(float theta) {
      glm::vec3 col1 = glm::vec3 (1, 0, 0); 
      glm::vec3 col2 = glm::vec3 (0,  cos(theta), -sin(theta)); 
      glm::vec3 col3 = glm::vec3 (0, sin(theta), cos(theta));
      glm::mat3 rotationMatrix (col1, col2, col3);
      
      const glm::vec3 centre = GetCentre();

      for (int i=0; i<faces.size(); i++) {
        faces.at(i).vertices[0] = centre + rotationMatrix * (faces.at(i).vertices[0] - centre);
        faces.at(i).vertices[1] = centre + rotationMatrix * (faces.at(i).vertices[1] - centre);
        faces.at(i).vertices[2] = centre + rotationMatrix * (faces.at(i).vertices[2] - centre);
      }
    }
    // Rotate about the centre in the YX direction.
    void RotateYX(float theta) {
      glm::vec3 col1 = glm::vec3 (cos(theta), -sin(theta), 0); 
      glm::vec3 col2 = glm::vec3 (sin(theta), cos(theta), 0); 
      glm::vec3 col3 = glm::vec3 (0, 0, 1);
      glm::mat3 rotationMatrix (col1, col2, col3);
      
      const glm::vec3 centre = GetCentre();

      for (int i=0; i<faces.size(); i++) {
        faces.at(i).vertices[0] = centre + rotationMatrix * (faces.at(i).vertices[0] - centre);
        faces.at(i).vertices[1] = centre + rotationMatrix * (faces.at(i).vertices[1] - centre);
        faces.at(i).vertices[2] = centre + rotationMatrix * (faces.at(i).vertices[2] - centre);
      }
    }
    // Move d distance in normalised direction.
    void Move(glm::vec3 direction, float distance) {
      direction = normalize(direction);
      for (int i = 0; i < faces.size(); i++){
        faces[i].vertices[0] += (distance * direction);
        faces[i].vertices[1] += (distance * direction);
        faces[i].vertices[2] += (distance * direction);
      }
    }

    float getLowestYValue() {
      float minY = std::numeric_limits<float>::infinity();
      // go through each vertex to find the minimum Y value.
      for (int i = 0; i < faces.size(); i++) {
        for (int j = 0; j < 3; j++) {
          const float tempY = faces.at(i).vertices[j].y;
          if (tempY < minY) minY = tempY;
        }
      }
      return minY;
    }

    // Snap To Floor - move object down or up so the lowest vertex is at Y=0.
    void SnapToY0() {
      const float minY = getLowestYValue();
      // go through each vertex and move up by - minY.
      for (int i = 0; i < faces.size(); i++) {
        for (int j = 0; j< 3; j++) {
          faces.at(i).vertices[j] += glm::vec3(0, -minY, 0);
        }
      }
    }

    void Scale(glm::vec3 scale) {
      glm::vec3 centre = GetCentre();
      for (int i = 0; i < faces.size(); i++) {
        faces[i].vertices[0] = centre + (scale * (faces[i].vertices[0] - centre));
        faces[i].vertices[1] = centre + (scale * (faces[i].vertices[1] - centre));
        faces[i].vertices[2] = centre + (scale * (faces[i].vertices[2] - centre));
      }
    }
    
    void Scale_Locked_YMin(glm::vec3 scale) {      
      float minY = std::numeric_limits<float>::infinity();
      int i_faceY = -1;
      int i_vertexY = -1;

      // go through each vertex to find the minimum Y value.
      for (int i = 0; i < faces.size(); i++) {
        for (int j = 0; j < 3; j++) {
          const float tempY = faces.at(i).vertices[j].y;
          if (tempY < minY) {
            minY = tempY;
            i_faceY = i;
            i_vertexY = j;
          }
        }
      }

      Scale(scale);

      const float scaledMinY = faces[i_faceY].vertices[i_vertexY][1]; // This is our current lowest Y value.
      const float distToMoveDown = scaledMinY - minY;
      
      // go through each vertex and move up by - minY.
      for (int i = 0; i < faces.size(); i++) {
        for (int j = 0; j< 3; j++) {
          faces.at(i).vertices[j] += glm::vec3(0, -distToMoveDown, 0);
        }
      }
      
    }

};
