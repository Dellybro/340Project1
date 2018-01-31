/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"
#include <algorithm>

// using namespace std;

/*

=================================================================================
START HELPERS
syntax_error();
expect(TokenType);
peek();
=================================================================================

*/


void Parser::syntax_error(){
    cout << "Syntax Error\n";
    exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type){
    Token t = lexer.GetToken();

    if (t.token_type != expected_type)
        syntax_error();
    return t;
}
// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek(){
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

/*

=================================================================================
END HELPERS
=================================================================================

*/

/*

=================================================================================
START MATCH SEQUENCE

Description:
	the point of the match sequence area is to take the input_text and print
	each token and it's matching lexeme.
	1. To start we strip the input_text of leading and trailing white spaces, because we just don't need them
	2. Loop through the input_text one character at a time
	3. each iteration create a matches vector that holds the matches of each expression
	4. Loop through the expressions with the input_text and get the longestmatch and store in the matches vector
	5. After if the matches vector is 0, respond with ERROR
	6. Loops through matches and fine longestMatching lexeme.
	7. Increase position to the new position, and print out the token and lexeme as such
			output: t3, "lexeme"
=================================================================================

*/

void Parser::matchSequence(string input_text){
		/* REMOVE THE DAMN LEADING AND TRAILING WHITE SPACE OMG, GOING CRAZY */

		size_t startpos = input_text.find_first_not_of(" ");
    size_t endpos = input_text.find_last_not_of(" ");
    input_text = input_text.substr( startpos, endpos-startpos+1 );

    int position = 0;

    /* Loop until position is at the end of string */
    while(position <= input_text.length()-1){

      /* Create a matched vector for each matching expression */
      std::vector<Matched*> matches;

      for(int i = 0; i < regular_expressions.size(); i++){
        /* Attempt to match Regex to string starting at position */
        REG *reg = regular_expressions[i]->regularExpression;
        LongestMatched* lastMatched = match(reg, input_text, position);
        /* If matched save into matched vector */
        if(lastMatched != NULL){
          Matched* matched = new Matched();
          matched->expression = regular_expressions[i];
          matched->matched = lastMatched;
          matches.push_back(matched);
        }

      }

      /*
        Make sure there were matches
        If there weren't than the string is unparsable and syntax_error
      */
      if(matches.size() > 0){
        int longestLength = 0;
        int longestLengthIndex = 0;
        for(int i = 0; i < matches.size(); i++){
          if(matches[i]->matched->lexeme.length() > longestLength){
            longestLength = matches[i]->matched->lexeme.length();
            longestLengthIndex = i;
          }
        }

        /* Add the longestLength to position for the new position for the next iteration */
        position = matches[longestLengthIndex]->matched->position+1;

        /* Create parsedInput */
        ParsedInput *parsedInput = new ParsedInput();
        parsedInput->id = matches[longestLengthIndex]->expression->id;
        parsedInput->lexeme = matches[longestLengthIndex]->matched->lexeme;

        /* Print out the next token and lexeme */
        cout << parsedInput->id << ", \"" << parsedInput->lexeme << "\"" << endl;

      } else {
        cout << "ERROR" << endl;
				exit(1);
      }
    }


}

/*
MATCH

Descriptoin:
	Find the longest possible matching substring an return a LongestMatched Struct.
	Essentially what happens is
	1. Skip any white spaces.
	2. Create initial setVector by setting the startNode of the regex, and finding any open paths
	3. Loop through string.
	4. create the nextSet by finding consumed paths for the next char.
	5. now find all possible paths frmo those consumed paths
	6. If the accepting state is in any of the paths, put that vector into the lastMatchedSet, and update current position
	7. and continue doing this until there are no more paths or the string has been consumed.
	8. At this point that should leave us with the longestMatched.
	9. Return the longestMatched but if it doesn't exist than return a NULL string

*/

