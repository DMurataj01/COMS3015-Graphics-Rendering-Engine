#ifndef OBJ_H
#define OBJ_H

#ifndef VECTOR_H
#define VECTOR_H
#include <vector>
#endif

#ifndef OBJECT_H
#define OBJECT_H
#include "Object.h"
#endif

#ifndef FSTREAM_H
#define FSTREAM_H
#include <fstream>
#endif

#ifndef MODELTRIANGLE_H
  #define MODELTRIANGLE_H
  #include <ModelTriangle.h>
#endif

using namespace std;
using namespace glm;

// given a line, say 'v 12 3 5', it retreives the values inbetween the spaces, outputting them as strings in a vector 
vector<string> separateLine(string inputLine){ 
  inputLine = inputLine + " "; // we add a space at the end to we know when to end; 
  vector<string> output; 
  int n = inputLine.length(); 
  int spaceIndex = inputLine.find(' '); 
  // if we have a space in the line still 
  while ((spaceIndex > 0) && (spaceIndex <= n)){ 
    string value = inputLine.substr(0, spaceIndex); 
    output.push_back(value); 
    inputLine = inputLine.substr(spaceIndex + 1, n); 
    n = inputLine.length(); 
    spaceIndex = inputLine.find(' '); 
  } 
  return output; 
} 

// given a string in form 'v 12 5 4' it returns (12,5,4) 
vec3 getVertex(string inputLine){ 
  vector<string> points = separateLine(inputLine); 
  vec3 output; 
  output[0] = stof(points[1]); // it is points[1] as the first element is 'v' 
  output[1] = stof(points[2]); 
  output[2] = stof(points[3]); 
  return output; 
} 

// given a line in the form 'f 1/1 2/2 3/3' and also all vertices found so far  
// it returns a ModelTriangle object, which contains the 3 vertices and the colour too 
ModelTriangle getFace(string inputLine, vector<vec3> vertices, Colour colour, float scalingFactor){ 
  // initialise the output 
  ModelTriangle output; 
  output.colour = colour; 
 
  vector<string> faces = separateLine(inputLine); // this turns 'f 1/1 2/2 3/3' into ['f','1/1','2/2','3/3'] 
  for (int i = 1 ; i < 4 ; i++){ 
    string element = faces[i]; // this will be '1/1' for example 
    int slashIndex = element.find('/'); 
    string number = element.substr(0, slashIndex); 
    int index = stoi(number); 
    vec3 vertex = vertices[index-1]; // -1 as the vertices are numbered from 1, but c++ indexes from 0 
    vertex = vertex * scalingFactor; 
    output.vertices[i-1] = vertex; // the -1 here comes from the face that we skip the 'f' in the vector and so i starts at 1 
  } 
 
  return output; 
} 


// this function reads in an OBJ material file and stores it in a vector of Colour objects, so an output may look like: 
// [['Red','1','0','0'] , ['Green','0','1','0']] 
vector<Colour> readOBJMTL(string filename){ 
  // store the colours in a vector of Colours 
  vector<Colour> colours; 
  // open the file, we then go through each line storing it in a string 
  string line; 
  ifstream myfile(filename); 
   
  if (myfile.is_open()){ 
    while (getline(myfile,line)){ 
      // if we have a new material, store it 
      if (line.find("newmtl") == 0){ 
        Colour colour; 
        vector<string> lineVector = separateLine(line); 
        colour.name = lineVector[1]; 
        // get new line to store the values 
        getline(myfile,line); 
        vector<string> values = separateLine(line); 
        colour.red = stof(values[1]) * 255; 
        colour.green = stof(values[2]) * 255; 
        colour.blue = stof(values[3]) * 255; 
        colours.push_back(colour); 
      } 
    } 
  } 
  else cout << "Unable to open file"; 
 
  return colours; 
} 


