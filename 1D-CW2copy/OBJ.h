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


vector<Object> readGroupedOBJ(std::string objFileName, std::string mtlFileName, float scalingFactor) {

  int currentFaceIndex = 0; // stores the number of how many faces we have gone through yet 
  vector<vec3> faceVertices; // stores the indices of which vertices make up each face 
  
  // where we store all the vertices and faces 
  vector<vec3> vertices; 
  vector<ModelTriangle> faces; 
  vector<Colour> colours = readOBJMTL(mtlFileName); 

  // Attempt to open OBJFile. 
  ifstream myfile(objFileName); 
 
  // if we cannot open the file, print an error 
  if (myfile.is_open() == 0) cout << "Unable to open file" << "\n"; 
 
    
  Colour colour (255,255,255); //Buffer - the correct colour for each face once found. 
  
  string line; //used as buffer for ifstream. 
 
  // go through and figure out how many vertices we have  
  int groupCount = 0;
  int numberOfFaces = 0;
  int numberOfVertices = 0; 

  while (getline(myfile, line)){ 
    if (line.find('g') == 0) groupCount++;
    else if (line.find('v') == 0) numberOfVertices++;  
    else if (line.find('f') == 0){ 
      numberOfFaces++;
      ModelTriangle triangle = getFace(line, vertices, colour, scalingFactor);
      triangle.faceIndex = (numberOfFaces-1);
      faces.push_back(triangle);
    }
  } 
 
  myfile.close(); 
  
  //                         //
  //                         //
  // Ready To Create Objects //
  //                         //
  //                         //

  vector<Object> objectList;

  Object currentObject; //Buffer - the current object once found.


  // open the obj file and then go through each line storing it in a string 
  ifstream myfile2(objFileName); 
 
  // if we cannot open the file, print an error 
  if (myfile2.is_open() == 0){ 
    cout << "Unable to open file" << "\n"; 
  } 
 

  vector<vector<vec3>> vertexNormals(numberOfVertices); 
  vector<vec3> averagedNormals(numberOfVertices); // once all normals for each vertex have been found, this stores the average of them 
  // take the OBJ file again and go through each face, get the normal and then store that normal 
  // in the vector for all 3 vertices 
  
  while (getline(myfile2, line)){ 
    if (line.find("usemtl") == 0){ 
      vector<string> colourVector = separateLine(line); 
      string colourName = colourVector[1]; 
      // now go through each of the colours we have saved until we find it 
      int n = colours.size(); 
      for (int i = 0 ; i < n ; i++){ 
        Colour col = colours[i]; 
        if (colours[i].name == colourName){ 
          colour = colours[i]; 
          break; // can stop once we have found it 
        } 
      } 
    } 
    else if (line.find('g') == 0){ 
      groupCount++;
      if (groupCount == 1) {
        //This is the first instance.
        cout << "1st Group\n";
      } else {
        // Push this Object Group.
        objectList.push_back(currentObject);
        currentObject.Clear();
        cout << "Group End - New Group\n";
      }

    }
    // if we have a vertex, then put it in a vec3 and store it with all other vertices 
    else if (line.find('v') == 0){ 
      numberOfVertices++; 
      vec3 vertex = getVertex(line); 
      vertex *= scalingFactor; 
      vertices.push_back(vertex); 
    } 
    else if (line.find('f') == 0){   
      // if this line is a face, then get the normal of it (we have pre-stored the faces so can do this) 
      ModelTriangle face = faces[currentFaceIndex]; 
      vec3 normal = face.getNormal(); 
      currentFaceIndex++; 
      vector<string> faceLine = separateLine(line); // this turns 'f 1/1 2/2 3/3' into ['f','1/1','2/2','3/3'] 

      vec3 verticesIndex (0,0,0); 
      // go through the face and for each vertex, store the normal in the corresponding space in the vector 
      for (int i = 1 ; i < 4 ; i++){ 
        string element = faceLine[i]; // this will be '1/1' for example 
        int slashIndex = element.find('/'); 
        string number = element.substr(0, slashIndex); 
        int index = stoi(number) - 1;// -1 as the vertices are numbered from 1, but c++ indexes from 0 
        verticesIndex[i-1] = index; 
        vertexNormals[index].push_back(normal); 
      } 
      faceVertices.push_back(verticesIndex); 
      currentObject.faces.push_back(face); 
    } 
  } 

  myfile2.close(); 


  // for each vertex, go through and average each of the normals 
  for (int i = 0 ; i < numberOfVertices ; i++){ 
    vector<vec3> normals = vertexNormals[i]; 
    int n = normals.size(); 
    vec3 sum (0,0,0); 
    for (int j = 0 ; j < n ; j++){ 
      sum += normals[j]; 
    } 
    averagedNormals[i] = sum / (float(n)); 
  } 


  // go through the faces again and store the average normal in the ModelTriangle object 
  for (int i = 0 ; i < faces.size() ; i++){ 
    vec3 v = faceVertices[i]; 
    // for each vertex 
    for (int j = 0 ; j < 3 ; j++){ 
      int index = v[j]; 
      vec3 normal = averagedNormals[index]; 
      faces[i].normals[j] = normal; 
    } 
  } 

  objectList.push_back(currentObject);
  cout << "Group End\n";
  return objectList;
}


