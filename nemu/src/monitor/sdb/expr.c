/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>

enum {
  /* TODO: Add more token types */
  TK_NOTYPE,  // 0
  TK_HNUM,    // 1
  TK_DNUM,    // 2
  TK_REG,     // 3 
  TK_LB,      // 4
  TK_RB,      // 5
  TK_ADD,     // 6
  TK_SUB,     // 7
  TK_MUL,     // 8
  TK_DIV,     // 9
  TK_EQ,      // 10
  TK_NEQ,     // 11
  TK_LAND    // 12
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"0x[0-9A-Fa-f]+", TK_HNUM},  // hexadecimal number
  {"[0-9]+u?", TK_DNUM},  // decimal number
  {"\\$[0-9a-z]{1,3}", TK_REG}, // reg name
  {"\\(", TK_LB},       // left bracket
  {"\\)", TK_RB},       // right bracket
  {"\\+", TK_ADD},      // add
  {"-", TK_SUB},        // substract
  {"\\*", TK_MUL},      // multiply
  {"/", TK_DIV},        // divide
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // not equal
  {"&&", TK_LAND}       // logical and
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[8192] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
              nr_token--;
              break;
          case TK_HNUM:
              tokens[nr_token].type = TK_HNUM;
              strncpy(tokens[nr_token].str, substr_start, substr_len);
              break;
          case TK_DNUM:
              tokens[nr_token].type = TK_DNUM;
              strncpy(tokens[nr_token].str, substr_start, substr_len);
              break;
          case TK_REG:
              tokens[nr_token].type = TK_REG;
              strncpy(tokens[nr_token].str, substr_start, substr_len);
              break;
          case TK_LB:
              tokens[nr_token].type = TK_LB;
              break;
          case TK_RB:
              tokens[nr_token].type = TK_RB;
              break;
          case TK_ADD:
              tokens[nr_token].type = TK_ADD;
              break;
          case TK_SUB:
              tokens[nr_token].type = TK_SUB;
              break;
          case TK_MUL:
              tokens[nr_token].type = TK_MUL;
              break;
          case TK_DIV:
              tokens[nr_token].type = TK_DIV;
              break;
          case TK_EQ:
              tokens[nr_token].type = TK_EQ;
              break;
          case TK_NEQ:
              tokens[nr_token].type = TK_NEQ;
              break;
          case TK_LAND:
              tokens[nr_token].type = TK_LAND;
              break;
          default: assert(0);
        }

        nr_token++;

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(word_t l, word_t r){
  if(tokens[l].type != TK_LB || tokens[r].type != TK_RB){
    return false;
  }
  
  int t = -1, size = r - l + 1;
  word_t stack_t[size];
  for(int i = l ; i < r; i++){
    if(tokens[i].type == TK_LB){
      stack_t[++t] = i;
    }else if(tokens[i].type == TK_RB){
      if(t >= 0)  --t;
      else
      {
        Log("Bracket mismatch");
        assert(0);
      }
    }else{
      continue;
    }
  }
  if(t != 0){
     Log("Bracket mismatch");
     assert(0);
  }
  if(stack_t[t] != l)
    return false;

  return true;
}

enum{
  PRIO_MINUS = 0,
  PRIO_DEREF = 0,
  PRIO_MUL = 1,
  PRIO_DIV = 1,
  PRIO_ADD = 2,
  PRIO_SUB = 2,
  PRIO_EQ = 3,
  PRIO_NEQ = 3,
  PRIO_LAND = 4
};

typedef struct operator{
  word_t index;
  uint8_t priority;
} Operator;