LongestMatched* Parser::match(REG* regex, std::string s, int p){
  std::vector<REG_node*> initial_set;
  REG_node *startNode = regex->starting;

  /* get next character */

  char c = s[p];
  while(isspace(c)){
    p++;
    c = s[p];
  }
  int startPosition = p;

  initial_set.push_back(startNode);
  findOpenPaths(&initial_set, startNode);

  std::vector<REG_node*> lastMatchedSet;
  int lastMatchedPosition;
  while(p <= s.length()-1){
    std::vector<REG_node*> nextSet;
    set < REG_node , less <REG_node> > :: iterator itr1;
    for (int i = 0; i< initial_set.size(); i++) {
      REG_node *node = initial_set[i];
      findConsumedPaths(&nextSet, c, node);
    }

    /* After finding all the consumed paths, we can now go to any open paths that the consumed paths have access to*/
    for (int i = 0; i< nextSet.size(); i++) {
      REG_node *node = nextSet[i];
      findOpenPaths(&nextSet, node);
    }

    if(nextSet.size() == 0){
      /* Not viable anymore */
      if(lastMatchedSet.size() > 0){
				LongestMatched *longestMatched = new LongestMatched();
				longestMatched->lexeme = s.substr(startPosition, lastMatchedPosition-startPosition+1);
				longestMatched->position = lastMatchedPosition;
        return longestMatched;
      }
      return NULL;
    }

    //
    for (int i = 0; i< nextSet.size(); i++) {
      REG_node *node = nextSet[i];
      if(node == regex->accepting){
        lastMatchedSet = nextSet;
        lastMatchedPosition = p;
      }
    }

    initial_set = nextSet;

    p++;
    c = s[p];
    if(isspace(c)){
      /* Check for accepting state on current set */
      if(lastMatchedSet.size() > 0){
				LongestMatched *longestMatched = new LongestMatched();
				longestMatched->lexeme = s.substr(startPosition, lastMatchedPosition-startPosition+1);
				longestMatched->position = lastMatchedPosition;
        return longestMatched;
      }

      return NULL;
    }
  }

  if(lastMatchedSet.size() > 0){
		LongestMatched *longestMatched = new LongestMatched();
		longestMatched->lexeme = s.substr(startPosition, lastMatchedPosition-startPosition+1);
		longestMatched->position = lastMatchedPosition;
		return longestMatched;
  }

  return NULL;
}

/*
	findOpenPaths
		- Find all paths with underscore recursively. Ensure that the node hasn't already been placed inside currentSet
*/
void Parser::findOpenPaths(std::vector<REG_node*>* currentSet, REG_node* currentNode){
  /* We add the possible routes to the currentSet */
  if(currentNode->first_label == '_'){
		if(find(currentSet->begin (), currentSet->end (), currentNode->first_neighbor) == currentSet->end()){
	    currentSet->push_back(currentNode->first_neighbor);
	    findOpenPaths(currentSet, currentNode->first_neighbor);
		}

  }
  if(currentNode->second_label == '_'){
		if(find(currentSet->begin (), currentSet->end (), currentNode->second_neighbor) == currentSet->end()){
	    currentSet->push_back(currentNode->second_neighbor);
	    findOpenPaths(currentSet, currentNode->second_neighbor);
		}
  }
}

/*
	findConsumedPaths
		- Check a nodes label and if the label is equal to the currentChar insert into currentSet
*/
void Parser::findConsumedPaths(std::vector<REG_node*>* currentSet, char currentChar, REG_node* currentNode){
  if(currentNode->first_label == currentChar){
		if(find(currentSet->begin (), currentSet->end (), currentNode->first_neighbor) == currentSet->end()){
	    currentSet->push_back(currentNode->first_neighbor);
		}
  }

  if(currentNode->second_label == currentChar){
		if(find(currentSet->begin (), currentSet->end (), currentNode->second_neighbor) == currentSet->end()){
	    currentSet->push_back(currentNode->second_neighbor);
		}
  }
}

/*

=================================================================================
END MATCH SEQUENCE
=================================================================================

*/

/*

=================================================================================
START PARSE EXPRESSION
=================================================================================

*/

