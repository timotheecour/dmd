
// Compiler implementation of the D programming language
// Copyright (c) 1999-2012 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// http://www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#ifndef DMD_STATEMENT_H
#define DMD_STATEMENT_H

#ifdef __DMC__
#pragma once
#endif /* __DMC__ */

#include "root.h"

#include "arraytypes.h"
#include "dsymbol.h"
#include "lexer.h"
#include "visitor.h"

struct OutBuffer;
struct Scope;
class Expression;
class LabelDsymbol;
class Identifier;
class IfStatement;
class ExpStatement;
class DefaultStatement;
class VarDeclaration;
class Condition;
class Module;
struct Token;
struct InlineDoState;
class ErrorStatement;
class ReturnStatement;
class CompoundStatement;
class Parameter;
class StaticAssert;
class AsmStatement;
class GotoStatement;
class ScopeStatement;
class TryCatchStatement;
class TryFinallyStatement;
class CaseStatement;
class DefaultStatement;
class LabelStatement;
struct HdrGenState;
struct InterState;
struct CompiledCtfeFunction;

enum TOK;

// Back end
struct IRState;
struct Blockx;
#ifdef IN_GCC
typedef union tree_node block;
typedef union tree_node elem;
#else
struct block;
struct elem;
#endif
struct code;

/* How a statement exits; this is returned by blockExit()
 */
enum BE
{
    BEnone =     0,
    BEfallthru = 1,
    BEthrow =    2,
    BEreturn =   4,
    BEgoto =     8,
    BEhalt =     0x10,
    BEbreak =    0x20,
    BEcontinue = 0x40,
    BEerrthrow = 0x80,
    BEany = (BEfallthru | BEthrow | BEreturn | BEgoto | BEhalt),
};

void toCBuffer(Statement *s, OutBuffer *buf, HdrGenState *hgs);

class Statement : public RootObject
{
public:
    Loc loc;

    Statement(Loc loc);
    virtual Statement *syntaxCopy();

    void print();
    char *toChars();

    void error(const char *format, ...);
    void warning(const char *format, ...);
    void deprecation(const char *format, ...);
    virtual Statement *semantic(Scope *sc);
    Statement *semanticScope(Scope *sc, Statement *sbreak, Statement *scontinue);
    Statement *semanticNoScope(Scope *sc);
    virtual Statement *getRelatedLabeled() { return this; }
    virtual bool hasBreak();
    virtual bool hasContinue();
    bool usesEH();
    virtual int blockExit(bool mustNotThrow);
    bool comeFrom();
    bool hasCode();
    virtual Statement *scopeCode(Scope *sc, Statement **sentry, Statement **sexit, Statement **sfinally);
    virtual Statements *flatten(Scope *sc);
    virtual Expression *interpret(InterState *istate);
    virtual Statement *last();

    virtual Expression *doInline(InlineDoState *ids);
    virtual Statement *doInlineStatement(InlineDoState *ids);

    // Avoid dynamic_cast
    virtual ErrorStatement *isErrorStatement() { return NULL; }
    virtual ScopeStatement *isScopeStatement() { return NULL; }
    virtual ExpStatement *isExpStatement() { return NULL; }
    virtual CompoundStatement *isCompoundStatement() { return NULL; }
    virtual ReturnStatement *isReturnStatement() { return NULL; }
    virtual IfStatement *isIfStatement() { return NULL; }
    virtual CaseStatement *isCaseStatement() { return NULL; }
    virtual DefaultStatement *isDefaultStatement() { return NULL; }
    virtual LabelStatement *isLabelStatement() { return NULL; }
    virtual void accept(Visitor *v) { v->visit(this); }
};

/** Any Statement that fails semantic() or has a component that is an ErrorExp or
 * a TypeError should return an ErrorStatement from semantic().
 */
class ErrorStatement : public Statement
{
public:
    ErrorStatement();
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);

    ErrorStatement *isErrorStatement() { return this; }
    void accept(Visitor *v) { v->visit(this); }
};

class PeelStatement : public Statement
{
public:
    Statement *s;

    PeelStatement(Statement *s);
    Statement *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class ExpStatement : public Statement
{
public:
    Expression *exp;

    ExpStatement(Loc loc, Expression *exp);
    ExpStatement(Loc loc, Dsymbol *s);
    static ExpStatement *create(Loc loc, Expression *exp);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    int blockExit(bool mustNotThrow);
    Statement *scopeCode(Scope *sc, Statement **sentry, Statement **sexit, Statement **sfinally);

    Expression *doInline(InlineDoState *ids);
    Statement *doInlineStatement(InlineDoState *ids);

    ExpStatement *isExpStatement() { return this; }
    void accept(Visitor *v) { v->visit(this); }
};

class DtorExpStatement : public ExpStatement
{
public:
    /* Wraps an expression that is the destruction of 'var'
     */

