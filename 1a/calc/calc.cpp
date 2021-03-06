#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stack>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;


/*** Enums and Print Functions for Terminals and Non-Terminals  **********************/

#define MAX_SYMBOL_NAME_SIZE 25
#define MAX_NUMBER 2147483647

//all of the terminals in the language
typedef enum {
	T_eof = 0,		// 0: end of file
	T_num,			// 1: numbers
	T_plus,			// 2: +
	T_minus,		// 3: -
	T_times,		// 4: *
    T_div,      	// 5: /
    T_lt,			// 6: <
	T_gt,			// 7: >
	T_eq, 			// 8: =
	T_semicolon,	// 9: ;
	T_openparen,	// 10: (
	T_closeparen 	// 11: )
} token_type;

// This function returns a string for the token. It is used in the parsetree_t
// class to actually dump the parsetree to a dot file (which can then be turned
// into a picture).  Note that the return char* is a reference to a local copy
// and it needs to be duplicated if you are a going require multiple instances
// simultaneously.
//
// No need to do anything here
char* token_to_string(token_type c) {
	static char buffer[MAX_SYMBOL_NAME_SIZE];
	switch( c ) {
		case T_eof: strncpy(buffer,"eof",MAX_SYMBOL_NAME_SIZE); break;
		case T_num: strncpy(buffer,"num",MAX_SYMBOL_NAME_SIZE); break;
		case T_plus: strncpy(buffer,"+",MAX_SYMBOL_NAME_SIZE); break;
		case T_minus: strncpy(buffer,"-",MAX_SYMBOL_NAME_SIZE); break;
		case T_times: strncpy(buffer,"*",MAX_SYMBOL_NAME_SIZE); break;
		case T_div: strncpy(buffer,"/",MAX_SYMBOL_NAME_SIZE); break;
		case T_lt: strncpy(buffer,"<",MAX_SYMBOL_NAME_SIZE); break;
		case T_gt: strncpy(buffer,">",MAX_SYMBOL_NAME_SIZE); break;
		case T_eq: strncpy(buffer,"=",MAX_SYMBOL_NAME_SIZE); break;
		case T_semicolon: strncpy(buffer,";",MAX_SYMBOL_NAME_SIZE); break;
		case T_openparen: strncpy(buffer,"(",MAX_SYMBOL_NAME_SIZE); break;
		case T_closeparen: strncpy(buffer,")",MAX_SYMBOL_NAME_SIZE); break;
		default: strncpy(buffer,"unknown_token",MAX_SYMBOL_NAME_SIZE); break;
	}
	return buffer;
}

// All of the non-terminals in the grammar. You need to add these in
// according to your grammar. These are used for printing the thing out, so
// please follow the convention we set up so that we can tell what the hack
// you are doing when I grade.
typedef enum {
	epsilon = 100,
	NT_List,
	NT_Expr,
	NT_ExprP,
	NT_Term,
	NT_TermP,
	NT_Rel,
	NT_RelP,
	NT_Fact
	// WRITEME: add symbolic names for your non-terminals here
} nonterm_type;

// This function returns a string for the non-terminals. It is used in the
// parsetree_t class to actually dump the parsetree to a dot file (which can
// then be turned into a picture). Note that the return char* is a reference
// to a local copy and it needs to be duplicated if you are a going require
// multiple instances simultaneously.
//
// Add entries for all the non-terminals of your grammar following the same
// convention.
char* nonterm_to_string(nonterm_type nt)
{
	static char buffer[MAX_SYMBOL_NAME_SIZE];
	switch( nt ) {
		  case epsilon: strncpy(buffer,"e",MAX_SYMBOL_NAME_SIZE); break;
		  case NT_List: strncpy(buffer,"List",MAX_SYMBOL_NAME_SIZE); break;
		  case NT_Expr: strncpy(buffer,"Expression",MAX_SYMBOL_NAME_SIZE); break;
		  case NT_ExprP: strncpy(buffer,"ExpressionPrime",MAX_SYMBOL_NAME_SIZE); break;
		  case NT_Term: strncpy(buffer,"Term",MAX_SYMBOL_NAME_SIZE); break;
		  case NT_TermP: strncpy(buffer,"TermPrime",MAX_SYMBOL_NAME_SIZE); break;
		  case NT_Rel: strncpy(buffer,"Relation",MAX_SYMBOL_NAME_SIZE); break;
		  case NT_RelP: strncpy(buffer,"RelationPrime",MAX_SYMBOL_NAME_SIZE); break;
		  case NT_Fact: strncpy(buffer,"Factor",MAX_SYMBOL_NAME_SIZE); break;
		  // WRITEME: add the other nonterminals you need here
		  default: strncpy(buffer,"unknown_nonterm",MAX_SYMBOL_NAME_SIZE); break;
		}
	return buffer;
}

