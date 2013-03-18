#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>

#define forall(iterator,listptr) \
  for(iterator = listptr->begin(); iterator != listptr->end(); iterator++) \

#define forallR(iterator,listptr) \
  for(iterator = listptr->rbegin(); iterator != listptr->rend(); iterator++) \

class Codegen : public Visitor
{
	private:

	FILE * m_outputfile;
	SymTab *m_st;

	// basic size of a word (integers and booleans) in bytes
	static const int wordsize = 4;

	int label_count; //access with new_label

	// ********** Helper functions ********************************

	// this is used to get new unique labels (cleverly named label1, label2, ...)
	int new_label() { return label_count++; }

	// this mode is used for the code
	void set_text_mode() { fprintf( m_outputfile, ".text\n\n"); }
	
	// PART 1:
	// 1) get arithmetic expressions on integers working:
	//	  you wont really be able to run your code,
	//	  but you can visually inspect it to see that the correct
	//    chains of opcodes are being generated.
	// 2) get function calls working:
	//    if you want to see at least a very simple program compile
	//    and link successfully against gcc-produced code, you
	//    need to get at least this far
	// 3) get boolean operation working
	//    before we can implement any of the conditional control flow 
	//    stuff, we need to have booleans worked out.  
	// 4) control flow:
	//    we need a way to have if-elses and while loops in our language. 
	//
	// Hint: Symbols have an associated member variable called m_offset
	//    That offset can be used to figure out where in the activation 
	//    record you should look for a particuar variable

	///////////////////////////////////////////////////////////////////////////////
	//
	//  function_prologue
	//  function_epilogue
	//
	//  Together these two functions implement the callee-side of the calling
	//  convention.  A stack frame has the following layout:
	//
	//                          <- SP (before pre-call / after post-ret)
	//  high -----------------
	//       | actual arg n  |
	//       | ...           |
	//       | actual arg 1  |  <- SP (just before call / just after ret)
	//       -----------------
	//       |  Return Addr  |  <- SP (just after call / just before ret)
	//       =================
	//       | previous %ebp |
	//       -----------------
	//       | temporary 1   |
	//       | ...           |
	//       | temporary n   |  <- SP (after prologue / before epilogue)
	//  low  -----------------
	//
	//
	//			  ||		
	//			  ||
	//			 \  /
	//			  \/
	//
	//
	//  The caller is responsible for placing the actual arguments
	//  and the return address on the stack. Actually, the return address
	//  is put automatically on the stack as part of the x86 call instruction.
	//
	//  On function entry, the callee
	//
	//  (1) allocates space for the callee's temporaries on the stack
	//  
	//  (2) saves callee-saved registers (see below) - including the previous activation record pointer (%ebp)
	//
	//  (3) makes the activation record pointer (frame pointer - %ebp) point to the start of the temporary region
	//
	//  (4) possibly copies the actual arguments into the temporary variables to allow easier access
	//
	//  On function exit, the callee:
	//
	//  (1) pops the callee's activation record (temporary area) off the stack
	//  
	//  (2) restores the callee-saved registers, including the activation record of the caller (%ebp)	 
	//
	//  (3) jumps to the return address (using the x86 "ret" instruction, this automatically pops the 
	//	  return address of the stack. After the ret, remove the arguments from the stack
	//
	//	For more info on this convention, see http://unixwiz.net/techtips/win32-callconv-asm.html
	//
	//	This convention is called __cdecl
	//
	//////////////////////////////////////////////////////////////////////////////
  
  void emit_prologue(SymName *name, unsigned int size_locals, unsigned int num_args)
  {
    fprintf(m_outputfile, "\n.globl\t%s\n\n", name->spelling());
    fprintf(m_outputfile, "%s:\n", name->spelling());
    fprintf(m_outputfile, "\tpushl\t%%ebp\n");
    fprintf(m_outputfile, "\tmovl\t%%esp, %%ebp\n");
    

  }

  void emit_epilogue()
  {
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\tmovl\t%%ebx, %%ebp\n");
    fprintf(m_outputfile, "\tret\n");
  }
  
  // HERE: more functions to emit code

////////////////////////////////////////////////////////////////////////////////

public:
  
