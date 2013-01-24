transUCSB
=========

Trans Assignment 1:

Scanner: check
The scanner is simple. Uses getchar to read values from input files, and adds the read value to the "tokens-vector" and also adds a value to the "lines-vector" to keep track of which line which token belongs too. If there is a number, the number-value is added to the "numbers-vector". For blanks and new-lines nothing is added. This is done for the complete inputfile, and when the scanner is done, we have all the values tokenized in three vectors. If there is an unknown input there will be thrown a scan_error, this is also done for values above maximum number-input.
Next_token return the first token from the "token-vector"
Eat_token checks if the requested token to eat corresponds to the actual next token, if this is all well the token in erased from the vectors. Here the line-number will also be erased and the number for T_num tokens.

Parser: check
The parser follows the setup from "LL(1) Parse Table Construction" from lecture 5. Rel is added for '=', '<' and '>', and this is kept above both the "Term" part and the "Expr" part since you would evaluate > the last in expressions like 7 + 9 > 10.
It starts in List() 
List    -> num/openparen/endof/semicolon
Rel     -> num/lt/gt/eq/openparen           -> RelP     -> semicolon
Expr    -> num/plus/minus/openparen         -> ExprP    -> gt/lt/eq/semicolon/closeparen
Term    -> num/div/times/openparen          -> TermP    -> plus/minus/gt/lt/eq/semicolon/closeparen
Fact    -> num/openparen

As we can see they all handle num and openparen in some way, since these two can appear almost anyplace in the inputs. The precedence is handled with this setup, since we would like to handle Term before for instance Expr and Rel, and it will therefor end up in the bottom of the tree. 
The openparen will start a new Expr. This is because I don't want the inputs to be (7 + 5 > 8), and the parser will not accept them, I believe this is the right way to handle an input of this kind. EOF is not accepted in any other "states" than the upper level (List). Multiple Rel's are not allowed by choice.

Evaluator: check
The evaluator is simple. It sends an integer through the whole "tree" and adds to the value when it is safe (when the terminal token is eaten. It there does the operation expected on the current value and the newly fetched one. The tree is used to make sure that precedence is preserved, so inputs like 2 - 6 * 5 will be -28 and not -20. The Relation functions returns a 0 for false and 1 for true like requested in the assignment description. For values divided by 0 there is thrown a "div_by_zero_error" error, as requested. Numbers divided are round automatically by the c++ compiler, as requested.

Conclusion:
I believe I have satisfied every request.