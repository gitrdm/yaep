/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 27 "ansic.y"

#define YYSTYPE string_t

/* Forward declaration for parser error function */
void yyerror (const char *s);

#line 78 "ansic.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "ansic.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENTIFIER = 3,                 /* IDENTIFIER  */
  YYSYMBOL_CONSTANT = 4,                   /* CONSTANT  */
  YYSYMBOL_STRING_LITERAL = 5,             /* STRING_LITERAL  */
  YYSYMBOL_SIZEOF = 6,                     /* SIZEOF  */
  YYSYMBOL_PTR_OP = 7,                     /* PTR_OP  */
  YYSYMBOL_INC_OP = 8,                     /* INC_OP  */
  YYSYMBOL_DEC_OP = 9,                     /* DEC_OP  */
  YYSYMBOL_LEFT_OP = 10,                   /* LEFT_OP  */
  YYSYMBOL_RIGHT_OP = 11,                  /* RIGHT_OP  */
  YYSYMBOL_LE_OP = 12,                     /* LE_OP  */
  YYSYMBOL_GE_OP = 13,                     /* GE_OP  */
  YYSYMBOL_EQ_OP = 14,                     /* EQ_OP  */
  YYSYMBOL_NE_OP = 15,                     /* NE_OP  */
  YYSYMBOL_AND_OP = 16,                    /* AND_OP  */
  YYSYMBOL_OR_OP = 17,                     /* OR_OP  */
  YYSYMBOL_MUL_ASSIGN = 18,                /* MUL_ASSIGN  */
  YYSYMBOL_DIV_ASSIGN = 19,                /* DIV_ASSIGN  */
  YYSYMBOL_MOD_ASSIGN = 20,                /* MOD_ASSIGN  */
  YYSYMBOL_ADD_ASSIGN = 21,                /* ADD_ASSIGN  */
  YYSYMBOL_SUB_ASSIGN = 22,                /* SUB_ASSIGN  */
  YYSYMBOL_LEFT_ASSIGN = 23,               /* LEFT_ASSIGN  */
  YYSYMBOL_RIGHT_ASSIGN = 24,              /* RIGHT_ASSIGN  */
  YYSYMBOL_AND_ASSIGN = 25,                /* AND_ASSIGN  */
  YYSYMBOL_XOR_ASSIGN = 26,                /* XOR_ASSIGN  */
  YYSYMBOL_OR_ASSIGN = 27,                 /* OR_ASSIGN  */
  YYSYMBOL_TYPE_NAME = 28,                 /* TYPE_NAME  */
  YYSYMBOL_INLINE = 29,                    /* INLINE  */
  YYSYMBOL_TYPEDEF = 30,                   /* TYPEDEF  */
  YYSYMBOL_EXTERN = 31,                    /* EXTERN  */
  YYSYMBOL_STATIC = 32,                    /* STATIC  */
  YYSYMBOL_AUTO = 33,                      /* AUTO  */
  YYSYMBOL_REGISTER = 34,                  /* REGISTER  */
  YYSYMBOL_CHAR = 35,                      /* CHAR  */
  YYSYMBOL_SHORT = 36,                     /* SHORT  */
  YYSYMBOL_INT = 37,                       /* INT  */
  YYSYMBOL_LONG = 38,                      /* LONG  */
  YYSYMBOL_SIGNED = 39,                    /* SIGNED  */
  YYSYMBOL_UNSIGNED = 40,                  /* UNSIGNED  */
  YYSYMBOL_FLOAT = 41,                     /* FLOAT  */
  YYSYMBOL_DOUBLE = 42,                    /* DOUBLE  */
  YYSYMBOL_CONST = 43,                     /* CONST  */
  YYSYMBOL_VOLATILE = 44,                  /* VOLATILE  */
  YYSYMBOL_VOID = 45,                      /* VOID  */
  YYSYMBOL_STRUCT = 46,                    /* STRUCT  */
  YYSYMBOL_UNION = 47,                     /* UNION  */
  YYSYMBOL_ENUM = 48,                      /* ENUM  */
  YYSYMBOL_ELIPSIS = 49,                   /* ELIPSIS  */
  YYSYMBOL_CASE = 50,                      /* CASE  */
  YYSYMBOL_DEFAULT = 51,                   /* DEFAULT  */
  YYSYMBOL_IF = 52,                        /* IF  */
  YYSYMBOL_ELSE = 53,                      /* ELSE  */
  YYSYMBOL_SWITCH = 54,                    /* SWITCH  */
  YYSYMBOL_WHILE = 55,                     /* WHILE  */
  YYSYMBOL_DO = 56,                        /* DO  */
  YYSYMBOL_FOR = 57,                       /* FOR  */
  YYSYMBOL_GOTO = 58,                      /* GOTO  */
  YYSYMBOL_CONTINUE = 59,                  /* CONTINUE  */
  YYSYMBOL_BREAK = 60,                     /* BREAK  */
  YYSYMBOL_RETURN = 61,                    /* RETURN  */
  YYSYMBOL_62_ = 62,                       /* '('  */
  YYSYMBOL_63_ = 63,                       /* ')'  */
  YYSYMBOL_64_ = 64,                       /* '['  */
  YYSYMBOL_65_ = 65,                       /* ']'  */
  YYSYMBOL_66_ = 66,                       /* '.'  */
  YYSYMBOL_67_ = 67,                       /* ','  */
  YYSYMBOL_68_ = 68,                       /* '&'  */
  YYSYMBOL_69_ = 69,                       /* '*'  */
  YYSYMBOL_70_ = 70,                       /* '+'  */
  YYSYMBOL_71_ = 71,                       /* '-'  */
  YYSYMBOL_72_ = 72,                       /* '~'  */
  YYSYMBOL_73_ = 73,                       /* '!'  */
  YYSYMBOL_74_ = 74,                       /* '/'  */
  YYSYMBOL_75_ = 75,                       /* '%'  */
  YYSYMBOL_76_ = 76,                       /* '<'  */
  YYSYMBOL_77_ = 77,                       /* '>'  */
  YYSYMBOL_78_ = 78,                       /* '^'  */
  YYSYMBOL_79_ = 79,                       /* '|'  */
  YYSYMBOL_80_ = 80,                       /* '?'  */
  YYSYMBOL_81_ = 81,                       /* ':'  */
  YYSYMBOL_82_ = 82,                       /* '='  */
  YYSYMBOL_83_ = 83,                       /* ';'  */
  YYSYMBOL_84_ = 84,                       /* '{'  */
  YYSYMBOL_85_ = 85,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 86,                  /* $accept  */
  YYSYMBOL_primary_expr = 87,              /* primary_expr  */
  YYSYMBOL_postfix_expr = 88,              /* postfix_expr  */
  YYSYMBOL_argument_expr_list = 89,        /* argument_expr_list  */
  YYSYMBOL_unary_expr = 90,                /* unary_expr  */
  YYSYMBOL_unary_operator = 91,            /* unary_operator  */
  YYSYMBOL_cast_expr = 92,                 /* cast_expr  */
  YYSYMBOL_multiplicative_expr = 93,       /* multiplicative_expr  */
  YYSYMBOL_additive_expr = 94,             /* additive_expr  */
  YYSYMBOL_shift_expr = 95,                /* shift_expr  */
  YYSYMBOL_relational_expr = 96,           /* relational_expr  */
  YYSYMBOL_equality_expr = 97,             /* equality_expr  */
  YYSYMBOL_and_expr = 98,                  /* and_expr  */
  YYSYMBOL_exclusive_or_expr = 99,         /* exclusive_or_expr  */
  YYSYMBOL_inclusive_or_expr = 100,        /* inclusive_or_expr  */
  YYSYMBOL_logical_and_expr = 101,         /* logical_and_expr  */
  YYSYMBOL_logical_or_expr = 102,          /* logical_or_expr  */
  YYSYMBOL_conditional_expr = 103,         /* conditional_expr  */
  YYSYMBOL_assignment_expr = 104,          /* assignment_expr  */
  YYSYMBOL_assignment_operator = 105,      /* assignment_operator  */
  YYSYMBOL_expr = 106,                     /* expr  */
  YYSYMBOL_constant_expr = 107,            /* constant_expr  */
  YYSYMBOL_declaration = 108,              /* declaration  */
  YYSYMBOL_declaration_specifiers = 109,   /* declaration_specifiers  */
  YYSYMBOL_init_declarator_list = 110,     /* init_declarator_list  */
  YYSYMBOL_init_declarator = 111,          /* init_declarator  */
  YYSYMBOL_storage_class_specifier = 112,  /* storage_class_specifier  */
  YYSYMBOL_type_specifier = 113,           /* type_specifier  */
  YYSYMBOL_struct_or_union_specifier = 114, /* struct_or_union_specifier  */
  YYSYMBOL_115_1 = 115,                    /* $@1  */
  YYSYMBOL_116_2 = 116,                    /* @2  */
  YYSYMBOL_117_3 = 117,                    /* $@3  */
  YYSYMBOL_118_4 = 118,                    /* $@4  */
  YYSYMBOL_119_5 = 119,                    /* @5  */
  YYSYMBOL_120_6 = 120,                    /* $@6  */
  YYSYMBOL_after_struct = 121,             /* after_struct  */
  YYSYMBOL_struct_or_union = 122,          /* struct_or_union  */
  YYSYMBOL_struct_declaration_list = 123,  /* struct_declaration_list  */
  YYSYMBOL_struct_declaration = 124,       /* struct_declaration  */
  YYSYMBOL_struct_declarator_list = 125,   /* struct_declarator_list  */
  YYSYMBOL_struct_declarator = 126,        /* struct_declarator  */
  YYSYMBOL_enum_specifier = 127,           /* enum_specifier  */
  YYSYMBOL_enumerator_list = 128,          /* enumerator_list  */
  YYSYMBOL_enumerator = 129,               /* enumerator  */
  YYSYMBOL_declarator = 130,               /* declarator  */
  YYSYMBOL_declarator2 = 131,              /* declarator2  */
  YYSYMBOL_pointer = 132,                  /* pointer  */
  YYSYMBOL_type_specifier_list = 133,      /* type_specifier_list  */
  YYSYMBOL_parameter_identifier_list = 134, /* parameter_identifier_list  */
  YYSYMBOL_identifier_list = 135,          /* identifier_list  */
  YYSYMBOL_parameter_type_list = 136,      /* parameter_type_list  */
  YYSYMBOL_parameter_list = 137,           /* parameter_list  */
  YYSYMBOL_parameter_declaration = 138,    /* parameter_declaration  */
  YYSYMBOL_type_name = 139,                /* type_name  */
  YYSYMBOL_abstract_declarator = 140,      /* abstract_declarator  */
  YYSYMBOL_abstract_declarator2 = 141,     /* abstract_declarator2  */
  YYSYMBOL_initializer = 142,              /* initializer  */
  YYSYMBOL_initializer_list = 143,         /* initializer_list  */
  YYSYMBOL_statement = 144,                /* statement  */
  YYSYMBOL_labeled_statement = 145,        /* labeled_statement  */
  YYSYMBOL_compound_statement = 146,       /* compound_statement  */
  YYSYMBOL_147_7 = 147,                    /* $@7  */
  YYSYMBOL_148_8 = 148,                    /* $@8  */
  YYSYMBOL_149_9 = 149,                    /* $@9  */
  YYSYMBOL_incr_level = 150,               /* incr_level  */
  YYSYMBOL_declaration_list = 151,         /* declaration_list  */
  YYSYMBOL_statement_list = 152,           /* statement_list  */
  YYSYMBOL_expression_statement = 153,     /* expression_statement  */
  YYSYMBOL_selection_statement = 154,      /* selection_statement  */
  YYSYMBOL_iteration_statement = 155,      /* iteration_statement  */
  YYSYMBOL_jump_statement = 156,           /* jump_statement  */
  YYSYMBOL_file = 157,                     /* file  */
  YYSYMBOL_external_definition = 158,      /* external_definition  */
  YYSYMBOL_function_definition = 159,      /* function_definition  */
  YYSYMBOL_function_body = 160,            /* function_body  */
  YYSYMBOL_identifier = 161                /* identifier  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  61
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1407

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  86
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  76
/* YYNRULES -- Number of rules.  */
#define YYNRULES  226
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  380

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   316


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    73,     2,     2,     2,    75,    68,     2,
      62,    63,    69,    70,    67,    71,    66,    74,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    81,    83,
      76,    82,    77,    80,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    64,     2,    65,    78,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    84,    79,    85,    72,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    50,    50,    51,    52,    53,    57,    58,    59,    60,
      61,    62,    63,    64,    68,    69,    73,    74,    75,    76,
      77,    78,    82,    83,    84,    85,    86,    87,    91,    92,
      96,    97,    98,    99,   103,   104,   105,   109,   110,   111,
     115,   116,   117,   118,   119,   123,   124,   125,   129,   130,
     134,   135,   139,   140,   144,   145,   149,   150,   154,   155,
     159,   160,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   178,   179,   183,   187,   189,   195,   196,
     197,   198,   202,   203,   207,   208,   212,   213,   214,   215,
     216,   217,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   238,   239,   240,   238,
     241,   242,   244,   241,   245,   248,   252,   253,   257,   258,
     262,   266,   267,   271,   272,   273,   277,   278,   279,   283,
     284,   288,   289,   293,   294,   298,   300,   301,   302,   303,
     304,   305,   309,   310,   311,   312,   316,   317,   321,   322,
     326,   327,   331,   332,   336,   337,   341,   342,   346,   347,
     351,   352,   353,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   369,   370,   371,   375,   376,   380,   381,   382,
     383,   384,   385,   389,   390,   391,   395,   396,   396,   397,
     397,   398,   398,   402,   406,   407,   411,   412,   416,   417,
     421,   422,   423,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   440,   441,   442,   443,   444,   448,   449,
     453,   454,   458,   459,   463,   464,   468
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENTIFIER",
  "CONSTANT", "STRING_LITERAL", "SIZEOF", "PTR_OP", "INC_OP", "DEC_OP",
  "LEFT_OP", "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP",
  "OR_OP", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN",
  "SUB_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN",
  "OR_ASSIGN", "TYPE_NAME", "INLINE", "TYPEDEF", "EXTERN", "STATIC",
  "AUTO", "REGISTER", "CHAR", "SHORT", "INT", "LONG", "SIGNED", "UNSIGNED",
  "FLOAT", "DOUBLE", "CONST", "VOLATILE", "VOID", "STRUCT", "UNION",
  "ENUM", "ELIPSIS", "CASE", "DEFAULT", "IF", "ELSE", "SWITCH", "WHILE",
  "DO", "FOR", "GOTO", "CONTINUE", "BREAK", "RETURN", "'('", "')'", "'['",
  "']'", "'.'", "','", "'&'", "'*'", "'+'", "'-'", "'~'", "'!'", "'/'",
  "'%'", "'<'", "'>'", "'^'", "'|'", "'?'", "':'", "'='", "';'", "'{'",
  "'}'", "$accept", "primary_expr", "postfix_expr", "argument_expr_list",
  "unary_expr", "unary_operator", "cast_expr", "multiplicative_expr",
  "additive_expr", "shift_expr", "relational_expr", "equality_expr",
  "and_expr", "exclusive_or_expr", "inclusive_or_expr", "logical_and_expr",
  "logical_or_expr", "conditional_expr", "assignment_expr",
  "assignment_operator", "expr", "constant_expr", "declaration",
  "declaration_specifiers", "init_declarator_list", "init_declarator",
  "storage_class_specifier", "type_specifier", "struct_or_union_specifier",
  "$@1", "@2", "$@3", "$@4", "@5", "$@6", "after_struct",
  "struct_or_union", "struct_declaration_list", "struct_declaration",
  "struct_declarator_list", "struct_declarator", "enum_specifier",
  "enumerator_list", "enumerator", "declarator", "declarator2", "pointer",
  "type_specifier_list", "parameter_identifier_list", "identifier_list",
  "parameter_type_list", "parameter_list", "parameter_declaration",
  "type_name", "abstract_declarator", "abstract_declarator2",
  "initializer", "initializer_list", "statement", "labeled_statement",
  "compound_statement", "$@7", "$@8", "$@9", "incr_level",
  "declaration_list", "statement_list", "expression_statement",
  "selection_statement", "iteration_statement", "jump_statement", "file",
  "external_definition", "function_definition", "function_body",
  "identifier", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-271)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-107)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     996,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
    -271,  -271,    11,    10,  1252,  -271,    18,  1338,  1338,  -271,
    -271,  -271,  1151,   -31,    27,   950,  -271,  -271,  -271,    17,
     -17,    56,  -271,  -271,  1252,  -271,   -38,  -271,  1130,  -271,
    -271,    17,    13,  -271,    18,  -271,  1151,  -271,   896,   183,
     -31,  -271,  -271,   -42,  -271,    62,    17,  -271,  -271,  -271,
      10,  -271,   474,  -271,    92,   110,  -271,   457,    87,  -271,
    -271,  -271,  1088,   157,   156,   163,   168,  -271,  -271,  -271,
    -271,  -271,   849,   892,   892,   680,  -271,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,   222,  -271,   904,  -271,    79,    -9,
      97,    51,   141,   172,   166,   167,   234,     1,  -271,   207,
    -271,    17,  -271,   904,   -39,  -271,   474,   368,  -271,  -271,
    -271,  -271,   190,   904,   195,   217,   221,   223,   350,   228,
      17,   208,   209,   529,  -271,  -271,   -27,  -271,  -271,  -271,
     457,   350,  -271,  -271,  -271,  -271,   212,  1042,   695,  -271,
      35,  -271,   -12,  -271,    24,  -271,  1316,   680,  -271,   904,
    -271,  -271,    42,  1217,   231,    17,  -271,  -271,   726,   904,
      17,  -271,   904,   904,   904,   904,   904,   904,   904,   904,
     904,   904,   904,   904,   904,   904,   904,   904,   904,   904,
     904,  -271,  -271,  -271,  -271,  -271,    39,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,   904,  1359,
    -271,   216,   350,   904,   904,   904,   245,   545,   218,  -271,
    -271,  -271,    50,   904,  -271,   220,   350,  -271,   224,   350,
    -271,   239,   240,  -271,   241,   -12,  1287,   766,  -271,  -271,
    -271,  -271,   244,  -271,  1180,    72,   904,  -271,  -271,    55,
    -271,   176,  -271,  -271,  -271,  -271,    79,    79,    -9,    -9,
      97,    97,    97,    97,    51,    51,   141,   172,   166,   167,
     234,     2,   312,  -271,  -271,  1359,  -271,   633,  1359,   350,
    -271,    78,    80,   118,   248,   561,    82,  -271,  -271,  -271,
    -271,   226,  -271,  -271,  -271,  -271,  -271,  -271,   250,  -271,
     249,  -271,  -271,  -271,   904,  -271,   904,  -271,  -271,   238,
    -271,   904,   100,  -271,   243,  1359,  -271,   350,   350,   350,
     904,   778,   101,   577,  -271,  -271,  -271,  -271,  -271,  -271,
    -271,     7,  -271,   904,   246,   273,  -271,  -271,   130,   350,
     133,   797,   809,   112,  -271,  -271,  -271,   350,   251,  -271,
     350,   350,   158,   350,   165,   821,  -271,  -271,  -271,  -271,
     350,  -271,   350,   350,   171,  -271,  -271,  -271,   350,  -271
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   226,   105,    91,    86,    87,    88,    89,    90,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     116,   117,     0,     0,   142,   221,     0,    78,    80,   103,
     115,   104,     0,   133,     0,     0,   218,   220,   135,     0,
     128,     0,   146,   144,   143,    76,     0,    82,    84,    79,
      81,   110,   193,   194,     0,   224,     0,   222,     0,     0,
     134,     1,   219,     0,   129,   131,     0,   136,   147,   145,
       0,    77,     0,   223,     0,   114,   186,     0,    84,   195,
     225,   139,   158,     0,   148,     0,   152,   154,   157,   150,
       3,     4,     0,     0,     0,     0,   137,    22,    23,    24,
      25,    26,    27,     6,    16,    28,     0,    30,    34,    37,
      40,    45,    48,    50,    52,    54,    56,    58,    75,     0,
       2,     0,   126,     0,     0,    83,     0,    28,    60,   172,
      85,   111,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   198,    73,     0,   196,   177,   178,
     189,   187,   179,   180,   181,   182,     2,     0,     0,   156,
     160,   159,   161,   141,     0,   140,     0,     0,    20,     0,
      17,    18,     0,   158,     0,     0,    12,    13,     0,     0,
       0,    19,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   138,   130,   132,   127,   175,     0,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    62,     0,     0,
     107,     0,     0,     0,     0,     0,     0,     0,     0,   214,
     215,   216,     0,     0,   199,     0,   191,   197,     0,     0,
     168,     0,     0,   164,     0,   162,     0,     0,   149,   151,
     153,   155,     0,     5,     0,   160,     0,    11,     8,     0,
      14,     0,    10,    31,    32,    33,    35,    36,    38,    39,
      43,    44,    41,    42,    46,    47,    49,    51,    53,    55,
      57,     0,     0,   173,    61,   112,   118,     0,     0,     0,
     185,     0,     0,     0,     0,     0,     0,   213,   217,    74,
     190,     0,   188,   183,   169,   163,   165,   170,     0,   166,
       0,    21,    29,     9,     0,     7,     0,   174,   176,     0,
     119,     0,     0,   121,   123,   108,   184,     0,     0,     0,
       0,     0,     0,     0,   192,   171,   167,    15,    59,   113,
     124,     0,   120,     0,     0,   200,   202,   203,     0,     0,
       0,     0,     0,     0,   122,   125,   109,     0,     0,   205,
       0,     0,     0,     0,     0,     0,   201,   204,   206,   207,
       0,   209,     0,     0,     0,   208,   210,   211,     0,   212
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -271,  -271,  -271,  -271,    19,  -271,   -90,    85,    70,    71,
      88,   137,   140,   136,   139,   143,   138,   -48,   -55,  -271,
     -53,   -74,    12,   102,  -271,   269,  -271,     9,  -271,  -271,
    -271,  -271,  -271,  -271,  -271,  -271,  -271,    57,  -270,  -271,
       3,  -271,   277,   225,   -22,   -28,    76,   -21,  -271,  -271,
     -56,  -271,   181,   -71,  -150,  -152,  -121,  -271,    86,  -271,
      83,  -271,  -271,  -271,  -271,   271,   200,  -271,  -271,  -271,
    -271,  -271,   316,  -271,   304,     0
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,   103,   104,   259,   127,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   128,   145,   218,
     146,   119,    53,    54,    46,    47,    27,    42,    29,   132,
     288,   344,    74,   219,   319,    51,    30,   285,   286,   322,
     323,    31,    63,    64,    32,    33,    34,    82,    83,    84,
     241,    86,    87,    88,   161,   162,   130,   206,   147,   148,
     149,   238,   235,   301,    77,    56,   151,   152,   153,   154,
     155,    35,    36,    37,    57,   120
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      38,    41,    85,    44,    48,   205,    60,   242,   245,    28,
       1,   118,    25,     1,     1,   320,   181,   129,   199,   199,
       1,     1,    40,    38,   174,   121,    38,     1,   121,    70,
       1,    58,    78,    59,    38,    38,    28,    28,     1,    65,
     233,    28,   172,   122,    28,    71,   204,    25,    78,   203,
     246,    75,   247,    68,    38,   320,   234,    28,    89,   221,
     159,   185,   186,   189,   190,    28,    65,    66,    79,    23,
      38,   129,    23,   248,   173,   118,    24,   156,   105,    24,
      23,   200,    38,   316,   244,   118,    28,    24,   321,    23,
     232,    68,   263,   264,   265,    39,   252,   157,    76,   158,
      43,    45,    26,   245,   242,   253,   282,   187,   188,   233,
     118,   168,   170,   171,   172,    55,   172,   233,   313,    67,
      69,    65,   314,   260,   283,   105,   261,   191,   192,    49,
      50,    55,    60,   298,   254,    41,   158,    26,   156,    80,
     228,   327,   105,   328,   123,   233,   173,   233,   182,   233,
     156,   156,   105,   183,   184,   193,   194,    38,   160,    28,
      38,   318,    79,   284,   249,   333,   312,   341,   233,    72,
     291,   292,   293,   310,   296,   257,   131,   105,   299,   233,
     262,   329,    68,   342,   351,   233,     1,    90,    91,    92,
     308,    93,    94,   358,  -106,   365,   360,   233,   287,   118,
     233,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     163,   370,   156,   164,   226,   233,   165,   129,   372,   175,
     176,   177,   233,   160,   378,   166,   156,   237,   233,   156,
     195,   315,   332,   233,   196,    95,   197,   340,    96,   255,
     198,    97,    98,    99,   100,   101,   102,   268,   269,   337,
     270,   271,   272,   273,   287,   324,   105,   287,   338,   355,
     266,   267,   201,   118,   220,   105,   222,   348,   350,   223,
     353,   274,   275,   224,   178,   225,   179,    38,   180,   156,
     227,   229,   230,   239,   256,   118,    68,   289,   362,   364,
     294,   297,   304,   305,   287,   300,   306,   311,   290,   302,
     330,   334,   374,   335,   336,     1,    90,    91,    92,   324,
      93,    94,   237,   339,   343,   303,   357,   156,   156,   156,
     255,   356,   276,   278,   367,   105,   277,   279,   281,   125,
     105,    38,   280,   124,   354,   325,   202,   251,   150,   156,
     236,    62,    73,     1,    90,    91,    92,   156,    93,    94,
     156,   156,   105,   156,     0,     0,     0,     0,     0,     0,
     156,     0,   156,   156,    95,   326,     0,     0,   156,     0,
      97,    98,    99,   100,   101,   102,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   126,   317,     0,     0,
     133,   134,   135,     0,   136,   137,   138,   139,   140,   141,
     142,   143,    95,   345,   346,   347,     0,     0,    97,    98,
      99,   100,   101,   102,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   144,    52,   359,     0,     0,     0,     0,
       0,     0,     0,   366,     0,     0,   368,   369,     0,   371,
     217,     0,     0,     0,     0,     0,   375,     0,   376,   377,
       1,    90,    91,    92,   379,    93,    94,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     1,    90,    91,
      92,     0,    93,    94,     0,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,     0,   133,   134,   135,
       0,   136,   137,   138,   139,   140,   141,   142,   143,    95,
       0,     0,     0,     0,     0,    97,    98,    99,   100,   101,
     102,     0,     1,    90,    91,    92,    95,    93,    94,     0,
     144,    52,    97,    98,    99,   100,   101,   102,     1,    90,
      91,    92,     0,    93,    94,     0,     0,     0,   126,     0,
       0,     0,     0,     0,     1,    90,    91,    92,     0,    93,
      94,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,    90,    91,    92,     0,    93,    94,     0,     0,     0,
       0,    95,     0,     0,     0,     0,     0,    97,    98,    99,
     100,   101,   102,     0,     0,     0,     0,    95,     0,     0,
       0,     0,   231,    97,    98,    99,   100,   101,   102,     0,
       0,     0,     0,    95,     0,     0,     0,     0,   295,    97,
      98,    99,   100,   101,   102,     0,     1,     0,     0,    95,
       0,     0,     0,     0,   331,    97,    98,    99,   100,   101,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     352,     2,     0,     0,     0,     0,     0,     0,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,     0,     1,    90,    91,    92,     0,    93,    94,
       0,     0,     0,     0,     0,    23,     0,     0,     1,    90,
      91,    92,    24,    93,    94,     0,     0,     0,     2,     0,
       0,     0,     0,     0,   321,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     1,
      90,    91,    92,     0,    93,    94,     0,     0,     0,     0,
       0,     0,    95,     0,     0,     0,     0,     0,    97,    98,
      99,   100,   101,   102,     0,     0,     0,    95,     0,     0,
     243,     0,     0,    97,    98,    99,   100,   101,   102,     1,
      90,    91,    92,     0,    93,    94,     0,     0,     0,     0,
       0,     1,    90,    91,    92,     0,    93,    94,    95,   258,
       0,     0,     0,     0,    97,    98,    99,   100,   101,   102,
       1,    90,    91,    92,     0,    93,    94,     0,     0,     0,
       0,     0,     1,    90,    91,    92,     0,    93,    94,     0,
       0,     0,     0,     0,     1,    90,    91,    92,    95,    93,
      94,   309,     0,     0,    97,    98,    99,   100,   101,   102,
      95,   349,     0,     0,     0,     0,    97,    98,    99,   100,
     101,   102,     1,    90,    91,    92,     0,    93,    94,    95,
     361,     0,     0,     0,     0,    97,    98,    99,   100,   101,
     102,    95,   363,     0,     0,     0,     0,    97,    98,    99,
     100,   101,   102,    95,   373,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,     1,    90,    91,    92,     1,
      93,    94,     0,     0,     0,     0,     0,     1,    90,    91,
      92,   167,    93,    94,     0,     0,     0,    97,    98,    99,
     100,   101,   102,     0,     2,     0,     0,     0,     0,     0,
       0,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,     0,     0,     0,     0,
      61,     0,     0,     1,   169,     0,     0,     0,     0,    81,
      97,    98,    99,   100,   101,   102,    95,     0,     0,     0,
       0,     0,    97,    98,    99,   100,   101,   102,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     1,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    23,     0,     0,     0,     0,     0,     0,    24,
       0,     0,     0,     0,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     1,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    23,     0,
       0,     0,     0,     0,     0,    24,     0,     0,     0,     0,
       2,     0,     0,     0,     0,     0,     0,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     1,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   157,   240,   158,     0,     0,     0,
       0,    24,     0,     0,     0,     0,     2,     0,     0,     0,
       0,     0,     0,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     157,     0,   158,     0,     0,     0,     0,    24,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     2,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
       0,     0,     0,     0,     0,     0,     0,     0,     2,     0,
       0,     0,    72,     0,    52,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,     0,
       0,     0,     0,     0,     0,    52,     0,     0,     0,     0,
       0,     0,   254,   240,   158,     2,     0,     0,     0,    24,
       0,     0,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   254,
       2,   158,     0,     0,     0,     0,    24,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,     0,     0,     0,     0,
       0,    24,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,     0,     0,     0,     0,
       0,     0,     0,     0,     2,     0,     0,     0,     0,     0,
     307,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,   250,     2,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,     2,     0,     0,
       0,     0,     0,     0,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22
};

