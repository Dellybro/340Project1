/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"
#include <set>


using namespace std;

struct node;

struct REG_node {
  struct REG_node * first_neighbor;
  char first_label;
  struct REG_node * second_neighbor;
  char second_label;
  int id;
};

struct REG {
  struct REG_node * starting;
  struct REG_node * accepting;
};

struct TokenREG{
  struct REG* regularExpression;
  std::string id;
};



struct ParsedInput {
  std::string id;
  std::string lexeme;
};

struct LongestMatched {
  std::string lexeme;
  int position;
};

struct Matched {
  TokenREG *expression;
  LongestMatched *matched;
};

class Parser {
  private:
    LexicalAnalyzer lexer;

    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();


    void parse_input();
    void parse_char_list();
    void parse_tokens_section();
    void parse_token_list();
    void parse_token();
    REG * parse_expr();
    void parse_term();
    void parse_factor();
    void matchSequence(string input_text);
    LongestMatched* match(REG* r, std::string s, int p);
    void findOpenPaths(std::vector<REG_node*>* currentSet, REG_node* currentNode);
    void findConsumedPaths(std::vector<REG_node*>* currentSet, char currentChar, REG_node* currentNode);
    std::vector<TokenREG*> regular_expressions;


  public:
    void ParseProgram();
};

#endif
