#ifndef __EO_PARSER_H__
#define __EO_PARSER_H__

#include "eo_lexer.h"

Eina_Bool eo_parser_database_fill(const char *filename, Eina_Bool eot, Eolian_Class **cl);

#endif /* __EO_PARSER_H__ */