word_t find_primary_operator(word_t l, word_t r){
  int op_count = 0, size = r - l + 1;
  Operator operators[size];
  memset(operators, 0, sizeof(operators));
  int token_lb_num = 0;
  for(int i = l; i < r + 1; i++){
     switch (tokens[i].type)
     {
     case TK_LB:
      token_lb_num++;
      break;
     case TK_RB:
      token_lb_num--;
      break;
     case TK_ADD:
      if(token_lb_num == 0){
          operators[op_count].index = i;
          operators[op_count].priority = PRIO_ADD;
          op_count++;
      }
      break;
     case TK_SUB:
      if(token_lb_num == 0){
          operators[op_count].index = i;
          if( i >= 1 && 
              ( tokens[i - 1].type == TK_RB   || 
                tokens[i - 1].type == TK_DNUM || 
                tokens[i - 1].type == TK_HNUM   )
            )
          {
            operators[op_count].priority = PRIO_SUB;
          }
          else
          {
            operators[op_count].priority = PRIO_MINUS;
          }
          op_count++;
      }
      break;
     case TK_MUL:
       if(token_lb_num == 0){
          operators[op_count].index = i;
          if( i >= 1 && 
              ( tokens[i - 1].type == TK_RB   || 
                tokens[i - 1].type == TK_DNUM || 
                tokens[i - 1].type == TK_HNUM   )
            )
          {
            operators[op_count].priority = PRIO_MUL;
          }
          else
          {
            operators[op_count].priority = PRIO_DEREF;
          }
          op_count++;
      }
      break;
     case TK_DIV:
      if(token_lb_num == 0){
          operators[op_count].index = i;
          operators[op_count].priority = PRIO_DIV;
          op_count++;
      }
      break;
     case TK_EQ:
      if(token_lb_num == 0){
          operators[op_count].index = i;
          operators[op_count].priority = PRIO_EQ;
          op_count++;
      }
      break;
     case TK_NEQ:
      if(token_lb_num == 0){
          operators[op_count].index = i;
          operators[op_count].priority = PRIO_NEQ;
          op_count++;
       }
      break;
     case TK_LAND:
      if(token_lb_num == 0){
          operators[op_count].index = i;
          operators[op_count].priority = PRIO_DIV;
          op_count++;
      }
      break;
     default:
      break;
     }
  }

  Operator result = operators[0];
  for(int i = 1; i < op_count; i++){
    if(operators[i].priority >= result.priority)
      result = operators[i];
  }

  return result.index;
}


word_t eval(word_t l, word_t r){
  if(l > r){
    Log("l > r : Bad expression l: %lu\t r: %lu", l, r);
    assert(0);
    
  }else if(l == r){
    Token token = tokens[l];
    char* endptr;
    word_t result;
    switch (token.type)
    {
    case TK_HNUM:
      result = strtoul(token.str, &endptr, 16);
      if(endptr == token.str){
        Log("convert TK_DNUM error: %s", token.str);
        assert(0);
      }
      return result;
      break;
    case TK_DNUM:
      result = strtoul(token.str, &endptr, 10);
      if(endptr == token.str){
        Log("convert TK_HNUM error: %s", token.str);
        assert(0);
      }
      return result;
      break;
    case TK_REG:
      bool success = true;
      result = isa_reg_str2val(token.str, &success);
      if(success == false){
        Log("get reg error: %s\n", token.str);
        assert(0);
      }
      return result;
      break;
    default:
      Log("Single token, neither number nor reg name");
      assert(0);
      break;
    }

  }else if(check_parentheses(l, r) == true){
    return eval(l + 1, r - 1);

  }else{
    word_t index = find_primary_operator(l, r);
    word_t result = 0;

    if(index == l){
      Token token = tokens[l];
      switch (token.type)
      {
      case TK_SUB:
        word_t val = eval(index + 1, r);
        result = -1 * val;
        return result;
        break;
      case TK_MUL:
        word_t addr = eval(index + 1, r);
        word_t result = vaddr_read(addr, 4);
        return result;
        break;
      default:
        Log("primary operator is the first. However, neither TK_SUB nor TK_MUL");
        assert(0);
        break;
      }
    }

    if(index == l + 1 && tokens[l].type == TK_SUB){
      word_t val = eval(index + 1, r);
      return val;
    }

    word_t val1 = eval(l, index - 1);
    word_t val2 = eval(index + 1, r);

    switch(tokens[index].type){
      case TK_ADD:
          result = val1 + val2;
          break;
      case TK_SUB:
          result = val1 - val2;
          break;
      case TK_MUL:
          result = val1 * val2;
          break;
      case TK_DIV:
          result = val1 / val2;
          break;
      case TK_EQ:
          result = (val1 == val2);
          break;
      case TK_NEQ:
          result = (val1 != val2);
          break;
      case TK_LAND:
          result = (val1 && val2);
          break;
      default: assert(0);
    }


    return result;
  }
}

word_t expr(char *e, bool *success) {
  memset(tokens, 0, sizeof tokens);
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  word_t l = 0, r = nr_token - 1;
  word_t result = eval(l, r);

  return result;
}