REG * Parser::parse_expr() {
    // expr -> CHAR
    // expr -> LPAREN expr RPAREN DOT LPAREN expr RPAREN
    // expr -> LPAREN expr RPAREN OR LPAREN expr RPAREN
    // expr -> LPAREN expr RPAREN STAR
    // expr -> UNDERSCORE

    Token t = lexer.GetToken();
    if(t.token_type == CHAR || t.token_type == UNDERSCORE){
        /* This creates an initial REG node. This would be the starting point of any Expression */
        /* Structure #1 */
        REG_node *startingNode = new REG_node();
        REG_node *acceptingNode = new REG_node();
        /*
          Set the first label as the lexeme character
          So that when this character is seen it goes to the accepting node.
        */
        if(t.token_type == UNDERSCORE){
          startingNode->first_label = '_';
        } else {
          startingNode->first_label = *t.lexeme.c_str();
        }
        
        startingNode->first_neighbor = acceptingNode;

        REG *r1 = new REG();
        r1->starting = startingNode;
        r1->accepting = acceptingNode;

        return r1;
    }
    else if(t.token_type == LPAREN){
        // expr -> LPAREN expr RPAREN DOT LPAREN expr RPAREN
        // expr -> LPAREN expr RPAREN OR LPAREN expr RPAREN
        // expr -> LPAREN expr RPAREN STAR
        REG *r1 = parse_expr();
        expect(RPAREN);

        Token t2 = lexer.GetToken();
        if(t2.token_type == DOT || t2.token_type == OR){
          /* So now we know what r1's accepting state first_label will be */
          expect(LPAREN);
          REG *r2 = parse_expr();
          expect(RPAREN);


          REG *combinedExpression = new REG();
          REG_node *starting = new REG_node();
          REG_node *accepting = new REG_node();

          if(t2.token_type == OR){
            /* This creates the "or" */
            /* Structure #2 */

            /* New starting point will point to both r1 and r2 starting point with empty char */
            starting->first_label = '_';
            starting->second_label = '_';
            starting->first_neighbor = r1->starting;
            starting->second_neighbor = r2->starting;

            /* r1 and r2 accepting states will now point to new accepting state through empty char */
            r1->accepting->first_label = '_';
            r1->accepting->first_neighbor = accepting;
            r2->accepting->first_label = '_';
            r2->accepting->first_neighbor = accepting;

          } else if(t2.token_type == DOT){
            /* This creates the "and" */
            /* Structure #3 */

            /* Reset starting with the starting point */
            starting = r1->starting;

            /* Set the accepting label to epsilon so it can detect r2 starting*/
            r1->accepting->first_label = '_';
            r1->accepting->first_neighbor = r2->starting;

            /* Reset accepting state to r2 accepting */
            accepting = r2->accepting;
          }

          /* Return combinedExpression */

          combinedExpression->starting = starting;
          combinedExpression->accepting = accepting;
          return combinedExpression;

        }
        else if(t2.token_type == STAR){
          /* This creates the kleene star */
          /* Structure #4 */

          REG *starredExpression = new REG();
          REG_node* starting = new REG_node();
          REG_node* accepting = new REG_node();

          /* This allows for 0 */
          starting->second_label = '_';
          starting->second_neighbor = accepting;
          /* This allows for the or more part*/
          starting->first_label = '_';
          starting->first_neighbor = r1->starting;

          /* Recursively allow */
          r1->accepting->first_label = '_';
          r1->accepting->first_neighbor = r1->starting;

          /* Allows accept with new structure */
          r1->accepting->second_label = '_';
          r1->accepting->second_neighbor = accepting;

          /* Set starting and accepting for new REG and return */
          starredExpression->accepting = accepting;
          starredExpression->starting = starting;

          // expect(RPAREN);

          return starredExpression;

        } else {
          lexer.UngetToken(t2);
          return r1;
        }

    }
    else
    {
        syntax_error();
        /* This line should never hit, because syntax_error calls exit(1) */
        return NULL;
    }
}

/*

=================================================================================
END PARSE EXPRESSION
=================================================================================

*/

/*

=================================================================================
START PROGRAM FUNCTIONS
=================================================================================

*/

void Parser::parse_input() {
  //input -> tokens_section INPUT_TEXT
  parse_tokens_section();
  Token t = expect(INPUT_TEXT);

	matchSequence(t.lexeme);

}

void Parser::parse_tokens_section()
{
    // tokens_section -> token_list HASH

    parse_token_list();
    // for(int i=0; i < regular_expressions.size(); i++){
    //   runThroughNodes(regular_expressions[i]);
    // }
    expect(HASH);

}


void Parser::parse_token_list()
{
    // token_list -> token
    // token_list -> token COMMA token_list
    parse_token();

    Token t = peek();

    if (t.token_type == COMMA) {
        // token_list -> token COMMA token_list
        expect(COMMA);
        parse_token_list();
    } else if (t.token_type == HASH) {
      /* At this point all regular expressions have been created, and this function can return */
      return;
    } else {
        syntax_error();
    }

}

void Parser::parse_token()
{
    // get ID and REGEX
    Token theID = expect(ID);
    REG *regex = parse_expr();

    /* Create structure to hold ID and REGEX */
    TokenREG *token = new TokenREG();
    token->id = theID.lexeme;
    token->regularExpression = regex;

    /* Save structure to regular_expressions vector */
    regular_expressions.push_back(token);

}

void Parser::ParseProgram()
{
    parse_input();
    expect(END_OF_FILE);
}


/*

=================================================================================
END PROGRAM FUNCTIONS
=================================================================================

*/
