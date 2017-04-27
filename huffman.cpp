#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <utility>
#include <queue>
#include <map>
#include "node.hpp"

using namespace std;

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

void mapChar(map<char,string>& m, Node<Cpair>* root, char c, string key = ""){
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

void printTree(const Node<Cpair>* r){
  cout << r->data << endl;
  if (r->left != NULL) printTree(r->left);
  if (r->right != NULL) printTree(r->right);
}


int main (int argc, char* argv[]){
  if (argc == 1){
    cerr << "No argument given.             \n" <<
            "Usage:                         \n" << 
            "\t~ ./topzip.out <path-to-text>\n" <<
            "Aborting.                      \n";
    exit(-1);
  }
  string stream;
  short frequency[256];
  memset (frequency, 0, sizeof frequency);
  string line;
  ifstream text (argv[1]);
  char c;
  // reading each character of file
  while (text.get(c)){
    frequency[(int) c] ++; // trick so v[c] is frequency of c;
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
  vector<char> ch;
  for (int i = 0 ; i < 256 ; ++i){
    if ( frequency [i] != 0 ){
      f_list.push(Node<Cpair>(Cpair((char) i, frequency[i])));
      ch.push_back((char) i);
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
  map<char,string> code;
  if (root!= NULL) printTree(root);
  for (char c : ch){
    mapChar(code,root,c);
    cout << c << "'s code: "<< code[c] << endl;
  }
  cout << stream ;
  // Generating binary stream of digits
  string bitstream;
  for (char c : stream){
    bitstream += code[c];
  }
  cout << bitstream << endl;
  return 0;
}
