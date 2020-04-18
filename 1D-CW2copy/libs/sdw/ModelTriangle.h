#include <glm/glm.hpp>
#include <Colour.h>
#include <string>

class ModelTriangle
{
  public:
    glm::vec3 vertices[3];
    glm::vec2 vertices_textures[3];
    Colour colour;
    glm::vec3 normals[3]; // these correspond to the averaged normals of the vertices
    std::string texture;
    int faceIndex; // stores the number of which face it is out of all of them
    bool culled; // has this face been culled or not? do we need to check for intersections with it?

    ModelTriangle()
    {
      normals[0] = glm::vec3 (0,0,0);
      normals[1] = glm::vec3 (0,0,0);
      normals[2] = glm::vec3 (0,0,0);
      texture = "none";
      culled = false;
      faceIndex = -1;
    }

    ModelTriangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Colour trigColour)
    {
      vertices[0] = v0;
      vertices[1] = v1;
      vertices[2] = v2;
      colour = trigColour;
      normals[0] = glm::vec3 (0,0,0);
      normals[1] = glm::vec3 (0,0,0);
      normals[2] = glm::vec3 (0,0,0);
      texture = "none";
      culled = false;
      faceIndex = -1;
    }
};

std::ostream& operator<<(std::ostream& os, const ModelTriangle& triangle)
{
    os << "(" << triangle.vertices[0].x << ", " << triangle.vertices[0].y << ", " << triangle.vertices[0].z << ")" << std::endl;
    os << "(" << triangle.vertices[1].x << ", " << triangle.vertices[1].y << ", " << triangle.vertices[1].z << ")" << std::endl;
    os << "(" << triangle.vertices[2].x << ", " << triangle.vertices[2].y << ", " << triangle.vertices[2].z << ")" << std::endl;
    os << std::endl;
    return os;
}
