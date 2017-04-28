#include <iostream>
#include <fstream>

#include <cstring>
#include <vector>
#include <utility>
#include <queue>
#include <map>

#include <cstring>
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

void mapChar(map<unsigned char,string>& m, Node<Cpair>* root, unsigned char c, string key = ""){
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
string unZip( const string& bitstream, const map<uchar, string>& code){
  string aux;
  string unzip;
  for (size_t i = 0 ; i < bitstream.size() ; ++i){
    aux += bitstream[i];
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

void printTree(const Node<Cpair>* r){
  cout << r->data << endl;
  if (r->left != NULL) printTree(r->left);
  if (r->right != NULL) printTree(r->right);
}

unsigned short bit2short( unsigned char* bytes ){
  return (short) (((short)bytes[1]) << 8) | bytes[0];
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
    memset (frequency, 0, sizeof frequency);
    string line;
    char c;
    // reading each character of file
    while (text.get(c)){
      unsigned char aux = c;
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
    vector<unsigned char> ch;
    for (int i = 0 ; i < 256 ; ++i){
      if ( frequency [i] != 0 ){
        f_list.push(Node<Cpair>(Cpair((unsigned char) i, frequency[i])));
        ch.push_back((unsigned char) i);
      }
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
    // We instantiate a binary tree with root as
    // the main root resulted from building of the tree.
    map<unsigned char,string> code;
    if (root!= NULL) printTree(root);
    for (unsigned char c : ch){
      mapChar(code,root,c);
    }
    // Generating binary stream of digits
    string bitstream;
    for (unsigned char c : stream){
      bitstream += code[c];
    }
    // bitstream is the text's binary code. Now we need to pack
    // the frequency list information into our binary .top file,
    // so we can build the tree back again and hence read the
    // zipped file.
    // This binary code will follow the preceding protocol:
    // a char n : 1 byte for the number of nonzero frequencies
    // preceding n*3 chars (bytes) :
    //  for each 3 bytes:
    //    first byte: related character
    //    next 2 bytes: the 2 bytes corresponding to the short
    //                   binary representation of the character's
    //                   representation.
    string unzip = unZip(bitstream, code);
    cout << "Number of characters in text:" << root->data.second << endl;
    cout << "Number of characters in bitstream:" << ((bitstream.size() >> 3) +(bitstream.size()%8?0:1)) << endl;
  }
  return 0;
}
