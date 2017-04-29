#include <iostream>
#include <fstream>

#include <cstring>
#include <vector>
#include <utility>
#include <queue>
#include <map>
#include <cstring>
#include <cassert>

#include "node.hpp"

using namespace std;

typedef unsigned char uchar;

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


ostream &operator<<(std::ostream &os, const Cpair &m) { 
    return os << "char: " << m.first << " frequency: " << m.second ;
}


// Takes two characters bit representation and cast it
// to an unsigned short:
// i.e: uchar bits[2] = {168,2} becomes 680
// i.e: uchar bits[2] = {0,2} becomes 520
// i.e: uchar bits[2] = {168,0} becomes 168 
// We simply interpret the array of two chars as
// if they were a short (which, bitwise, are same as
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

void printTree(const Node<Cpair>* r){
  cout << r->data << endl;
  if (r->left != NULL) printTree(r->left);
  if (r->right != NULL) printTree(r->right);
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
      cout << "Entered in unzipping mode." << endl;
      zip = false;
    }
  }
  ifstream text (argv[argc-1]);
  if (!text.is_open()){
    cerr << "Could not open file. Aborting" << endl;
    cerr << "(maybe you mispelled it ?)" << endl;
    exit (-1);
  }
  if (zip){
    string stream;
    int frequency[256];
    vector<Cpair> frequencies;
    memset (frequency, 0, sizeof frequency);
    string line;
    char c;
    char n = 0 ;
    // reading each character of file
    while (text.get(c)){
      uchar aux = c;
      frequency[aux] ++; // trick so v[c] is frequency of c;
      stream += c;
    }
    // Building the Huffman's tree.
    // We turn first 2 elements of the priority queue sorted in increasing
    // order of frequency into leafes of a root element with frequency
    // as the sum of both. Then we push this root to the priority queue
    // and repeat the process untill we are left with only one element.
    
    // before that, we initialize the frequency list priority queue.
    priority_queue <Node<Cpair>, vector<Node<Cpair> >, greater<Node<Cpair>> >  f_list;
    Node<Cpair>* root;
    vector<uchar> ch;
    for (int i = 0 ; i < 256 ; ++i){
      if ( frequency [i] != 0 ){
        n++;
        f_list.push(Node<Cpair>(Cpair((uchar) i, frequency[i])));
        frequencies.push_back((Cpair((uchar) i, frequency[i])));
        ch.push_back((uchar) i);
      }
    }
    // Building tree loop
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
    // We instantiate a binary tree with root as
    // the main root resulted from building of the tree.
    if (root!= NULL) printTree(root);
    map<uchar,string> code;
    for (uchar c : ch){
      mapChar(code,root,c);
    }
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
    assert (bitstream.size()%8 == 0);
    string prefix;
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
    bitstream = prefix+bitstream;
    parseFrequencies(bitstream);
    string unzip = unZip(bitstream, code);
    cout << "Number of characters in text:" << root->data.second << endl;
    cout << "Number of characters in bitstream:" << (bitstream.size() >> 3) << endl;
  }
  return 0;
}