/*** ParseTree Class **********************************************/

// This class is used to dump the parsed tree as a dot file. As you parse,
// you should dump the results here and not worry about it. This class is
// complete and you should not have to touch anything (if everything goes
// according to plan).
//
// While you don't have to modify it, you will have to call it from your
// recursive decent parser so read about the interface below.
class parsetree_t {
  public:
	void push(token_type t);
	void push(nonterm_type nt);
	void pop();
	void drawepsilon();
	parsetree_t();
  private:
	enum stype_t{
		TERMINAL=1,
		NONTERMINAL=0,
		UNDEF=-1
	};

	struct stuple {
		nonterm_type nt;
		token_type t;
		stype_t stype;
		int uniq;
	};
	void printedge(stuple temp); //prints edge from TOS->temp
	stack<stuple> stuple_stack;
	char* stuple_to_string(const stuple& s);
	int counter;
};


// the constructer just starts by initializing a counter (used to uniquely
// name all the parse tree nodes) and by printing out the necessary dot commands
parsetree_t::parsetree_t()
{
	counter = 0;
	printf("digraph G { page=\"30,11\"; size=\"30, 11\"\n");
}

// This push function taken a non terminal and keeps it on the parsetree
// stack. The stack keeps trace of where we are in the parse tree as
// we walk it in a depth first way. You should call push when you start
// expanding a symbol, and call pop when you are done. The parsetree_t
// will keep track of everything, and draw the parse tree as you go.
//
// This particular function should be called if you are pushing a non-terminal
void parsetree_t::push(nonterm_type nt)
{
	counter ++;
	stuple temp;
	temp.nt = nt;
	temp.stype = NONTERMINAL;
	temp.uniq = counter;
	printedge( temp );
	stuple_stack.push( temp );
}

//same as above, but for terminals
void parsetree_t::push(token_type t)
{
	counter ++;
	stuple temp;
	temp.t = t;
	temp.stype = TERMINAL;
	temp.uniq = counter;
	printedge( temp );
	stuple_stack.push( temp );
}

// When you are finished parsing a symbol, pop it. That way the parsetree_t
// will know that you are now working on a higher part of the tree.
void parsetree_t::pop()
{
	if ( !stuple_stack.empty() ) {
		stuple_stack.pop();
	}

	if ( stuple_stack.empty() ) {
		printf( "};\n" );
	}
}

// Draw an epsilon on the parse tree hanging off of the top of stack
void parsetree_t::drawepsilon()
{
	push(epsilon);
	pop();
}

// This private print function is called from push. Basically it
// just prints out the command to draw an edge from the top of the stack (TOS)
// to the new symbol that was just pushed. If it happens to be a terminal
// then it makes it a snazzy blue color so you can see your program on the leaves
void parsetree_t::printedge(stuple temp)
{
	if ( temp.stype == TERMINAL ) {
		printf("\t\"%s_%d\" [label=\"%s\",style=filled,fillcolor=powderblue]\n",
		  stuple_to_string(temp),
		  temp.uniq,
		  stuple_to_string(temp));
	} else {
		printf("\t\"%s_%d\" [label=\"%s\"]\n",
		  stuple_to_string(temp),
		  temp.uniq,
		  stuple_to_string(temp));
	}

	//no edge to print if this is the first node
	if ( !stuple_stack.empty() ) {
		//print the edge
		printf( "\t\"%s_%d\" ", stuple_to_string(stuple_stack.top()), stuple_stack.top().uniq );
		printf( "-> \"%s_%d\"\n", stuple_to_string(temp), temp.uniq );
	}
}

// just a private utility for helping with the printing of the dot stuff
char* parsetree_t::stuple_to_string(const stuple& s)
{
	static char buffer[MAX_SYMBOL_NAME_SIZE];
	if ( s.stype == TERMINAL ) {
		snprintf( buffer, MAX_SYMBOL_NAME_SIZE, "%s", token_to_string(s.t) );
	} else if ( s.stype == NONTERMINAL ) {
		snprintf( buffer, MAX_SYMBOL_NAME_SIZE, "%s", nonterm_to_string(s.nt) );
	} else {
		assert(0);
	}

	return buffer;
}


/*** Scanner Class ***********************************************/

class scanner_t {
  public:

	// eats the next token and prints an error if it is not of type c
	void eat_token(token_type c);

