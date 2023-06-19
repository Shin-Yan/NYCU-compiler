#ifndef AST_AST_NODE_H
#define AST_AST_NODE_H

#include <cstdint>
#include <vector>
#include <string>
#include "visitor/AstNodeVisitor.hpp"

class AstNodeVisitor;

struct Location {
    uint32_t line;
    uint32_t col;

    ~Location() = default;
    Location(const uint32_t line, const uint32_t col) : line(line), col(col) {}
};

struct Unary_Op{
  bool neg = false;
  bool NOT = false;
};

struct Binary_Op{
  bool mul = false;
  bool div = false;
  bool mod = false;
  bool add = false;
  bool sub = false;
  bool lt = false;
  bool le = false;
  bool eq = false;
  bool ne = false;
  bool ge = false;
  bool gt = false;
  bool AND = false;
  bool OR = false;
};

struct Constant_Value{
  uint32_t line;
  uint32_t col;
  int int_value;
  float real_value;
  char *str_value;
  bool int_type = false;
  bool real_type = false;
  bool str_type = false;
  bool bool_type = false;
};

struct Name{
  char *id;
  uint32_t line;
  uint32_t col;
};

class AstNode {
  public:
    virtual ~AstNode() = 0;
    AstNode(const uint32_t line, const uint32_t col);

    const Location &getLocation() const;

    virtual void print() = 0;
    virtual void accept(AstNodeVisitor &p_visitor) = 0;

  protected:
    Location location;

};

#endif