vector<ModelTriangle> readOBJNormal(std::string objFileName, std::string mtlFileName, float scalingFactor){ 
  // get the colours 
  vector<Colour> colours = readOBJMTL(mtlFileName); 
 
  // where we store all the vertices and faces 
  vector<vec3> vertices; 
  vector<ModelTriangle> faces; 
   
  // open the obj file and then go through each line storing it in a string 
  string line; 
  ifstream myfile(objFileName); 
 
  // if we cannot open the file, print an error 
  if (myfile.is_open() == 0){ 
    cout << "Unable to open file" << "\n"; 
  } 
 
  // this is where we will save the correct colour for each face once found 
  Colour colour (255,255,255); 
 
  int numberOfFaces = 0;
  // while we have a new line, get it 
  while (getline(myfile, line)){ 
    // if the line starts with 'usemtl', then we retrieve the new colour and use this colour until we get a new one 
    if (line.find("usemtl") == 0){ 
      vector<string> colourVector = separateLine(line); 
      string colourName = colourVector[1]; 
       
      // now go through each of the colours we have saved until we find it 
      int n = colours.size(); 
      for (int i = 0 ; i < n ; i++){ 
        Colour col = colours[i]; 
        if (colours[i].name == colourName){ 
          colour = colours[i]; 
          break; // can stop once we have found it 
        } 
      } 
    } 
    // if we have a vertex, then put it in a vec3 and store it with all other vertices 
    else if (line.find('v') == 0){ 
      vec3 vertex = getVertex(line); 
      vertex = vertex * scalingFactor; 
      vertices.push_back(vertex); 
    } 
    // if we have a face, then get the corresponding vertices and store it as a ModelTriangle object, then add it to the collection of faces 
    else if (line.find('f') == 0){ 
      ModelTriangle triangle = getFace(line, vertices, colour, scalingFactor);
      triangle.faceIndex = numberOfFaces;
      numberOfFaces = numberOfFaces + 1;
      faces.push_back(triangle); 
    } 
  } 


  // ** NOW COMPUTE NORMALS ** 

  int currentFaceIndex = 0; // stores the number of how many faces we have gone through yet
  vector<vec3> faceVertices; // stores the indices of which vertices make up each face

  // Attempt to open OBJFile.
  ifstream myfile3(objFileName);

  // if we cannot open the file, print an error
  if (myfile.is_open() == 0) cout << "Unable to open file" << "\n";

  // go through and figure out how many vertices we have
  int numberOfVertices = 0;
  while (getline(myfile3, line)){
    if (line.find('v')==0){
      numberOfVertices++;
    }
  }

  // open and close the file again
  myfile3.close();

  ifstream myfile2(objFileName);

  // make a vector to store the normals for each vertex (so each element in the vector will be another
  // vector containing all the normals of the faces in which that vertex is a part of)
  vector<vector<vec3>> vertexNormals(numberOfVertices);
  vector<vec3> averagedNormals(numberOfVertices); // once all normals for each vertex have been found, this stores the average of them

  // take the OBJ file again and go through each face, get the normal and then store that normal
  // in the vector for all 3 vertices


  while (getline(myfile2, line)){
    if (line.find('f') == 0){
      // if this line is a face, then get the normal of it (we have pre-stored the faces so can do this)
      ModelTriangle face = faces[currentFaceIndex];
      vec3 normal = face.getNormal();
      currentFaceIndex++;

      vector<string> faceLine = separateLine(line); // this turns 'f 1/1 2/2 3/3' into ['f','1/1','2/2','3/3']

      vec3 verticesIndex (0,0,0);

      // go through the face and for each vertex, store the normal in the corresponding space in the vector
      for (int i = 1 ; i < 4 ; i++){
        string element = faceLine[i]; // this will be '1/1' for example
        int slashIndex = element.find('/');
        string number = element.substr(0, slashIndex);
        int index = stoi(number) - 1;// -1 as the vertices are numbered from 1, but c++ indexes from 0
        verticesIndex[i-1] = index;
        vertexNormals[index].push_back(normal);
      }
      faceVertices.push_back(verticesIndex);
    }
  }

  myfile2.close();

  // for each vertex, go through and average each of the normals
  for (int i = 0 ; i < numberOfVertices ; i++){
    vector<vec3> normals = vertexNormals[i];
    int n = normals.size();
    vec3 sum (0,0,0);
    for (int j = 0 ; j < n ; j++){
      sum += normals[j];
    }
    averagedNormals[i] = sum / (float(n));
  }

  // go through the faces again and store the average normal in the ModelTriangle object
  for (int i = 0 ; i < faces.size() ; i++){
    vec3 v = faceVertices[i];
    // for each vertex
    for (int j = 0 ; j < 3 ; j++){
      int index = v[j];
      faces[i].normals[j] = averagedNormals[index];
    }
  }






  //cout << "Printing the faces: \n"; 
  //for (int i = 0 ; i < faces.size() ; i++){ 
  //  cout << faces[i] << "\n"; 
  //} 
  return faces; 
} 


