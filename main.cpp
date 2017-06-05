/////////////////////////////////////////////////////////////////
// _____              _                                        //
//|_   _|            (_)                                       //
//  | | ___  _ __ _____ _ __                                   //
//  | |/ _ \| '_ \_  / | '_ \                                  //
//  | | (_) | |_) / /| | |_) |                                 //
//  \_/\___/| .__/___|_| .__/                                  // 
//          | |        | |                                     //
//          |_|        |_|                                     //
//                                                             //
//*For instructions on how to use topzip, see README.md        //
//*For information on this project's license, see LICENSE.md   //
/////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>

#include <cstring>
#include <vector>
#include <queue>
#include <map>
#include <cassert>

#include "node.hpp"

using namespace std;

typedef unsigned char uchar;


// Made this struct so we could pair a character with it's frequency
// and define proper operators so our implementation is simplified.
struct Cpair{
  char first;
  int second;
  Cpair(char c, int v):
    first(c), second(v){}
  bool operator < ( const Cpair& b ) const {
    return second < b.second;
  }
  bool operator > ( const Cpair& b ) const {
    return second > b.second;
  }
};

// This function defines operator for printing our data
ostream& operator<<(std::ostream &os, const Cpair& m) {
    return os << "char: " << m.first << " frequency: " << m.second ;
}


// Takes two characters bit representation and cast it
// to an unsigned short:
// i.e: uchar bits[2] = {168,2} becomes 680
// i.e: uchar bits[2] = {0,2} becomes 520
// i.e: uchar bits[2] = {168,0} becomes 168 
// We simply interpret the array of two chars as
// if they were a short (which, bitwise, may be same as
// unsigned short)
unsigned short bit2short( uchar* bytes ){
  return (unsigned short) (((short)bytes[1]) << 8) | bytes[0];
}

// Gets the tree and map each char to its string binary representation
void mapChar(map<uchar,string>& m, Node<Cpair>* root, uchar c, string key = ""){
  if (root->data.first == c){
    m[c] = key;
    return ;
  } else{
    if (root->left != NULL)
      mapChar(m, root->left, c, key+'0');
    if (root->right!= NULL)
      mapChar(m, root->right, c, key+'1');
  }
}

// Gets a string of ones and zeroes and returns a
// byte (a char))
uchar byte2char ( const string& byte ){
  char aux = 0;
  for (int i = 7 ; i >= 0 ; --i){
    aux = aux | ((byte[7-i]-'0') << i);
  }
  return aux;
}

// Does the same thing as previous function but the
// other way around (takes byte and returns string)
string char2byte (uchar c){
  string aux;
  for ( int i = 7 ; i >= 0 ; --i ){
    aux += ((1 << i) & c) ? '1' : '0';
  }
  return aux;
}

// With the Huffman's tree's root and vector of pairs, we can generate the mappings
// of each character to its Huffman's code. That's what this function does.
map<uchar, string> getMap (Node<Cpair>* root, const vector<Cpair>& f){
  map<uchar, string> code;
  for (auto v : f){
    mapChar(code, root, v.first);
  }
  return code;
}

// Gets the char => binary string mapping and text compressed string
// (from only text code info on, without tree building protocol part)
// and returns the unzipped text.
string unZip( const string& bits, const map<uchar,string>& code){
  string aux;
  string unzip;
  for (size_t i = 0 ; i < bits.size() ; ++i){
    aux += bits[i];
    for (auto it = code.begin() ; it!= code.end() ; ++it){
      if (it->second == aux){
        unzip += it->first;
        aux.clear();
        break;
      }
    }
  }
  return unzip;
}

// Accordingly to our protocol, we use this function to get a
// frequency list out of the bit string representation of the
// zipped file.
vector<Cpair> parseFrequencies(const string& bits){
  vector<Cpair> aux;
  int n = (int) bits[0];
  uchar buf[2];
  for (int i = 2; i < 3*n+2; i+=3){
    buf[0] = bits[i+1];
    buf[1] = bits[i+2];
    aux.push_back(Cpair(bits[i], bit2short(buf)));
  }
  return aux;
}

// Function for generating the tree, return it's node solely.
// And the node is all we need to know everything about the tree.
Node<Cpair>* buildTree (const vector<Cpair>& list){
  Node<Cpair>* root;
  priority_queue <Node<Cpair>, vector<Node<Cpair> >, greater<Node<Cpair>> >  f_list;
  for (auto v : list){
    f_list.push(Node<Cpair>(v));
  }
  while (f_list.size() != 1){
    auto first = f_list.top();
    f_list.pop();
    auto second = f_list.top();
    f_list.pop();
    Cpair r('*', first.data.second+second.data.second);
    root = new Node<Cpair> (r);
    root->left = new Node<Cpair> (first);
    root->right = new Node<Cpair> (second);
    f_list.push(*root);
  }
  return root;
}

// For debugging purposes, this function takes a tree root and
// prints it in preorder.
void printTree(const Node<Cpair>* r){
  cout << r->data << endl;
  if (r->left != NULL) printTree(r->left);
  if (r->right != NULL) printTree(r->right);
}

// Simply cuts off the whole bit string representation of zipped file
// the part related to the text.
string getTextBits( const string& bytes ){
  int n = (int) bytes[0];
  unsigned int rem = (int) bytes[1];
  string aux = bytes.substr(3*n+2);
  string txt;
  for (size_t i = 0 ; i < aux.size(); ++i){
    txt += char2byte(aux[i]);
  }
  return txt.substr(0,txt.size()-rem);
}