static const yytype_int16 yycheck[] =
{
       0,    23,    58,    24,    26,   126,    34,   157,   160,     0,
       3,    59,     0,     3,     3,   285,   106,    72,    17,    17,
       3,     3,    22,    23,    95,    67,    26,     3,    67,    67,
       3,    62,    54,    64,    34,    35,    27,    28,     3,    39,
      67,    32,    95,    85,    35,    83,    85,    35,    70,   123,
      62,    51,    64,    44,    54,   325,    83,    48,    58,   133,
      82,    70,    71,    12,    13,    56,    66,    84,    56,    62,
      70,   126,    62,    49,    95,   123,    69,    77,    59,    69,
      62,    80,    82,    81,   158,   133,    77,    69,    81,    62,
     143,    82,   182,   183,   184,    84,   167,    62,    85,    64,
      24,    83,     0,   255,   254,    63,    67,    10,    11,    67,
     158,    92,    93,    94,   167,    32,   169,    67,    63,    63,
      44,   121,    67,   178,    85,   106,   179,    76,    77,    27,
      28,    48,   160,    83,    62,   157,    64,    35,   138,    56,
     140,    63,   123,    63,    82,    67,   167,    67,    69,    67,
     150,   151,   133,    74,    75,    14,    15,   157,    82,   150,
     160,   282,   150,   218,   164,    83,   256,    67,    67,    82,
     223,   224,   225,   247,   227,   175,    84,   158,   233,    67,
     180,    63,   173,    83,    83,    67,     3,     4,     5,     6,
     246,     8,     9,    63,    84,    83,    63,    67,   219,   247,
      67,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
      63,    63,   222,    67,   138,    67,    63,   282,    63,     7,
       8,     9,    67,   157,    63,    67,   236,   151,    67,   239,
      68,    65,   295,    67,    78,    62,    79,   321,    65,   173,
      16,    68,    69,    70,    71,    72,    73,   187,   188,   314,
     189,   190,   191,   192,   285,   287,   247,   288,   316,   343,
     185,   186,    65,   321,    84,   256,    81,   330,   331,    62,
     333,   193,   194,    62,    62,    62,    64,   287,    66,   289,
      62,    83,    83,    81,    63,   343,   287,    81,   351,   352,
      55,    83,    63,    63,   325,    85,    65,    63,   222,    85,
      62,    85,   365,    63,    65,     3,     4,     5,     6,   341,
       8,     9,   236,    85,    81,   239,    53,   327,   328,   329,
     254,    85,   195,   197,    83,   316,   196,   198,   200,    70,
     321,   341,   199,    66,   341,   288,   121,   166,    77,   349,
     150,    35,    48,     3,     4,     5,     6,   357,     8,     9,
     360,   361,   343,   363,    -1,    -1,    -1,    -1,    -1,    -1,
     370,    -1,   372,   373,    62,   289,    -1,    -1,   378,    -1,
      68,    69,    70,    71,    72,    73,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    84,    85,    -1,    -1,
      50,    51,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,   327,   328,   329,    -1,    -1,    68,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    83,    84,   349,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   357,    -1,    -1,   360,   361,    -1,   363,
      82,    -1,    -1,    -1,    -1,    -1,   370,    -1,   372,   373,
       3,     4,     5,     6,   378,     8,     9,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,    -1,     8,     9,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    51,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,
      73,    -1,     3,     4,     5,     6,    62,     8,     9,    -1,
      83,    84,    68,    69,    70,    71,    72,    73,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    84,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    62,    -1,    -1,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    62,    -1,    -1,
      -1,    -1,    83,    68,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,    83,    68,
      69,    70,    71,    72,    73,    -1,     3,    -1,    -1,    62,
      -1,    -1,    -1,    -1,    83,    68,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    -1,    62,    -1,    -1,     3,     4,
       5,     6,    69,     8,     9,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    81,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,    68,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    62,    -1,    -1,
      65,    -1,    -1,    68,    69,    70,    71,    72,    73,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    62,    63,
      -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,    73,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    62,     8,
       9,    65,    -1,    -1,    68,    69,    70,    71,    72,    73,
      62,    63,    -1,    -1,    -1,    -1,    68,    69,    70,    71,
      72,    73,     3,     4,     5,     6,    -1,     8,     9,    62,
      63,    -1,    -1,    -1,    -1,    68,    69,    70,    71,    72,
      73,    62,    63,    -1,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    73,    62,    63,    -1,    -1,    -1,    -1,    68,
      69,    70,    71,    72,    73,     3,     4,     5,     6,     3,
       8,     9,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,    62,     8,     9,    -1,    -1,    -1,    68,    69,    70,
      71,    72,    73,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    -1,    -1,    -1,    -1,
       0,    -1,    -1,     3,    62,    -1,    -1,    -1,    -1,    63,
      68,    69,    70,    71,    72,    73,    62,    -1,    -1,    -1,
      -1,    -1,    68,    69,    70,    71,    72,    73,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,     3,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,     3,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,     3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    -1,    64,    -1,    -1,    -1,    -1,    69,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    82,    -1,    84,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      -1,    -1,    -1,    -1,    -1,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    28,    -1,    -1,    -1,    69,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      28,    64,    -1,    -1,    -1,    -1,    69,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      -1,    69,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      63,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    62,    69,   108,   109,   112,   113,   114,
     122,   127,   130,   131,   132,   157,   158,   159,   161,    84,
     161,   130,   113,   132,   133,    83,   110,   111,   130,   109,
     109,   121,    84,   108,   109,   146,   151,   160,    62,    64,
     131,     0,   158,   128,   129,   161,    84,    63,   113,   132,
      67,    83,    82,   160,   118,   161,    85,   150,   130,   108,
     146,    63,   133,   134,   135,   136,   137,   138,   139,   161,
       4,     5,     6,     8,     9,    62,    65,    68,    69,    70,
      71,    72,    73,    87,    88,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   107,
     161,    67,    85,    82,   128,   111,    84,    90,   103,   104,
     142,    84,   115,    50,    51,    52,    54,    55,    56,    57,
      58,    59,    60,    61,    83,   104,   106,   144,   145,   146,
     151,   152,   153,   154,   155,   156,   161,    62,    64,   130,
     132,   140,   141,    63,    67,    63,    67,    62,    90,    62,
      90,    90,   106,   133,   139,     7,     8,     9,    62,    64,
      66,    92,    69,    74,    75,    70,    71,    10,    11,    12,
      13,    76,    77,    14,    15,    68,    78,    79,    16,    17,
      80,    65,   129,   107,    85,   142,   143,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    82,   105,   119,
      84,   107,    81,    62,    62,    62,   144,    62,   161,    83,
      83,    83,   106,    67,    83,   148,   152,   144,   147,    81,
      63,   136,   140,    65,   107,   141,    62,    64,    49,   161,
      49,   138,   139,    63,    62,   132,    63,   161,    63,    89,
     104,   106,   161,    92,    92,    92,    93,    93,    94,    94,
      95,    95,    95,    95,    96,    96,    97,    98,    99,   100,
     101,   102,    67,    85,   104,   123,   124,   133,   116,    81,
     144,   106,   106,   106,    55,    83,   106,    83,    83,   104,
      85,   149,    85,   144,    63,    63,    65,    63,   136,    65,
     107,    63,    92,    63,    67,    65,    81,    85,   142,   120,
     124,    81,   125,   126,   130,   123,   144,    63,    63,    63,
      62,    83,   106,    83,    85,    63,    65,   104,   103,    85,
     107,    67,    83,    81,   117,   144,   144,   144,   106,    63,
     106,    83,    83,   106,   126,   107,    85,    53,    63,   144,
      63,    63,   106,    63,   106,    83,   144,    83,   144,   144,
      63,   144,    63,    63,   106,   144,   144,   144,    63,   144
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    86,    87,    87,    87,    87,    88,    88,    88,    88,
      88,    88,    88,    88,    89,    89,    90,    90,    90,    90,
      90,    90,    91,    91,    91,    91,    91,    91,    92,    92,
      93,    93,    93,    93,    94,    94,    94,    95,    95,    95,
      96,    96,    96,    96,    96,    97,    97,    97,    98,    98,
      99,    99,   100,   100,   101,   101,   102,   102,   103,   103,
     104,   104,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   106,   106,   107,   108,   108,   109,   109,
     109,   109,   110,   110,   111,   111,   112,   112,   112,   112,
     112,   112,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   115,   116,   117,   114,
     118,   119,   120,   114,   114,   121,   122,   122,   123,   123,
     124,   125,   125,   126,   126,   126,   127,   127,   127,   128,
     128,   129,   129,   130,   130,   131,   131,   131,   131,   131,
     131,   131,   132,   132,   132,   132,   133,   133,   134,   134,
     135,   135,   136,   136,   137,   137,   138,   138,   139,   139,
     140,   140,   140,   141,   141,   141,   141,   141,   141,   141,
     141,   141,   142,   142,   142,   143,   143,   144,   144,   144,
     144,   144,   144,   145,   145,   145,   146,   147,   146,   148,
     146,   149,   146,   150,   151,   151,   152,   152,   153,   153,
     154,   154,   154,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   156,   156,   156,   156,   156,   157,   157,
     158,   158,   159,   159,   160,   160,   161
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     1,     4,     3,     4,
       3,     3,     2,     2,     1,     3,     1,     2,     2,     2,
       2,     4,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     3,     1,     3,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     5,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     2,     3,     1,     2,
       1,     2,     1,     3,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     0,     0,     9,
       0,     0,     0,     8,     3,     0,     1,     1,     1,     2,
       3,     1,     3,     1,     2,     3,     4,     5,     2,     1,
       3,     1,     3,     1,     2,     1,     3,     3,     4,     3,
       4,     4,     1,     2,     2,     3,     1,     2,     1,     3,
       1,     3,     1,     3,     1,     3,     2,     1,     1,     2,
       1,     1,     2,     3,     2,     3,     3,     4,     2,     3,
       3,     4,     1,     3,     4,     1,     3,     1,     1,     1,
       1,     1,     1,     3,     4,     3,     2,     0,     5,     0,
       5,     0,     6,     0,     1,     2,     1,     2,     1,     2,
       5,     7,     5,     5,     7,     6,     7,     7,     8,     7,
       8,     8,     9,     3,     2,     2,     2,     3,     1,     2,
       1,     1,     2,     3,     1,     2,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 76: /* declaration: declaration_specifiers ';'  */
#line 188 "ansic.y"
            {typedef_flag = NULL;}
#line 1720 "ansic.tab.c"
    break;

  case 77: /* declaration: declaration_specifiers init_declarator_list ';'  */
#line 190 "ansic.y"
            {typedef_flag = NULL;}
#line 1726 "ansic.tab.c"
    break;

  case 86: /* storage_class_specifier: TYPEDEF  */
#line 212 "ansic.y"
                          {typedef_flag = (string_t) 64;}
#line 1732 "ansic.tab.c"
    break;

  case 106: /* $@1: %empty  */
#line 238 "ansic.y"
                                                   {after_struct_flag = 0;}
#line 1738 "ansic.tab.c"
    break;

  case 107: /* @2: %empty  */
#line 239 "ansic.y"
                 {yyval = typedef_flag;level++;typedef_flag = NULL;}
#line 1744 "ansic.tab.c"
    break;

  case 108: /* $@3: %empty  */
#line 240 "ansic.y"
                                     {typedef_flag = yyvsp[-1];level--;}
#line 1750 "ansic.tab.c"
    break;

  case 110: /* $@4: %empty  */
#line 241 "ansic.y"
                                        {after_struct_flag = 0;}
#line 1756 "ansic.tab.c"
    break;

  case 111: /* @5: %empty  */
#line 242 "ansic.y"
                 {yyval = typedef_flag;level++;typedef_flag = NULL;}
#line 1762 "ansic.tab.c"
    break;

  case 112: /* $@6: %empty  */
#line 244 "ansic.y"
                              {typedef_flag = yyvsp[-1];level--;}
#line 1768 "ansic.tab.c"
    break;

  case 114: /* struct_or_union_specifier: struct_or_union after_struct identifier  */
#line 245 "ansic.y"
                                                   {after_struct_flag = 0;}
#line 1774 "ansic.tab.c"
    break;

  case 115: /* after_struct: %empty  */
#line 248 "ansic.y"
               {after_struct_flag = 1;}
#line 1780 "ansic.tab.c"
    break;

  case 135: /* declarator2: identifier  */
#line 298 "ansic.y"
                                      {if (typedef_flag != NULL)
                                         add_typedef ((char *) yyvsp[0], level);}
#line 1787 "ansic.tab.c"
    break;

  case 187: /* $@7: %empty  */
#line 396 "ansic.y"
                                        {level--;}
#line 1793 "ansic.tab.c"
    break;

  case 189: /* $@8: %empty  */
#line 397 "ansic.y"
                                          {level--;}
#line 1799 "ansic.tab.c"
    break;

  case 191: /* $@9: %empty  */
#line 398 "ansic.y"
                                                         {level--;}
#line 1805 "ansic.tab.c"
    break;

  case 193: /* incr_level: %empty  */
#line 402 "ansic.y"
             {level++;}
#line 1811 "ansic.tab.c"
    break;

  case 226: /* identifier: IDENTIFIER  */
#line 468 "ansic.y"
                      {yyval = yyvsp[0];}
#line 1817 "ansic.tab.c"
    break;


#line 1821 "ansic.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 470 "ansic.y"


#include <stdio.h>

extern int column;
extern int line;

/* Use an ANSI-style prototype for yyerror to avoid implicit-int and
	strict-prototype warnings from older K&R-style function definitions. */
void yyerror (const char *s)
{
	fprintf (stderr, "syntax error line - %d, column - %d\n", line, column + 1);
}