    VarDeclaration *var;

    DtorExpStatement(Loc loc, Expression *exp, VarDeclaration *v);
    Statement *syntaxCopy();
    void accept(Visitor *v) { v->visit(this); }
};

class CompileStatement : public Statement
{
public:
    Expression *exp;

    CompileStatement(Loc loc, Expression *exp);
    Statement *syntaxCopy();
    Statements *flatten(Scope *sc);
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);
    void accept(Visitor *v) { v->visit(this); }
};

class CompoundStatement : public Statement
{
public:
    Statements *statements;

    CompoundStatement(Loc loc, Statements *s);
    CompoundStatement(Loc loc, Statement *s1);
    CompoundStatement(Loc loc, Statement *s1, Statement *s2);
    static CompoundStatement *create(Loc loc, Statement *s1, Statement *s2);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);
    Statements *flatten(Scope *sc);
    ReturnStatement *isReturnStatement();
    Expression *interpret(InterState *istate);
    Statement *last();

    Expression *doInline(InlineDoState *ids);
    Statement *doInlineStatement(InlineDoState *ids);

    CompoundStatement *isCompoundStatement() { return this; }
    void accept(Visitor *v) { v->visit(this); }
};

class CompoundDeclarationStatement : public CompoundStatement
{
public:
    CompoundDeclarationStatement(Loc loc, Statements *s);
    Statement *syntaxCopy();
    void accept(Visitor *v) { v->visit(this); }
};

/* The purpose of this is so that continue will go to the next
 * of the statements, and break will go to the end of the statements.
 */
class UnrolledLoopStatement : public Statement
{
public:
    Statements *statements;

    UnrolledLoopStatement(Loc loc, Statements *statements);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool hasBreak();
    bool hasContinue();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    Expression *doInline(InlineDoState *ids);
    Statement *doInlineStatement(InlineDoState *ids);

    void accept(Visitor *v) { v->visit(this); }
};

class ScopeStatement : public Statement
{
public:
    Statement *statement;

    ScopeStatement(Loc loc, Statement *s);
    Statement *syntaxCopy();
    ScopeStatement *isScopeStatement() { return this; }
    ReturnStatement *isReturnStatement();
    Statement *semantic(Scope *sc);
    bool hasBreak();
    bool hasContinue();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    Expression *doInline(InlineDoState *ids);
    Statement *doInlineStatement(InlineDoState *ids);

    void accept(Visitor *v) { v->visit(this); }
};

class WhileStatement : public Statement
{
public:
    Expression *condition;
    Statement *body;

