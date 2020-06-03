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
  return vec3(stof(points[1]), stof(points[2]), stof(points[3])); 
} 
// given a string in form 'vt 12 5' it returns (12,5) 
vec2 getVertexTexture(string inputLine){ 
  vector<string> points = separateLine(inputLine); 
  return vec2(stof(points[1]), stof(points[2])); 
} 

// given a line in the form 'f 1/ 2/ 3/' and also all vertices found so far  
// it returns a ModelTriangle object, which contains the 3 vertices and the colour too 
ModelTriangle getFace(string inputLine, vector<vec3> vertices, Colour colour, float scalingFactor){ 
  ModelTriangle output; 
  output.colour = colour; 
 
  vector<string> faces = separateLine(inputLine); // this turns 'f 1/ 2/ 3/' into ['f','1/','2/','3/'] 
  for (int i = 1 ; i < 4 ; i++){ 
    string element = faces[i]; // this will be '1/' for example 
    int slashIndex = element.find('/'); 
    string number = element.substr(0, slashIndex); 
    int index = stoi(number); 
    vec3 vertex = vertices[index-1]; // -1 as the vertices are numbered from 1, but c++ indexes from 0 
    vertex *= scalingFactor; 
    output.vertices[i-1] = vertex; // the -1 here comes from the face that we skip the 'f' in the vector and so i starts at 1 
  } 
 
  return output; 
} 

ModelTriangle getFaceTex(string inputLine, vector<vec3> vertices, vector<vec2> verticesTextures, float scalingFactor){
  // initialise the output
  ModelTriangle output;

  vector<string> faces = separateLine(inputLine); // this turns 'f 1/1 2/2 3/3' into ['f','1/1','2/2','3/3']
  for (int i = 1 ; i < 4 ; i++){
    string element = faces[i]; // this will be '1/1' for example

    //cout << "el: " << element << "\n";
    int slashIndex = element.find('/');
    string number_pre_slash = element.substr(0, slashIndex);
    string number_post_slash = element.substr(slashIndex+1, element.length());
    int i_v = stoi(number_pre_slash);
    int i_vt = stoi(number_post_slash);
    //cout << "a: " << i_v << " .. : " << i_vt << "\n";
    //Set Vertex
    output.vertices[i-1] = scalingFactor * vertices[i_v-1]; // the -1 here comes from the face that we skip the 'f' in the vector and so i starts at 1
    output.vertices_textures[i-1] = verticesTextures[i_vt-1];
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

vector<Object> readGroupedOBJ(std::string objFileName, std::string mtlFileName, float scalingFactor) {
  vector<Colour> colours = readOBJMTL(mtlFileName); 
  vector<vec3> vertices; 
  vector<vec2> verticesTextures;
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
  int i_group = 0; // 0 based index.
  bool hasGroup = false;
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
      hasGroup = true;
    }
    else if (line.find("vt") == 0) {
      verticesTextures.push_back(getVertexTexture(line));
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
      ModelTriangle triangle;
      //If we have read no vertex textures
      if (verticesTextures.size() == 0) triangle = getFace(line, vertices, colour, scalingFactor);
      else triangle = getFaceTex(line, vertices, verticesTextures, scalingFactor);
      triangle.objectIndex = (hasGroup) ? (i_group - 1) : 0; 
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

  const int groupSize = (hasGroup) ? i_group : 1;
  vector<Object> outputList(groupSize);
  vector<int> perObjectFaceIndex(groupSize);

  for (int i=0; i< i_group; i++) perObjectFaceIndex.at(i) = 0;
  

  // go through the faces again and store the average normal in the ModelTriangle object
  for (int i = 0 ; i < faces.size(); i++){
    ModelTriangle face = faces[i];
     
    // for each vertex
    face.normals[0] = averagedNormals[faceVertices[i][0]];
    face.normals[1] = averagedNormals[faceVertices[i][1]];
    face.normals[2] = averagedNormals[faceVertices[i][2]];

    const int objectIndex = face.objectIndex;
    face.faceIndex = perObjectFaceIndex[objectIndex];
    perObjectFaceIndex[objectIndex]++;
    outputList[objectIndex].faces.push_back(face);
  }
  //At this point we have i_group GROUPS.
  return outputList;
}
#endif