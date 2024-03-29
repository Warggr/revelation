#define PRODUCE_JSON // produce JSON
#define CODE(x) //ignore code blocks

#define NB_AGENTS 2

// The function FIN() represents a final, non-expanded token. By defining it as...
// - FIN(x)        the file should expand to nothing
// - FIN(x) x      the file should expand to valid JSON
// - FIN(x) STR(x) the file should expand to a valid C string representing JSON
#ifndef FIN
#define FIN(x) STR(x)
#define FINCOMMA "," // FINCOMMA works around the fact that FIN(,) is invalid
#endif

#define COMMA_FUNCTION() FINCOMMA
#define UNRESOLVED_COMMA COMMA_FUNCTION
#define IGNORE_NEXT_PARENS()

#define OPTION(option, body) option FINCOMMA
#define LITERAL(x) FIN([) FIN(x) FIN(])

#define GETSYMBOL_B(a, b) PARENS GETSYMBOL(b, _)
#define SELECT_BY_TYPE(...) FIN([) IGNORE_NEXT_PARENS MAKE_PAIRS(GETSYMBOL_B, UNRESOLVED_COMMA, __VA_ARGS__) FIN(])
#define MAKE_FIRST_LITERAL(a, b) PARENS LITERAL(a)
#define SELECT_LITERAL(...) FIN([) IGNORE_NEXT_PARENS MAKE_PAIRS(MAKE_FIRST_LITERAL, UNRESOLVED_COMMA, __VA_ARGS__) FIN(])
#define CHILD(name, body) PARENS FIN(name) FIN(:) body
#define OBJECT(...) FIN([{) IGNORE_NEXT_PARENS MAKE_PAIRS( CHILD, UNRESOLVED_COMMA, __VA_ARGS__ ) FIN(}])

#define AS_ARRAY(arrsize, foreach) FIN([[) foreach FINCOMMA FIN(arrsize) FIN(]])

#define GETSYMBOL(symbol, c_value) FIN(#symbol)

#define HAS_TYPE(typename) FIN([{"type":) LITERAL(typename) FIN(}])

#define CLASS(requires, allows) FIN({) requires FINCOMMA allows FIN(})
#define CLASS_REQUIRES(...) FIN("required":{) IGNORE_NEXT_PARENS MAKE_PAIRS( CHILD, UNRESOLVED_COMMA, __VA_ARGS__ ) FIN(})
#define CLASS_ALLOWS(...) FIN("optional":{) IGNORE_NEXT_PARENS MAKE_PAIRS( CHILD, UNRESOLVED_COMMA, __VA_ARGS__ ) FIN(})
#define CLASS_REQUIRES_NONE FIN("required":{})
#define CLASS_ALLOWS_NONE FIN("optional":{})

#define SYMBOL(symbol, c_type, body) FIN(#symbol) FIN(:) body FINCOMMA

#ifndef PURE_JSON
#include <string_view>
#endif

extern const std::string_view grammar_as_json_string =
#include "grammar.hpp"
;