  Codegen(FILE * outputfile, SymTab * st)
  {
	m_outputfile = outputfile;
	m_st = st;
	
	label_count = 0;
  }
  
  void visitProgram(Program * p)
  {
    set_text_mode();
    p->visit_children(this);
  }
  void visitFunc(Func * p)
  {
    emit_prologue(p->m_symname, 0, (int)p->m_param_list->size());

    // if(p->m_function_block->m_return->m_expr->m_attribute.m_lattice_elem == BOTTOM) {
    //   fprintf(m_outputfile, "\t## TOP!! ##\n");
    // }
    if(p->m_function_block->m_return->m_expr->m_attribute.m_lattice_elem != TOP &&
      p->m_function_block->m_return->m_expr->m_attribute.m_lattice_elem != BOTTOM)
    {
      // fprintf(m_outputfile, "\t## %d ##\n", p->m_function_block->m_return->m_expr->m_attribute.m_lattice_elem.value);

      fprintf(m_outputfile, "\tpushl\t$%d\n", 
        p->m_function_block->m_return->m_expr->m_attribute.m_lattice_elem.value);
      fprintf(m_outputfile, "\tpopl\t%%eax\n");
    } else {
      for (int i = 1; i <= (int)p->m_param_list->size(); i++) {
        int offset = wordsize + (i*wordsize);
        fprintf(m_outputfile, "\tpushl\t%d(%%ebp)\n", offset);      
      }
      p->m_function_block->accept(this);
    }

    emit_epilogue();
  }
  void visitFunction_block(Function_block * p)
  {
    int num_vars = 0;
    
    list<Decl_ptr>::iterator m_decl_list_iter;
    forall(m_decl_list_iter, p->m_decl_list) {
      if (((Decl *)(*m_decl_list_iter))->m_attribute.m_basetype != bt_intarray)
        num_vars += ((Decl *)(*m_decl_list_iter))->m_symname_list->size();
      else
        num_vars += ((Decl *)(*m_decl_list_iter))->m_symname_list->size() * ((TIntArray *)((Decl *)(*m_decl_list_iter))->m_type)->m_primitive->m_data;
    }

    fprintf(m_outputfile, "\tsubl\t$%d, %%esp\n", num_vars*wordsize); 

    list<Func_ptr>::iterator m_fun_list_iter;
    forall(m_fun_list_iter, p->m_func_list) {
        (*m_fun_list_iter)->accept(this);
    }

    list<Stat_ptr>::iterator m_stat_list_iter;
    forall(m_stat_list_iter, p->m_stat_list) {
      (*m_stat_list_iter)->accept(this);
    }
    
    p->m_return->accept(this);
    fprintf(m_outputfile, "\tmovl\t%%ebp, %%esp\n");
  }
  void visitNested_block(Nested_block * p)
  {
    p->visit_children(this);
  }
  void visitAssignment(Assignment * p)
  {
    if(p->m_expr->m_attribute.m_lattice_elem != TOP &&
      p->m_expr->m_attribute.m_lattice_elem != BOTTOM) {
      // fprintf(m_outputfile, "\t## %d ##\n", p->m_expr->m_attribute.m_lattice_elem.value);
      fprintf(m_outputfile, "\tpushl\t$%d\n", p->m_expr->m_attribute.m_lattice_elem.value);
    } else {
      p->visit_children(this);
    }
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname->spelling());
    fprintf(m_outputfile, "\tmovl\t%%eax, %d(%%ebp)\n", -(wordsize+s->get_offset()) ); 
  }
  void visitArrayAssignment(ArrayAssignment * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname->spelling());
    fprintf(m_outputfile, "\timull\t$4, %%ebx\n");
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", s->get_offset());
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", wordsize);
    fprintf(m_outputfile, "\tneg\t%%ebx\n");
    fprintf(m_outputfile, "\taddl\t%%ebp, %%ebx\n");
    fprintf(m_outputfile, "\tmovl\t%%eax, (%%ebx)\n");
  }
  void visitCall(Call * p)
  {
    list<Expr_ptr>::reverse_iterator m_expr_list_iter;
    forallR(m_expr_list_iter, p->m_expr_list) {
      (*m_expr_list_iter)->accept(this);
    }

    fprintf(m_outputfile, "\tcall\t%s\n", p->m_symname_2->spelling());
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname_1->spelling());
    fprintf(m_outputfile, "\tmovl\t%%eax, %d(%%ebx)\n", -(wordsize+s->get_offset()) );  
  }
  void visitArrayCall(ArrayCall *p)
  {
    list<Expr_ptr>::reverse_iterator m_expr_list_iter;
    forallR(m_expr_list_iter, p->m_expr_list_2) {
      (*m_expr_list_iter)->accept(this);
    }

    fprintf(m_outputfile, "\tcall\t%s\n", p->m_symname_2->spelling());
    p->m_expr_1->accept(this);
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\timull\t$4, %%ebx\n");
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname_1->spelling());
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", s->get_offset());
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", wordsize);
    fprintf(m_outputfile, "\tneg\t%%ebx\n");
    fprintf(m_outputfile, "\taddl\t%%ebp, %%ebx\n");
    fprintf(m_outputfile, "\tmovl\t%%eax, (%%ebx)\n");
  }
  void visitReturn(Return * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
  }

  // control flow
  void visitIfNoElse(IfNoElse * p)
  {
    if(p->m_expr->m_attribute.m_lattice_elem.value == 0) {
      //skip if
      // fprintf(m_outputfile, "\t## %d ##\n", p->m_expr->m_attribute.m_lattice_elem.value);
    } else {
      p->m_expr->accept(this);
      int label = new_label();
      fprintf(m_outputfile, "\tpopl\t%%ecx\n");
      fprintf(m_outputfile, "\tmovl $0, %%ebx\n");
      fprintf(m_outputfile, "\tcmp %%eax, %%ebx\n");
      fprintf(m_outputfile, "\tje D%d\n", label);
      p->m_nested_block->accept(this);
      fprintf(m_outputfile, "D%d:", label);
    }
  }
  void visitIfWithElse(IfWithElse * p)
  {
    if(p->m_expr->m_attribute.m_lattice_elem.value == 1) {
      //do if
      p->m_nested_block_1->accept(this);
    } else if (p->m_expr->m_attribute.m_lattice_elem.value == 0){
      //do else
      p->m_nested_block_2->accept(this);
    } else {
      p->m_expr->accept(this);
      int label_else = new_label();
      int label_done = new_label();
      fprintf(m_outputfile, "\tpopl\t%%ecx\n");
      fprintf(m_outputfile, "\tmovl $0, %%ebx\n");
      fprintf(m_outputfile, "\tcmp %%eax, %%ebx\n");
      fprintf(m_outputfile, "\tje E%d\n", label_else);
      p->m_nested_block_1->accept(this);
      fprintf(m_outputfile, "\tjmp\tD%d\n", label_done);
      fprintf(m_outputfile, "E%d:", label_else);
      p->m_nested_block_2->accept(this);
      fprintf(m_outputfile, "D%d:", label_done);
    }
  }
  void visitWhileLoop(WhileLoop * p)
  {
    if(p->m_expr->m_attribute.m_lattice_elem.value == 0) {
      //skip while
      // p->m_nested_block_1->accept(this);
    } else {
      int label_loop = new_label();
      int label_done = new_label();
      fprintf(m_outputfile, "L%d:", label_loop);
      p->m_expr->accept(this);
      fprintf(m_outputfile, "\tpopl\t%%ecx\n");
      fprintf(m_outputfile, "\tmovl $0, %%ebx\n");
      fprintf(m_outputfile, "\tcmp %%eax, %%ebx\n");
      fprintf(m_outputfile, "\tje D%d\n", label_done);
      p->m_nested_block->accept(this);
      fprintf(m_outputfile, "\tjmp\tL%d\n", label_loop);    
      fprintf(m_outputfile, "D%d:", label_done);
    }
  }

  // variable declarations (no code generation needed)
  void visitDecl(Decl * p)
  {
    p->visit_children(this);
  }
  void visitParam(Param *p)
  {
    p->visit_children(this);
  }
  void visitTInt(TInt * p)
  {
    p->visit_children(this);
  }
  void visitTBool(TBool * p)
  {
    p->visit_children(this);
  }
  void visitTIntArray(TIntArray * p)
  {
    p->visit_children(this);
  }

  // comparison operations
  void visitCompare(Compare * p)
  {
    p->visit_children(this);
    int label_true = new_label();
    int label_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tcmp\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tje\tT%d\n", label_true);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tD%d\n", label_done);
    fprintf(m_outputfile, "T%d:", label_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "D%d:", label_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitNoteq(Noteq * p)
  {
    p->visit_children(this);
    int label_true = new_label();
    int label_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tcmp\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjne\tT%d\n", label_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tD%d\n", label_done);
    fprintf(m_outputfile, "T%d:", label_true);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "D%d:", label_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitGt(Gt * p)
  {
    p->visit_children(this);
    int label_true = new_label();
    int label_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tcmp\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjg\tT%d\n", label_true);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tD%d\n", label_done);
    fprintf(m_outputfile, "T%d:", label_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "D%d:", label_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitGteq(Gteq * p)
  {
    p->visit_children(this);
    int label_true = new_label();
    int label_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tcmp\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjge\tT%d\n", label_true);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tD%d\n", label_done);
    fprintf(m_outputfile, "T%d:", label_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "D%d:", label_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitLt(Lt * p)
  {
    p->visit_children(this);
    int label_true = new_label();
    int label_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tcmp\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjl\tT%d\n", label_true);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tD%d\n", label_done);
    fprintf(m_outputfile, "T%d:", label_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "D%d:", label_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitLteq(Lteq * p)
  {
    p->visit_children(this);
    int label_true = new_label();
    int label_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tcmp\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjle\tT%d\n", label_true);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tD%d\n", label_done);
    fprintf(m_outputfile, "T%d:", label_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "D%d:", label_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }


  // arithmetic and logic operations
  void visitAnd(And * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\tandl\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitOr(Or * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\torl\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitMinus(Minus * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\tsubl\t%%eax, %%ebx\n");
    fprintf(m_outputfile, "\tpushl\t%%ebx\n");
  }
  void visitPlus(Plus * p)
  {
    // if(p->m_expr->m_attribute.m_lattice_elem != TOP &&
    //   p->m_expr->m_attribute.m_lattice_elem != BOTTOM)
    // {
    //   fprintf(m_outputfile, "\t## %d ##\n", p->m_expr->m_attribute.m_lattice_elem.value);

    // }
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\taddl\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitTimes(Times * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\timull\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitDiv(Div * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\timull\t%%ebx\n"); //hack from le Internet
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tidivl\t%%ebx\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitNot(Not * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\txor\t$1, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitUminus(Uminus * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\timull\t$-1, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitMagnitude(Magnitude * p)
  {
    p->visit_children(this);    // fabs not working?
    int label = new_label();    // abs(x) = (x XOR y) - y where y = x >>> 31
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tmovl\t%%eax, %%ebx\n");
    fprintf(m_outputfile, "\tsarl\t$31, %%ebx\n");
    fprintf(m_outputfile, "\tmovl\t%%ebx, %%ecx\n");
    fprintf(m_outputfile, "\txorl\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tmovl\t%%ecx, %%eax\n");
    fprintf(m_outputfile, "\tsubl\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }

  // variable and constant access
  void visitIdent(Ident * p)
  {
    p->visit_children(this);
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname->spelling());
    fprintf(m_outputfile, "\tpushl\t%d(%%ebp)\n", -(wordsize+s->get_offset()));
  }
  void visitIntLit(IntLit * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpushl\t$%d\n", p->m_primitive->m_data);
  }
  void visitBoolLit(BoolLit * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpushl\t$%d\n", p->m_primitive->m_data);
  }
  void visitArrayAccess(ArrayAccess * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname->spelling());
    fprintf(m_outputfile, "\timull\t$4, %%ebx\n");
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", s->get_offset());
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", wordsize);
    fprintf(m_outputfile, "\tneg\t%%ebx\n");
    fprintf(m_outputfile, "\taddl\t%%ebp, %%ebx\n");
    fprintf(m_outputfile, "\tpushl\t(%%ebx)\n");
  }

  // special cases
  void visitSymName(SymName * p)
  {
  }
  void visitPrimitive(Primitive * p)
  {
  }
};

