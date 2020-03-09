#include <ModelTriangle.h>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <sstream>

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

// given a string in form 'v 12 5 4' -> return vec3(12,5,4)
vec3 getVertex(string inputLine){
  string *s = split(inputLine, ' ');
  return vec3(stof(s[1]), stof(s[2]), stof(s[3]));
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



vector<ModelTriangle> readOBJ(string objFileName, string mtlFileName, float scalingFactor){

  vector<Colour> colours = readOBJMTL(mtlFileName);

  vector<vec3> vertices;
  vector<ModelTriangle> faces;
  
  // Attempt to open OBJFile.
  ifstream myfile(objFileName);

  // if we cannot open the file, print an error
  if (myfile.is_open() == 0) cout << "Unable to open file" << "\n";
  

  Colour currentCol (255,255,255);

  string line; //used as buffer for ifstream.

  while (getline(myfile, line)){
    if (line.find("usemtl") == 0){
      //Material: retrieve the new colour and use this colour until we get a new one.
      vector<string> colourVector = separateLine(line);
      string colourName = colourVector[1];
      
      //Find the colour in list of saved colours.
      int n = colours.size();
      for (int i = 0 ; i < n ; i++){
        Colour col = colours[i];
        if (colours[i].name == colourName){
          currentCol = colours[i];
          break;
        }
      }
    }
    else if (line.find('v') == 0){
      //Vertex: put it in a vec3 and store it with all other vertices.
      vec3 vertex = getVertex(line);
      vertex = vertex * scalingFactor;
      vertices.push_back(vertex);
    }
    else if (line.find('f') == 0){
      //Face: get the corresponding vertices and store it as ModelTriangle object into the collection of faces.
      faces.push_back(getFace(line, vertices, currentCol, scalingFactor)); 
    }
  }

  myfile.close();

  return faces;
}