	// peeks at the lookahead token
	token_type next_token();

	// return line number for errors
	int get_line();
	int get_sline();

	// constructor - inits g_next_token
	scanner_t();

	int get_number();

  private:
	vector<token_type> tokens;
	vector<int> lines;		// kinda stupid, but works
	int line;
	vector<int> numbers;

	void scan_error(char x);
	// error message and exit for mismatch
	void mismatch_error(token_type c);
};

token_type scanner_t::next_token()
{
	return tokens.front();
}

void scanner_t::eat_token(token_type c)
{
	if (c != next_token()) mismatch_error(c);
	else 
	{
		if (c == T_num)
		{
			tokens.erase(tokens.begin());
			numbers.erase(numbers.begin());
			lines.erase(lines.begin());
		}
		else
		{
			tokens.erase(tokens.begin());
			lines.erase(lines.begin());
		}
	}
}

scanner_t::scanner_t()
{
	char ca;
	line = 1;

	while (ca != EOF)     // loop while extraction from file is possible
	{
		ca = getchar();	

		if (isdigit(ca))
		{
			string num = "";
			while (isdigit(ca))
			{
				num += ca;
				ca = getchar();
			}
			long int temp = atol(num.c_str());		// atio forces number under MAX_NUMBER

			if((MAX_NUMBER < temp || temp < 0 || num.length() >= 11)) scan_error('O');
			else
			{
				tokens.push_back(T_num);
				numbers.push_back(atoi(num.c_str()));
				lines.push_back(line);
			}
		}

		switch (ca)
		{
			case ' ': break;
			case '+': tokens.push_back(T_plus); break;
			case '-': tokens.push_back(T_minus); break;
			case '*': tokens.push_back(T_times); break;
			case '/': tokens.push_back(T_div); break;
			case '<': tokens.push_back(T_lt); break;
			case '>': tokens.push_back(T_gt); break;
			case '=': tokens.push_back(T_eq); break;
			case ';': tokens.push_back(T_semicolon); break;
			case '(': tokens.push_back(T_openparen); break;
			case ')': tokens.push_back(T_closeparen); break;
			case EOF: tokens.push_back(T_eof); break;
			case '\n': line++; break;
			default: scan_error(ca); break;
		}

		if(ca != ' ' && ca != '\n') lines.push_back(line);
	}
	// for( std::vector<int>::const_iterator i = lines.begin(); i != lines.end(); ++i)
 //    std::cout << *i << ' ';
}

int scanner_t::get_number()
{
	return numbers.front();
}

int scanner_t::get_line()
{
	return lines.front();
}

int scanner_t::get_sline()
{
	return line;
}

void scanner_t::scan_error (char x)
{
	printf("scan error: unrecognized character '%c' -line %d\n",x, get_sline());
	exit(1);
}

void scanner_t::mismatch_error (token_type x)
{
	printf("syntax error: found %s ",token_to_string(next_token()) );
	printf("expecting %s - line %d\n", token_to_string(x), get_line());
	exit(1);
}





/*** Parser Class ***********************************************/

// Now on to the big stuff!
//
// The parser_t class handles everything. It has an instance of scanner_t
// so it can peek at tokens and eat them up. It also has access to the
// parsetree_t class so it can print out the parse tree as it figures it out.
// To make the recursive decent parser work, you will have to add some
// methods to this class. The helper functions are described below

class parser_t {
  private:
	scanner_t scanner;
	parsetree_t parsetree;
	void eat_token(token_type t);
	void syntax_error(nonterm_type);
	void div_by_zero_error();

	int List(int);
	int Expr(int);
	int ExprP(int);
	int Term(int);
	int TermP(int);
	int Rel(int);
	int RelP(int);
	int Fact();

	vector<int> values;
	int current_v;
	vector<token_type> tokens;
	token_type current_t;

	int final_v;


  public:
	void parse();
};

void parser_t::eat_token(token_type t)
{
	parsetree.push(t);
	scanner.eat_token(t);
	parsetree.pop();
}

void parser_t::syntax_error(nonterm_type nt)
{
	printf("syntax error: found %s in parsing %s - line %d\n",
		token_to_string( scanner.next_token()),
		nonterm_to_string(nt),
		scanner.get_line() );
	exit(1);
}

void parser_t::div_by_zero_error()
{
        printf("div by zero error: line %d\n", scanner.get_line() );
        exit(0);
}

void parser_t::parse()
{
	final_v = -1;
	List(final_v);
}