vector<ModelTriangle> readOBJ(std::string objFileName, std::string mtlFileName, float scalingFactor){ 
  vector<Colour> colours = readOBJMTL(mtlFileName); 
  vector<vec3> vertices; 
  vector<vec3> faceVertices; // stores the indices of which vertices make up each face
  vector<ModelTriangle> faces; 
    

  string line; //buffer to store each read line.

  ifstream myfile(objFileName); 
  if (myfile.is_open() == 0) cout << "Unable to open file" << "\n"; 
  
  Colour colour (255,255,255); // buffer to store colour.

  vector<vector<vec3>> vertexNormals;
  vector<vec3> averagedNormals; // once all normals for each vertex have been found, this stores the average of them


  int numberOfFaces = 0;
  int numberOfVertices = 0;


  int current_i = 0; // triangles index.
  while (getline(myfile, line)){ 

    if (line.find("usemtl") == 0){ 
      vector<string> colourVector = separateLine(line); 
      // now go through each of the colours we have saved until we find it 
      for (int i = 0; i < colours.size(); i++){ 
        Colour col = colours[i]; 
        if (colours[i].name == colourVector[1]){ 
          colour = colours[i]; break; // optimised - stop once found.
        } 
      } 
    } 
    // if we have a vertex, then put it in a vec3 and store it with all other vertices 
    else if (line.find('v') == 0){ 
      numberOfVertices++;
      averagedNormals.push_back(vec3());
      vertexNormals.push_back(vector<vec3>());
      vertices.push_back(scalingFactor * getVertex(line)); 
    } 
    // if we have a face, then get the corresponding vertices and store it as a ModelTriangle object, then add it to the collection of faces 
    else if (line.find('f') == 0){ 
      ModelTriangle triangle = getFace(line, vertices, colour, scalingFactor);
      triangle.faceIndex = numberOfFaces;
      numberOfFaces++;
      faces.push_back(triangle); 

      // if this line is a face, then get the normal of it (we have pre-stored the faces so can do this)
      vector<string> faceLine = separateLine(line); // this turns 'f 1/1 2/2 3/3' into ['f','1/1','2/2','3/3']
      vec3 verticesIndex(0,0,0);

      // go through the face and for each vertex, store the normal in the corresponding space in the vector
      for (int i = 1 ; i < 4 ; i++){
        string element = faceLine[i]; // this will be '1/1' for example
        int slashIndex = element.find('/');
        string number = element.substr(0, slashIndex);
        int index = stoi(number) - 1;// -1 as the vertices are numbered from 1, but c++ indexes from 0
        verticesIndex[i-1] = index;
        vertexNormals[index].push_back(faces[current_i].getNormal());
      }
      faceVertices.push_back(verticesIndex);
      current_i++;
      
    } 
  } 

  // for each vertex, go through and average each of the normals
  for (int i = 0 ; i < numberOfVertices; i++){
    vec3 sum (0,0,0);
    const int n = vertexNormals[i].size();
    for (int j = 0; j < n; j++){
      sum += vertexNormals[i][j]; //Sum up each dimension.
    }
    averagedNormals[i] = sum / (float(n));
  }

  // go through the faces again and store the average normal in the ModelTriangle object
  for (int i = 0 ; i < faces.size() ; i++){
    // for each vertex
    for (int j = 0 ; j < 3 ; j++) {
      int index = faceVertices[i][j];
      faces[i].normals[j] = averagedNormals[index];
    }
  }

  return faces; 
} 

vector<Object> readGroupedOBJ(std::string objFileName, std::string mtlFileName, float scalingFactor) {
  vector<Colour> colours = readOBJMTL(mtlFileName); 
  vector<vec3> vertices; 
  vector<vec3> faceVertices; // stores the indices of which vertices make up each face
  vector<ModelTriangle> faces; 
    

  string line; //buffer to store each read line.

  ifstream myfile(objFileName); 
  if (myfile.is_open() == 0) cout << "Unable to open file" << "\n"; 
  
  Colour colour (255,255,255); // buffer to store colour.

  vector<vector<vec3>> vertexNormals;
  vector<vec3> averagedNormals; // once all normals for each vertex have been found, this stores the average of them


  int numberOfVertices = 0;

  int i_faces = 0;
  int i_group = 0;
  while (getline(myfile, line)){ 
    if (line.find("usemtl") == 0){ 
      vector<string> colourVector = separateLine(line); 
      // now go through each of the colours we have saved until we find it 
      for (int i = 0; i < colours.size(); i++){ 
        Colour col = colours[i]; 
        if (colours[i].name == colourVector[1]){ 
          colour = colours[i]; break; // optimised - stop once found.
        } 
      } 
    } 
    else if (line.find('g') == 0) {
      i_group++;
    }
    // if we have a vertex, then put it in a vec3 and store it with all other vertices 
    else if (line.find('v') == 0){ 
      numberOfVertices++;
      averagedNormals.push_back(vec3());
      vertexNormals.push_back(vector<vec3>());
      vertices.push_back(scalingFactor * getVertex(line)); 
    } 
    // if we have a face, then get the corresponding vertices and store it as a ModelTriangle object, then add it to the collection of faces 
    else if (line.find('f') == 0){ 
      ModelTriangle triangle = getFace(line, vertices, colour, scalingFactor);
      triangle.objectIndex = i_group;
      triangle.faceIndex = i_faces;
      faces.push_back(triangle); 

      // if this line is a face, then get the normal of it (we have pre-stored the faces so can do this)
      vector<string> faceLine = separateLine(line); // this turns 'f 1/1 2/2 3/3' into ['f','1/1','2/2','3/3']
      vec3 verticesIndex(0,0,0);

      // go through the face and for each vertex, store the normal in the corresponding space in the vector
      for (int i = 1 ; i < 4 ; i++){
        string element = faceLine[i]; // this will be '1/1' for example
        int slashIndex = element.find('/');
        string number = element.substr(0, slashIndex);
        int index = stoi(number) - 1;// -1 as the vertices are numbered from 1, but c++ indexes from 0
        verticesIndex[i-1] = index;
        vertexNormals[index].push_back(faces[i_faces].getNormal());
      }
      faceVertices.push_back(verticesIndex);
      i_faces++;
      
    } 
  } 

  // for each vertex, go through and average each of the normals
  for (int i = 0 ; i < numberOfVertices; i++){
    vec3 sum (0,0,0);
    const int n = vertexNormals[i].size();
    for (int j = 0; j < n; j++){
      sum += vertexNormals[i][j]; //Sum up each dimension.
    }
    averagedNormals[i] = sum / (float(n));
  }

  // go through the faces again and store the average normal in the ModelTriangle object
  for (int i = 0 ; i < faces.size() ; i++){
    // for each vertex
    for (int j = 0 ; j < 3 ; j++) {
      int index = faceVertices[i][j];
      faces[i].normals[j] = averagedNormals[index];
    }
  }
  Object newObject(faces);
  vector<Object> out;
  out.push_back(newObject);
  return out;
}
#endif