    WhileStatement(Loc loc, Expression *c, Statement *b);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool hasBreak();
    bool hasContinue();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class DoStatement : public Statement
{
public:
    Statement *body;
    Expression *condition;

    DoStatement(Loc loc, Statement *b, Expression *c);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool hasBreak();
    bool hasContinue();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class ForStatement : public Statement
{
public:
    Statement *init;
    Expression *condition;
    Expression *increment;
    Statement *body;

    // When wrapped in try/finally clauses, this points to the outermost one,
    // which may have an associated label. Internal break/continue statements
    // treat that label as referring to this loop.
    Statement *relatedLabeled;

    ForStatement(Loc loc, Statement *init, Expression *condition, Expression *increment, Statement *body);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Statement *scopeCode(Scope *sc, Statement **sentry, Statement **sexit, Statement **sfinally);
    Statement *getRelatedLabeled() { return relatedLabeled ? relatedLabeled : this; }
    bool hasBreak();
    bool hasContinue();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    Statement *doInlineStatement(InlineDoState *ids);

    void accept(Visitor *v) { v->visit(this); }
};

class ForeachStatement : public Statement
{
public:
    TOK op;                // TOKforeach or TOKforeach_reverse
    Parameters *arguments;      // array of Parameter*'s
    Expression *aggr;
    Statement *body;

    VarDeclaration *key;
    VarDeclaration *value;

    FuncDeclaration *func;      // function we're lexically in

    Statements *cases;          // put breaks, continues, gotos and returns here
    ScopeStatements *gotos;     // forward referenced goto's go here

    ForeachStatement(Loc loc, TOK op, Parameters *arguments, Expression *aggr, Statement *body);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool checkForArgTypes();
    int inferAggregate(Scope *sc, Dsymbol *&sapply);
    int inferApplyArgTypes(Scope *sc, Dsymbol *&sapply);
    bool hasBreak();
    bool hasContinue();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class ForeachRangeStatement : public Statement
{
public:
    TOK op;                // TOKforeach or TOKforeach_reverse
    Parameter *arg;             // loop index variable
    Expression *lwr;
    Expression *upr;
    Statement *body;

    VarDeclaration *key;

    ForeachRangeStatement(Loc loc, TOK op, Parameter *arg,
        Expression *lwr, Expression *upr, Statement *body);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool hasBreak();
    bool hasContinue();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class IfStatement : public Statement
{
public:
    Parameter *arg;
    Expression *condition;
    Statement *ifbody;
    Statement *elsebody;

    VarDeclaration *match;      // for MatchExpression results

    IfStatement(Loc loc, Parameter *arg, Expression *condition, Statement *ifbody, Statement *elsebody);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    int blockExit(bool mustNotThrow);
    IfStatement *isIfStatement() { return this; }

    Expression *doInline(InlineDoState *ids);
    Statement *doInlineStatement(InlineDoState *ids);

    void accept(Visitor *v) { v->visit(this); }
};

class ConditionalStatement : public Statement
{
public:
    Condition *condition;
    Statement *ifbody;
    Statement *elsebody;

    ConditionalStatement(Loc loc, Condition *condition, Statement *ifbody, Statement *elsebody);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Statements *flatten(Scope *sc);
    int blockExit(bool mustNotThrow);

    void accept(Visitor *v) { v->visit(this); }
};

class PragmaStatement : public Statement
{
public:
    Identifier *ident;
    Expressions *args;          // array of Expression's
    Statement *body;

    PragmaStatement(Loc loc, Identifier *ident, Expressions *args, Statement *body);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);

    void accept(Visitor *v) { v->visit(this); }
};

class StaticAssertStatement : public Statement
{
public:
    StaticAssert *sa;

    StaticAssertStatement(StaticAssert *sa);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);

    void accept(Visitor *v) { v->visit(this); }
};

class SwitchStatement : public Statement
{
public:
    Expression *condition;
    Statement *body;
    bool isFinal;

    DefaultStatement *sdefault;
    TryFinallyStatement *tf;
    GotoCaseStatements gotoCases;  // array of unresolved GotoCaseStatement's
    CaseStatements *cases;         // array of CaseStatement's
    int hasNoDefault;           // !=0 if no default statement
    int hasVars;                // !=0 if has variable case values

    SwitchStatement(Loc loc, Expression *c, Statement *b, bool isFinal);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool hasBreak();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class CaseStatement : public Statement
{
public:
    Expression *exp;
    Statement *statement;

    int index;          // which case it is (since we sort this)
    block *cblock;      // back end: label for the block

    CaseStatement(Loc loc, Expression *exp, Statement *s);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int compare(RootObject *obj);
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);
    CaseStatement *isCaseStatement() { return this; }

    void accept(Visitor *v) { v->visit(this); }
};


class CaseRangeStatement : public Statement
{
public:
    Expression *first;
    Expression *last;
    Statement *statement;

    CaseRangeStatement(Loc loc, Expression *first, Expression *last, Statement *s);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};


class DefaultStatement : public Statement
{
public:
    Statement *statement;
#ifdef IN_GCC
    block *cblock;      // back end: label for the block
#endif

    DefaultStatement(Loc loc, Statement *s);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);
    DefaultStatement *isDefaultStatement() { return this; }

    void accept(Visitor *v) { v->visit(this); }
};

class GotoDefaultStatement : public Statement
{
public:
    SwitchStatement *sw;

    GotoDefaultStatement(Loc loc);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    int blockExit(bool mustNotThrow);

    void accept(Visitor *v) { v->visit(this); }
};

class GotoCaseStatement : public Statement
{
public:
    Expression *exp;            // NULL, or which case to goto
    CaseStatement *cs;          // case statement it resolves to

    GotoCaseStatement(Loc loc, Expression *exp);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    int blockExit(bool mustNotThrow);

    void accept(Visitor *v) { v->visit(this); }
};

class SwitchErrorStatement : public Statement
{
public:
    SwitchErrorStatement(Loc loc);
    int blockExit(bool mustNotThrow);

    void accept(Visitor *v) { v->visit(this); }
};

class ReturnStatement : public Statement
{
public:
    Expression *exp;
    bool implicit0;             // this is an implicit "return 0;"

    ReturnStatement(Loc loc, Expression *exp);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    Expression *doInline(InlineDoState *ids);
    Statement *doInlineStatement(InlineDoState *ids);

    ReturnStatement *isReturnStatement() { return this; }
    void accept(Visitor *v) { v->visit(this); }
};

class BreakStatement : public Statement
{
public:
    Identifier *ident;

    BreakStatement(Loc loc, Identifier *ident);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    int blockExit(bool mustNotThrow);

    void accept(Visitor *v) { v->visit(this); }
};

class ContinueStatement : public Statement
{
public:
    Identifier *ident;

    ContinueStatement(Loc loc, Identifier *ident);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Expression *interpret(InterState *istate);
    int blockExit(bool mustNotThrow);

