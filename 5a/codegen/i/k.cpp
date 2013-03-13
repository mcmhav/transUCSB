/*********************************************************************
*  Author:     Jesse Farless
*  Class:      CS 160 -- Prof. Kruegel -- F08
*  Assignment: Project 5 (Code Generation) -- codegen.cpp
*  Due:        December 7th, 2008 @ 11:59:59pm
*  
*********************************************************************/

#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>


class Codegen : public Visitor
{
    private:

    FILE * m_outputfile;
    SymTab *m_st;

    // basic size of a word (integers and booleans) in bytes
    static const int wordsize = 4;

    int label_count; //access with new_label

    int new_label() { return label_count++; }

    void set_text_mode() { fprintf( m_outputfile, ".text\n\n"); }
    void set_data_mode() { fprintf( m_outputfile, ".data\n\n"); }

public:
  
  Codegen(FILE * outputfile, SymTab * st)
  {
    m_outputfile = outputfile;
    m_st = st;
    
    label_count = 0;
  }
  
  void visitProgramImpl(ProgramImpl * p)
  {
    set_text_mode();
    fprintf(m_outputfile, "\t.globl\tMain\n");
    p->visit_children(this);
  }
  void visitProcImpl(ProcImpl * p)
  {
    int skip_label = new_label();
    fprintf(m_outputfile, "\tjmp\tskip_nested_procedure%d\n", skip_label);
    fprintf(m_outputfile, "\t ## Proc ##\n");
    fprintf(m_outputfile, "%s:\n", p->m_symname->spelling());
    fprintf(m_outputfile, "\tpushl\t%%ebp\n");
    fprintf(m_outputfile, "\tmovl\t%%esp, %%ebp\n");
    int num_args = 0;
    list<Decl_ptr>::iterator m_decl_list_iter;
    for(m_decl_list_iter = p->m_decl_list->begin();
      m_decl_list_iter != p->m_decl_list->end();
      ++m_decl_list_iter)
    {
      num_args += ((Decl *)(*m_decl_list_iter))->m_symname_list->size();
    }
    for (int i = 1; i <= num_args; i++)
    {
      int offset = 4 + (i*4);
      fprintf(m_outputfile, "\tpushl\t%d(%%ebp)\n", offset);      
    }
    p->m_function_block->accept(this);
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\tmovl\t%%ebx, %%ebp\n");
    fprintf(m_outputfile, "\tret\n");
    fprintf(m_outputfile, "\t ## END Proc ##\n");
    fprintf(m_outputfile, "skip_nested_procedure%d:\n", skip_label);
  }
  void visitProcedure_blockImpl(Procedure_blockImpl * p)
  {
    fprintf(m_outputfile, "\t ## Procedure_block ##\n");
    int num_vars = 0;
    list<Decl_ptr>::iterator m_decl_list_iter;
    for(m_decl_list_iter = p->m_decl_list->begin();
      m_decl_list_iter != p->m_decl_list->end();
      ++m_decl_list_iter)
    {
      if (((Decl *)(*m_decl_list_iter))->m_attribute.m_basetype != bt_intarray)
    num_vars += ((Decl *)(*m_decl_list_iter))->m_symname_list->size();
      else
    num_vars += ((Decl *)(*m_decl_list_iter))->m_symname_list->size() * ((TIntArray *)((Decl *)(*m_decl_list_iter))->m_type)->m_primitive->m_data;
    }
    fprintf(m_outputfile, "\tsubl\t$%d, %%esp\n", num_vars*4); 

    list<Proc_ptr>::iterator m_func_list_iter;
    for(m_func_list_iter = p->m_func_list->begin();
      m_func_list_iter != p->m_func_list->end();
      ++m_func_list_iter){
        (*m_func_list_iter)->accept( this );
    }

    list<Stat_ptr>::iterator m_stat_list_iter;
    for(m_stat_list_iter = p->m_stat_list->begin();
      m_stat_list_iter != p->m_stat_list->end();
      ++m_stat_list_iter)
    {
      (*m_stat_list_iter)->accept( this );
    }
    
    p->m_return->accept(this);
    fprintf(m_outputfile, "\tmovl\t%%ebp, %%esp\n");
    fprintf(m_outputfile, "\t ## END Procedure_block ##\n");
  }
  void visitNested_blockImpl(Nested_blockImpl * p)
  {
    fprintf(m_outputfile, "\t ## Nested_block ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\t ## END Nested_block ##\n");
  }
  void visitAssignment(Assignment * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname->spelling());
    fprintf(m_outputfile, "\tmovl\t%%eax, %d(%%ebp)\n", -(4+s->get_offset()) );    
  }
  void visitArrayAssignment(ArrayAssignment * p)
  {
    fprintf(m_outputfile, "\t ## ArrayAssignment ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname->spelling());
    fprintf(m_outputfile, "\timull\t$4, %%ebx\n");
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", s->get_offset());
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", 4);
    fprintf(m_outputfile, "\tneg\t%%ebx\n");
    fprintf(m_outputfile, "\taddl\t%%ebp, %%ebx\n");
    fprintf(m_outputfile, "\tmovl\t%%eax, (%%ebx)\n");
    fprintf(m_outputfile, "\t ## END ArrayAssignment ##\n");
  }
  void visitCall(Call * p)
  {
    int num_arguments = 0;
    list<Expr_ptr>::reverse_iterator m_expr_list_iter;
    for(m_expr_list_iter = p->m_expr_list->rbegin();
      m_expr_list_iter != p->m_expr_list->rend();
      ++m_expr_list_iter)
    {
      num_arguments++;
      (*m_expr_list_iter)->accept( this );
    }
    fprintf(m_outputfile, "\tcall\t%s\n", p->m_symname_2->spelling());
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname_1->spelling());
    fprintf(m_outputfile, "\tmovl\t%%eax, %d(%%ebx)\n", -(4+s->get_offset()) );    
  }
  void visitArrayCall(ArrayCall *p)
  {
    fprintf(m_outputfile, "\t ## ArrayCall ##\n");
    int num_arguments = 0;
    list<Expr_ptr>::reverse_iterator m_expr_list_iter;
    for(m_expr_list_iter = p->m_expr_list_2->rbegin();
      m_expr_list_iter != p->m_expr_list_2->rend();
      ++m_expr_list_iter)
    {
      num_arguments++;
      (*m_expr_list_iter)->accept( this );
    }
    fprintf(m_outputfile, "\tcall\t%s\n", p->m_symname_2->spelling());
    p->m_expr_1->accept(this);
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\timull\t$4, %%ebx\n");
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname_1->spelling());
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", s->get_offset());
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", 4);
    fprintf(m_outputfile, "\tneg\t%%ebx\n");
    fprintf(m_outputfile, "\taddl\t%%ebp, %%ebx\n");
    fprintf(m_outputfile, "\tmovl\t%%eax, (%%ebx)\n");
    fprintf(m_outputfile, "\t ## END ArrayCall ##\n");
  }
  void visitReturn(Return * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
  }

  // control flow
  void visitIfNoElse(IfNoElse * p)
  {
    fprintf(m_outputfile, "\t ## IfNoElse ##\n");
    p->m_expr->accept(this);
    int label_number_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tDone_%d\n", label_number_done);
    p->m_nested_block->accept(this);
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\t ## END IfNoElse ##\n");
  }
  void visitIfWithElse(IfWithElse * p)
  {
    fprintf(m_outputfile, "\t ## IfWithElse ##\n");
    p->m_expr->accept(this);
    int label_number_else = new_label();
    int label_number_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tElse_%d\n", label_number_else);
    p->m_nested_block_1->accept(this);
    fprintf(m_outputfile, "\tjmp\tDone_%d\n", label_number_done);
    fprintf(m_outputfile, "Else_%d:\n", label_number_else);
    p->m_nested_block_2->accept(this);
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\t ## END IfWithElse ##\n");
  }
  void visitWhileLoop(WhileLoop * p)
  {
    fprintf(m_outputfile, "\t ## WhileLoop ##\n");
    int label_number_loop = new_label();
    int label_number_done = new_label();
    fprintf(m_outputfile, "Loop_%d:\n", label_number_loop);
    p->m_expr->accept(this);
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tDone_%d\n", label_number_done);
    p->m_nested_block->accept(this);
    fprintf(m_outputfile, "\tjmp\tLoop_%d\n", label_number_loop);    
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\t ## END WhileLoop ##\n");
  }

  // variable declarations (no code generation needed)
  void visitDecl(Decl * p)
  {
    p->visit_children(this);
  }
  void visitTInteger(TInteger * p)
  {
    p->visit_children(this);
  }
  void visitTBoolean(TBoolean * p)
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
    int label_number_true = new_label();
    int label_number_done = new_label();
    fprintf(m_outputfile, "\t ## Compare ##\n");
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tsubl\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tTrue_%d\n", label_number_true);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tDone_%d\n", label_number_done);
    fprintf(m_outputfile, "True_%d:\n", label_number_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitNoteq(Noteq * p)
  {
    p->visit_children(this);
    int label_number_false = new_label();
    int label_number_done = new_label();
    fprintf(m_outputfile, "\t ## Noteq ##\n");
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tsubl\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tFalse_%d\n", label_number_false);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tDone_%d\n", label_number_done);
    fprintf(m_outputfile, "False_%d:\n", label_number_false);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitGt(Gt * p)
  {
    fprintf(m_outputfile, "\t ## Gt ##\n");
    p->visit_children(this);
    int label_number_true = new_label();
    int label_number_false = new_label();
    int label_number_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tsubl\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tFalse_%d\n", label_number_false);
    fprintf(m_outputfile, "\tshr\t$31, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tTrue_%d\n", label_number_true);
    fprintf(m_outputfile, "False_%d:\n", label_number_false);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tDone_%d\n", label_number_done);
    fprintf(m_outputfile, "True_%d:\n", label_number_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END Gt ##\n");
  }
  void visitGteq(Gteq * p)
  {
    fprintf(m_outputfile, "\t ## Gteq ##\n");
    p->visit_children(this);
    int label_number_true = new_label();
    int label_number_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tsubl\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tTrue_%d\n", label_number_true);
    fprintf(m_outputfile, "\tshr\t$31, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tTrue_%d\n", label_number_true);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tDone_%d\n", label_number_done);
    fprintf(m_outputfile, "True_%d:\n", label_number_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END Gteq ##\n");
  }
  void visitLt(Lt * p)
  {
    fprintf(m_outputfile, "\t ## Lt ##\n");
    p->visit_children(this);
    int label_number_true = new_label();
    int label_number_false = new_label();
    int label_number_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tsubl\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tFalse_%d\n", label_number_false);
    fprintf(m_outputfile, "\tshr\t$31, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tTrue_%d\n", label_number_true);
    fprintf(m_outputfile, "False_%d:\n", label_number_false);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tDone_%d\n", label_number_done);
    fprintf(m_outputfile, "True_%d:\n", label_number_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END Lt ##\n");
  }
  void visitLteq(Lteq * p)
  {
    fprintf(m_outputfile, "\t ## Lteq ##\n");
    p->visit_children(this);
    int label_number_true = new_label();
    int label_number_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tsubl\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tTrue_%d\n", label_number_true);
    fprintf(m_outputfile, "\tshr\t$31, %%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tTrue_%d\n", label_number_true);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tDone_%d\n", label_number_done);
    fprintf(m_outputfile, "True_%d:\n", label_number_true);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END Lteq ##\n");
  }

  // arithmetic and logic operations
  void visitAnd(And * p)
  {
    fprintf(m_outputfile, "\t ## And ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\tandl\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END And ##\n");
  }
  void visitOr(Or * p)
  {
    p->visit_children(this);
    fprintf(m_outputfile, "\t ## Or ##\n");
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\torl\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
  }
  void visitMinus(Minus * p)
  {
    fprintf(m_outputfile, "\t ## Minus ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\tsubl\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END Minus ##\n");
  }
  void visitPlus(Plus * p)
  {
    fprintf(m_outputfile, "\t ## Plus ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\taddl\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END Plus ##\n");
  }
  void visitTimes(Times * p)
  {
    fprintf(m_outputfile, "\t ## Times ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\timull\t%%ebx, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## NOT Times ##\n");
  }
  void visitDiv(Div * p)
  {
    fprintf(m_outputfile, "\t ## Div ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "\tmovl\t$1, %%ebx\n");
    fprintf(m_outputfile, "\timull\t%%ebx\n");
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tidivl\t%%ebx\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## NOT Div ##\n");
  }
  void visitNot(Not * p)
  {
    fprintf(m_outputfile, "\t ## Not ##\n");
    p->visit_children(this);
    int label_number_set_to_one = new_label();
    int label_number_done = new_label();
    fprintf(m_outputfile, "\tpopl\t%%ecx\n");
    fprintf(m_outputfile, "\tjecxz\tSet_To_One_%d\n", label_number_set_to_one);
    fprintf(m_outputfile, "\tmovl\t$0, %%eax\n");
    fprintf(m_outputfile, "\tjmp\tDone_%d\n", label_number_done);
    fprintf(m_outputfile, "Set_To_One_%d:\n", label_number_set_to_one);
    fprintf(m_outputfile, "\tmovl\t$1, %%eax\n");
    fprintf(m_outputfile, "Done_%d:\n", label_number_done);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END Not ##\n");
  }
  void visitUminus(Uminus * p)
  {
    fprintf(m_outputfile, "\t ## UMinus ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\timull\t$-1, %%eax\n");
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END UMinus ##\n");
  }
  void visitMagnitude(Magnitude * p)
  {
    fprintf(m_outputfile, "\t ## Magnitude ##\n");
    p->visit_children(this);
    int label_number = new_label();
    fprintf(m_outputfile, "\tpopl\t%%eax\n");
    fprintf(m_outputfile, "\tmovl\t%%eax, %%ecx\n");
    fprintf(m_outputfile, "\tshr\t$31, %%ecx\n"); //if %eax was negative, then after the shift, %ecx should be 1, otherwise it will be 0.
    fprintf(m_outputfile, "\tjecxz\tSkip_Negation_%d\n", label_number);
    fprintf(m_outputfile, "\timull\t$-1, %%eax\n");
    fprintf(m_outputfile, "Skip_Negation_%d:\n", label_number);
    fprintf(m_outputfile, "\tpushl\t%%eax\n");
    fprintf(m_outputfile, "\t ## END Magnitude ##\n");
  }

  // variable and constant access
  void visitIdent(Ident * p)
  {
    fprintf(m_outputfile, "\t ## Ident ##\n");
    p->visit_children(this);
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname->spelling());
    fprintf(m_outputfile, "\tpushl\t%d(%%ebp)\n", -(4+s->get_offset()));
    fprintf(m_outputfile, "\t ## END Ident ##\n");
  }
  void visitIntLit(IntLit * p)
  {
    fprintf(m_outputfile, "\t ## IntLit ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tpushl\t$%d\n", p->m_primitive->m_data);
    fprintf(m_outputfile, "\t ## END IntLit ##\n");
  }
  void visitBoolLit(BoolLit * p)
  {
    fprintf(m_outputfile, "\t ## BoolLit ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tpushl\t$%d\n", p->m_primitive->m_data);
    fprintf(m_outputfile, "\t ## END BoolLit ##\n");
  }
  void visitArrayAccess(ArrayAccess * p)
  {
    fprintf(m_outputfile, "\t ## ArrayAccess ##\n");
    p->visit_children(this);
    fprintf(m_outputfile, "\tpopl\t%%ebx\n");
    Symbol *s = m_st->lookup(p->m_attribute.m_scope, p->m_symname->spelling());
    fprintf(m_outputfile, "\timull\t$4, %%ebx\n");
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", s->get_offset());
    fprintf(m_outputfile, "\taddl\t$%d, %%ebx\n", 4);
    fprintf(m_outputfile, "\tneg\t%%ebx\n");
    fprintf(m_outputfile, "\taddl\t%%ebp, %%ebx\n");
    fprintf(m_outputfile, "\tpushl\t(%%ebx)\n");
    fprintf(m_outputfile, "\t ## END ArrayAccess ##\n");
  }

  // special cases
  void visitSymName(SymName * p) {}
  void visitPrimitive(Primitive * p) {}
  
};

void
dopass_codegen(Program_ptr ast, SymTab * st)
{
    Codegen *codegen = new Codegen(stdout, st); //create the visitor
    ast->accept(codegen);                           //walk the tree with the visitor above
    delete codegen;
}