int parser_t::List(int v)
{
	switch( scanner.next_token() )
	{
		case T_semicolon:
			eat_token(T_semicolon);
			fprintf(stderr, "%d\n", v);
			v = -1;
			break;
		default:
			break;
	}	

	switch( scanner.next_token() )
	{
		case T_eof:
			parsetree.drawepsilon();
			return v;
		default:
			break;
	}	


	parsetree.push(NT_List);
	switch( scanner.next_token() )
	{
		case T_num:
			v = List(Rel(v));
			break;
		case T_openparen:
			v = List(Rel(v));
			break;
		default:
			syntax_error(NT_List);
			break;
	}
	parsetree.pop();

	return v;
}

int parser_t::Rel(int v)
{
	parsetree.push(NT_Rel);
	switch( scanner.next_token() )
	{
		case T_num:
			v = RelP(Expr(v));
			break;
		case T_lt:
			eat_token(T_lt);
			if (v < Expr(v)) v = 1;
			else v = 0;
			break;
		case T_gt:
			eat_token(T_gt);
			if (v > Expr(v)) v = 1;
			else v = 0;
			break;
		case T_eq:
			eat_token(T_eq);
			if (v == Expr(v)) v = 1;
			else v = 0;
			break;
		case T_openparen:
			v = RelP(Expr(v));
			break;
		default:
			syntax_error(NT_Rel);
			break;
	}
	parsetree.pop();
	return v;
}

int parser_t::RelP(int v)
{
	switch( scanner.next_token() )
	{
		case T_semicolon: return v;
		// case T_eof: parsetree.drawepsilon(); break;
		default: v = Rel(v); break;
	}
	return v;
}

int parser_t::Expr(int v)
{
	parsetree.push(NT_Expr);
	switch (scanner.next_token())
	{
		case T_num:
			v = ExprP(Term(v));
			break;
		case T_plus:
			eat_token(T_plus);
			v += Term(v);
			v = ExprP(v);
			break;
		case T_minus:
			eat_token(T_minus);
			v -= Term(v);
			v = ExprP(v);
			break;
		case T_openparen:
			eat_token(T_openparen);
			v = ExprP(v);
			eat_token(T_closeparen);
			break;
		default:
			syntax_error(NT_Expr);
			break;
	}
	parsetree.pop();
	return v;
}

int parser_t::ExprP(int v)
{
	switch (scanner.next_token())
	{
		case T_gt: case T_lt: case T_eq: case T_semicolon: case T_closeparen: return v;
		// case T_eof: parsetree.drawepsilon(); return v;
		default: v = Expr(v); break;
	}
	return v;
}

int parser_t::Term(int v)
{
	int tempd;
	parsetree.push(NT_Term);
	switch (scanner.next_token())
	{
		case T_num:
			v = TermP(Fact());
			break;
		case T_div:
			eat_token(T_div);
			tempd = Fact();
			if (tempd == 0)
			{
				div_by_zero_error();
			}
			v /= tempd;
			v = TermP(v);
			break;
		case T_times:
			eat_token(T_times);
			v *= Fact();
			v = TermP(v);
			break;
		case T_openparen:
			eat_token(T_openparen);
			v = Expr(v);
			eat_token(T_closeparen);
			break;
		default:
			syntax_error(NT_Term);
			break;
	}
	parsetree.pop();
	return v;
}

int parser_t::TermP(int v)
{
	switch (scanner.next_token())
	{
		case T_plus: case T_minus: case T_gt: case T_lt: case T_eq: case T_semicolon: case T_closeparen: return v;
		// case T_eof: parsetree.drawepsilon(); return v;
		case T_num: syntax_error(NT_Term); break;
		default: v = Term(v); break;
	}
	return v;
}

int parser_t::Fact()
{
	int v;
	parsetree.push(NT_Fact);
	switch (scanner.next_token())
	{
		case T_num:
			v = scanner.get_number();
			eat_token(T_num);
			break;
		case T_openparen:
			eat_token(T_openparen);
			v = Expr(v);
			eat_token(T_closeparen);
			break;
		default:
			syntax_error(NT_Fact);
			break;
	}
	parsetree.pop();

	return v;
}

/*** Main ***********************************************/

int main(int argc, char* argv[])
{
	if (argc > 1) {
		if (true || strcmp(argv[1], "-s") == 0) {
			scanner_t scanner;
			token_type tok = scanner.next_token();
			while(tok != T_eof){
				scanner.eat_token(tok);
				printf("%s", token_to_string(tok));
				tok = scanner.next_token();
			}
			printf("%s\n", token_to_string(tok));
		}
	}
	else {
		parser_t parser;
		parser.parse();
	}

	return 0;
}
