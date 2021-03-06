#ifndef AST_DECLARATOR_HPP
#define AST_DECLARAOTR_HPP

#include "ast_node.hpp"
#include <string>
#include <vector>

class DirectDeclarator : public Node {
private:
  NodePtr dirDec;
  NodePtr idList = NULL;
  std::string val;

public:
  DirectDeclarator(NodePtr _decl,
                   NodePtr _list); // possibly add bolean to specify whether its
                                   // a function or array
  void insert(NodePtr _n) {}
  void pyPrint(std::ostream &os); //; when implementing delete {} and leave ;
  void mipsPrint(std::ostream &os) {} //;
  void getGlobal(std::vector<std::string> &v);
  std::string getName();
  void registerVariables();
};

class Declaration : public Node {
private:
  std::string decSpec;
  NodePtr initDecList;

public:
  Declaration(std::string *_spec, NodePtr _list);
  void insert(NodePtr _n) {}
  void pyPrint(std::ostream &os);
  void mipsPrint();
  void getGlobal(std::vector<std::string> &v);
  void registerVariables();
};

class InitDeclarator : public Node {
private:
  NodePtr declarator;  // will store name of the variable/function + parameters
                       // of function
  NodePtr initializer = NULL; // expression so the value of variable which we pass as
                       // default
public:
  InitDeclarator(NodePtr _d, NodePtr _i);
  void insert(NodePtr _n) {}
  void pyPrint(std::ostream &os);
  void mipsPrint();
  void getGlobal(std::vector<std::string> &v);
  void registerSingleVar();
};

class ParamDeclaration : public Node {
private:
  std::string decSpec;
  NodePtr declarator;

public:
  ParamDeclaration(std::string *_s, NodePtr _d);
  void insert(NodePtr _n) {}
  void pyPrint(std::ostream &os);
  void mipsPrint(std::ostream &os) {}
  void registerSingleVar();
};

#endif