    void accept(Visitor *v) { v->visit(this); }
};

class SynchronizedStatement : public Statement
{
public:
    Expression *exp;
    Statement *body;

    SynchronizedStatement(Loc loc, Expression *exp, Statement *body);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool hasBreak();
    bool hasContinue();
    int blockExit(bool mustNotThrow);

// Back end
    elem *esync;
    SynchronizedStatement(Loc loc, elem *esync, Statement *body);
    void accept(Visitor *v) { v->visit(this); }
};

class WithStatement : public Statement
{
public:
    Expression *exp;
    Statement *body;
    VarDeclaration *wthis;

    WithStatement(Loc loc, Expression *exp, Statement *body);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class TryCatchStatement : public Statement
{
public:
    Statement *body;
    Catches *catches;

    TryCatchStatement(Loc loc, Statement *body, Catches *catches);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool hasBreak();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class Catch : public RootObject
{
public:
    Loc loc;
    Type *type;
    Identifier *ident;
    VarDeclaration *var;
    Statement *handler;
    // was generated by the compiler,
    // wasn't present in source code
    bool internalCatch;

    Catch(Loc loc, Type *t, Identifier *id, Statement *handler);
    Catch *syntaxCopy();
    void semantic(Scope *sc);
    int blockExit(bool mustNotThrow);
};

class TryFinallyStatement : public Statement
{
public:
    Statement *body;
    Statement *finalbody;

    TryFinallyStatement(Loc loc, Statement *body, Statement *finalbody);
    static TryFinallyStatement *create(Loc loc, Statement *body, Statement *finalbody);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool hasBreak();
    bool hasContinue();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class OnScopeStatement : public Statement
{
public:
    TOK tok;
    Statement *statement;

    OnScopeStatement(Loc loc, TOK tok, Statement *statement);
    Statement *syntaxCopy();
    int blockExit(bool mustNotThrow);
    Statement *semantic(Scope *sc);
    Statement *scopeCode(Scope *sc, Statement **sentry, Statement **sexit, Statement **sfinally);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class ThrowStatement : public Statement
{
public:
    Expression *exp;
    // was generated by the compiler,
    // wasn't present in source code
    bool internalThrow;

    ThrowStatement(Loc loc, Expression *exp);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class DebugStatement : public Statement
{
public:
    Statement *statement;

    DebugStatement(Loc loc, Statement *statement);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Statements *flatten(Scope *sc);
    void accept(Visitor *v) { v->visit(this); }
};

class GotoStatement : public Statement
{
public:
    Identifier *ident;
    LabelDsymbol *label;
    TryFinallyStatement *tf;
    VarDeclaration *lastVar;
    FuncDeclaration *fd;

    GotoStatement(Loc loc, Identifier *ident);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    bool checkLabel();
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    void accept(Visitor *v) { v->visit(this); }
};

class LabelStatement : public Statement
{
public:
    Identifier *ident;
    Statement *statement;
    TryFinallyStatement *tf;
    Statement *gotoTarget;      // interpret
    VarDeclaration *lastVar;
    block *lblock;              // back end

    Blocks *fwdrefs;            // forward references to this LabelStatement

    LabelStatement(Loc loc, Identifier *ident, Statement *statement);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    Statements *flatten(Scope *sc);
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    LabelStatement *isLabelStatement() { return this; }

    void accept(Visitor *v) { v->visit(this); }
};

class LabelDsymbol : public Dsymbol
{
public:
    LabelStatement *statement;

    LabelDsymbol(Identifier *ident);
    static LabelDsymbol *create(Identifier *ident);
    LabelDsymbol *isLabel();
    void accept(Visitor *v) { v->visit(this); }
};

class AsmStatement : public Statement
{
public:
    Token *tokens;
    code *asmcode;
    unsigned asmalign;          // alignment of this statement
    unsigned regs;              // mask of registers modified (must match regm_t in back end)
    unsigned char refparam;     // !=0 if function parameter is referenced
    unsigned char naked;        // !=0 if function is to be naked

    AsmStatement(Loc loc, Token *tokens);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    //Expression *doInline(InlineDoState *ids);

    void accept(Visitor *v) { v->visit(this); }
};

class ImportStatement : public Statement
{
public:
    Dsymbols *imports;          // Array of Import's

    ImportStatement(Loc loc, Dsymbols *imports);
    Statement *syntaxCopy();
    Statement *semantic(Scope *sc);
    int blockExit(bool mustNotThrow);
    Expression *interpret(InterState *istate);

    Expression *doInline(InlineDoState *ids);
    Statement *doInlineStatement(InlineDoState *ids);

    void accept(Visitor *v) { v->visit(this); }
};

#endif /* DMD_STATEMENT_H */