int main (int argc, char* argv[]){
  if (argc == 1){
    cerr << "No argument given.             \n" <<
            "Usage:                         \n" << 
            "For help, run ./topzip.out -h  \n" <<
            "Aborting.                      \n";
    exit(-1);
  }
  bool zip = true;
  for (int i = 1 ; i < argc ; ++i){
    if (!strcmp(argv[i], "-h") or !strcmp(argv[i], "--help")){
      cout << "Need help ? Here is how to use it:"<< endl;
      cout << "\t~ ./topzip.out <flags> <path-to-text>\n"<< endl;
      cout << "flags = -u or --unzip for unzipping mode"<< endl;
      cout << "flags = -h or --help for help"<< endl;
      return 0;
    }
    if (!strcmp(argv[i], "-u") or !strcmp(argv[i], "--unzip")){
      zip = false;
    }
  }
  string file (argv[argc-1]);
  if (file.size() >= 4){
    if (file.substr(file.size()-4) == ".top" && zip){
      cerr << "You requested to zip a .top file... That is not cool." << endl;
      cerr << "I'll abort the program before you blow everything up." << endl;
      cerr << "If you want to unzip, use the -u or --unzip flag on execution, i.e:" << endl;
      cerr << "~ ./topzip.out --unzip <awesome-file>.top" << endl;
      exit(-1);
    }
  }
  ifstream in (file);
  ofstream out;
  if (zip)
    file+=".top";
  else
    file= file.substr(0,file.size()-4);
  out.open(file);
  if (!in.is_open() || !out.is_open()){
    cerr << "Could not open file. Aborting" << endl;
    cerr << "(maybe you mispelled it ?)" << endl;
    exit (-1);
  }
  if (zip){
    cout << "Entered in zipping mode..." << endl;
    string stream;
    int frequency[256];
    vector<Cpair> frequencies;
    memset (frequency, 0, sizeof frequency);
    char c;
    char n = 0 ;
    // reading each character of file
    while (in.get(c)){
      uchar aux = c;
      frequency[aux] ++; // trick so v[c] is frequency of c;
      stream += c;
    }
    for (int i = 0 ; i < 256 ; ++i){
      if ( frequency [i] != 0 ){
        n++;
        frequencies.push_back((Cpair((uchar) i, frequency[i])));
      }
    }
    // Building tree
    // We turn first 2 elements of the priority queue sorted in increasing
    // order of frequency into leafes of a root element with frequency
    // as the sum of both. Then we push this root to the priority queue
    // and repeat the process untill we are left with only one element.
    Node<Cpair>* root = buildTree(frequencies);
    // Map chars to its string of 1's and 0's
    map<uchar,string> code = getMap(root, frequencies);
    // Generating binary stream of digits
    string bitstream;
    for (uchar c : stream){
      bitstream += code[c];
    }
    // bitstream is the text's binary code. Now we need to pack
    // the frequency list information into our binary .top file,
    // so we can build the tree back again and hence read the
    // zipped file.
    // This binary code will follow the preceding protocol:
    // a char n : 1 byte for the number of nonzero frequencies
    // a char rem : 1 byte for the number of forgotten last bits
    // (3 bits would be enough, but that's too less for a variable)
    // preceding n*3 chars (bytes) :
    //  for each 3 bytes:
    //    first byte: related character
    //    next 2 bytes: the 2 bytes corresponding to the short
    //                   binary representation of the character's
    //                   representation.
    // First we complete final bits
    char rem = 0;
    for (size_t i = 0 ; i < bitstream.size()%8 ; ++i){
      rem ++;
      bitstream += '0';
    }
    // Packing prefix for rebuilding the tree on unzipping
    string prefix, postfix;
    prefix += n;
    prefix += rem;
    for (auto f: frequencies){
      uchar buf[2];
      unsigned short aux = (short) f.second;
      buf[1] = (char) (aux>> 8);
      buf[0] = (char) (aux);
      prefix += f.first;
      prefix += buf[0];
      prefix += buf[1];
    }
    for (size_t i = 0; i < bitstream.size() ; i+=8){
      postfix += byte2char(bitstream.substr(i,i+8));
    }
    bitstream = prefix+postfix;
    out << bitstream;
    string txt = getTextBits(bitstream);
    string unzip = unZip(txt, code);
    cout << "Number of chars in text:" << root->data.second << endl;
    cout << "Number of chars in zipped file:" << (bitstream.size() >> 3) << endl;
    cout << unzip ;
  } else {
    cout << "Entered in unzipping mode..." << endl;
    string stream;
    char c;
    while (in.get(c)){
      stream += c;
    }
    // Get frequency list
    vector<Cpair> frequencies = parseFrequencies(stream);
    // Get text side
    string txt = getTextBits(stream); 
    // Build Huffman's tree from frequency list
    Node<Cpair>* root = buildTree(frequencies);
    // Get mappings from tree
    map<uchar,string> code = getMap(root, frequencies);
    // Unzip the hell out of text!!
    string unzip = unZip(txt, code);
    cout << "Unzipped text:\n" <<  unzip;
    out << unzip;
  }
  return 0;
}