vector<ModelTriangle> readOBJ(std::string objFileName, std::string mtlFileName, float scalingFactor){ 
  // get the colours 
  vector<Colour> colours = readOBJMTL(mtlFileName); 
 
  // where we store all the vertices and faces 
  vector<vec3> vertices; 
  vector<ModelTriangle> faces; 
   
  // open the obj file and then go through each line storing it in a string 
  string line; 
  ifstream myfile(objFileName); 
 
  // if we cannot open the file, print an error 
  if (myfile.is_open() == 0){ 
    cout << "Unable to open file" << "\n"; 
  } 
 
  // this is where we will save the correct colour for each face once found 
  Colour colour (255,255,255); 
 
  int numberOfFaces = 0;
  // while we have a new line, get it 
  while (getline(myfile, line)){ 
    // if the line starts with 'usemtl', then we retrieve the new colour and use this colour until we get a new one 
    if (line.find("usemtl") == 0){ 
      vector<string> colourVector = separateLine(line); 
      string colourName = colourVector[1]; 
       
      // now go through each of the colours we have saved until we find it 
      int n = colours.size(); 
      for (int i = 0 ; i < n ; i++){ 
        Colour col = colours[i]; 
        if (colours[i].name == colourName){ 
          colour = colours[i]; 
          break; // can stop once we have found it 
        } 
      } 
    } 
 
    // if we have a vertex, then put it in a vec3 and store it with all other vertices 
    else if (line.find('v') == 0){ 
      vec3 vertex = getVertex(line); 
      vertex = vertex * scalingFactor; 
      vertices.push_back(vertex); 
    } 
 
    // if we have a face, then get the corresponding vertices and store it as a ModelTriangle object, then add it to the collection of faces 
    else if (line.find('f') == 0){ 
      ModelTriangle triangle = getFace(line, vertices, colour, scalingFactor);
      triangle.faceIndex = numberOfFaces;
      numberOfFaces = numberOfFaces + 1;
      faces.push_back(triangle); 
    } 
  } 
  //cout << "Printing the faces: \n"; 
  //for (int i = 0 ; i < faces.size() ; i++){ 
  //  cout << faces[i] << "\n"; 
  //} 
  return faces; 
} 

#endif