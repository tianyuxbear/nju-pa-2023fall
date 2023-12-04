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

#include "sdb.h"

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    memset(wp_pool[i].expression, 0, sizeof(wp_pool[i].expression));
    wp_pool[i].value = 0;
    wp_pool[i].status = WP_FREE;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char* str){
   if(free_ == NULL) return NULL;

   WP* result = free_;
   free_ = free_->next;

   result->next = NULL;
   memcpy(result->expression, str, strlen(str));
   result->status = WP_BUSY;
   bool success = true;
   result->value = expr(str, &success);
   if(success == false){
    Log("watch point expression make token error ==> %s", str);
    assert(0);
   }
   
   int i = 0;
   WP* ptr = head;
   while(ptr != NULL){
    i++;
    ptr = ptr->next;
   }
   result->NO = i + 1;

   WP* pre = head;
   if(i == 0){
     head = result;
   }else{
     int j = i - 1;
     while(j != 0){
      pre = pre->next;
      j--;
     }
     pre->next = result;
   }

   return result;
}

void free_wp(int num){
    assert(head != NULL);
    if(num == 1){
      WP* ptr = head;
      memset(ptr->expression, 0, sizeof(ptr->expression));
      ptr->value = 0;
      ptr->status = WP_FREE;
      ptr->next = free_;
      free_ = ptr;

      head = NULL;
    }else{
      int i = num - 2;
      WP* pre = head;
      while(i != 0) pre = pre->next;
      WP* ptr = pre->next;
      pre->next = ptr->next;

      memset(ptr->expression, 0, sizeof(ptr->expression));
      ptr->status = WP_FREE;
      ptr->next = free_;
      free_ = ptr;
    }
}

void watchpoint_display(){
    WP* ptr = head;
    if(ptr == NULL){
      printf("No watchpoints\n");
      return;
    }
    printf("expression: %s\n", ptr->expression);
    printf("%-8s%-15s%s\n", "Num", "Type", "What");
    while(ptr != NULL){
        printf("%-8d%-15s%s\n", ptr->NO, "watchpoint", ptr->expression);
        ptr = ptr->next; 
    }
}

void scan_watchpoint(){
    WP* ptr = head;
    bool flag = false;
    while(ptr != NULL)
    {   
        bool success = true;
        word_t value = expr(ptr->expression, &success);
        if(success == false){
          Log("watch point expression make token error ==> %s", ptr->expression);
          continue;
        }
        if(value != ptr->value){
          if(strstr(ptr->expression, "$pc") != NULL){
            Log("Hit NO.%d watchpoint == expression: %s  value: " FMT_WORD " ==> " FMT_WORD , 
              ptr->NO, ptr->expression, ptr->value, value);
          }else{
            Log("Hit NO.%d watchpoint == expression: %s  value: %lu ==> %lu", 
              ptr->NO, ptr->expression, ptr->value, value);
          }
          ptr->value = value;
          flag = true;
        }
        ptr = ptr->next;
    }
    if(flag == true){
      nemu_state.state = NEMU_STOP;
    }
}

