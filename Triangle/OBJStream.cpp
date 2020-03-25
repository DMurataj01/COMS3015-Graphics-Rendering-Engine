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
  vector<string> points = separateLine(inputLine);
  vec3 output;
  output[0] = stof(points[1]); // it is points[1] as the first element is 'v'
  output[1] = stof(points[2]);
  output[2] = stof(points[3]);
  return output;
}

//vec3 getVertex(string inputLine){
//  string *s = split(inputLine, ' ');
//  return vec3(stof(s[1]), stof(s[2]), stof(s[3]));
//}

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



/* STRUCTURE - ImageFile */
struct ImageFile {
  vector<Colour> vecPixelList;
  int width;
  int height;  
};

Colour getPixelColour(ImageFile imageFile, int x, int y) {
  int index = (imageFile.width * y) + x;

  if ((index >= 0) && (index < imageFile.vecPixelList.size())){
    return imageFile.vecPixelList.at(index);
  } else {
    cout << "Index out of range from imageFile. [" << x << ", " << y << " \n";
    throw 0;
  }
}

string removeLeadingWhitespace(string s){
  s.erase(0, s.find_first_not_of(" "));
  return s;
}

/* Read PPM P6 File */
ImageFile readPPMImage(string fileName) {
  std::ifstream ifs;
  ifs.open (fileName, std::ifstream::in);

  /* Parse Header */

  //Check if header is a P6 file.
  string headerInput = "";
  getline(ifs, headerInput);

  if (headerInput != "P6") {
    cout << "Error - Header file is invalid";
    ifs.close();
    throw 1;
  }

  int width = -1;
  int height = -1;
  int maxvalue = -1;

  /* Following Specification: http://netpbm.sourceforge.net/doc/ppm.html */ 
  // 1) Check if header is a P6 file.
  // 2) Ignore Comments.
  // 3) Parse Width + whitespace + Height.
  // 4) Parse Max value

  while (true || !ifs.eof()) {
    string inputLine = "";
    getline(ifs,inputLine);
    inputLine = removeLeadingWhitespace(inputLine);
    if (inputLine[0] == '#'){
      //This is a comment line -> ignore them.
    }
    else {
      //Parse Width + Height.
      stringstream ss_wh(inputLine);
      ss_wh >> width >> height;
      
      //Read new line -> Parse Max value: // 0<val<65536.
      getline(ifs,inputLine);
      inputLine = removeLeadingWhitespace(inputLine);
      
      stringstream ss_ml(inputLine);
      ss_ml >> maxvalue;

      cout << "Header Parse --  Width: " << width << " -- Height: " << height << " -- Max Value: " << maxvalue << "\n";
      break;
    }
  }
  
  /* Body RGB Parse */
  vector<Colour> vecPixelList; //RGB storage.

  while (ifs.peek()!= EOF) {
    vecPixelList.push_back(Colour ({ifs.get(), ifs.get(), ifs.get()})); //Create a Pixel element with three consecutive byte reads.
  }
  cout << "Body Parse -- Number of elements: " << vecPixelList.size() << "\n";
  ifs.close();
  ImageFile outputImageFile = ImageFile ({vecPixelList, width, height});
  return outputImageFile;
}

void renderImageFile(DrawingWindow window, ImageFile imageFile){
  for (int i=0; i<imageFile.vecPixelList.size(); i++){
    int red = imageFile.vecPixelList.at(i).red;
    int green = imageFile.vecPixelList.at(i).green;
    int blue = imageFile.vecPixelList.at(i).blue;
    uint32_t colour = (255<<24) + (red<<16) + (green<<8) + blue;
    int row = int(i/imageFile.width);
    int col = i - (row*imageFile.width);
    window.setPixelColour(col, row, colour);
  }
  cout << "\nImage render complete.\n";
}