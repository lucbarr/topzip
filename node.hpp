#pragma once 

template <class T>
struct Node {
  T data;
  Node* left;
  Node* right;
  Node(const T& a): 
    data(a), left(NULL), right(NULL){}
  bool operator > ( const Node& b ) const {
    return data > b.data;
  }
};

