/*
 * temp.c - functions to create and manipulate temporary variables which are
 *          used in the IR tree representation before it has been determined
 *          which variables are to go into registers.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"

struct Temp_temp_ {int num;};

int getTempNum(Temp_temp t) {
  return t->num;
}

string Temp_labelstring(Temp_label s)
{return S_name(s);
}

static int labels = 0;

Temp_label Temp_newlabel(void)
{char buf[100];
 sprintf(buf,"L%d",labels++);
 return Temp_namedlabel(String(buf));
}

/* The label will be created only if it is not found. */
Temp_label Temp_namedlabel(string s)
{return S_Symbol(s);
}

static int temps = 100;

Temp_temp Temp_newtemp(void)
{Temp_temp p = (Temp_temp) checked_malloc(sizeof (*p));
 p->num=temps++;
 {char r[16];
  sprintf(r, "%d", p->num);
  Temp_enter(Temp_name(), p, String(r));
 }
 return p;
}



struct Temp_map_ {TAB_table tab; Temp_map under;};


Temp_map Temp_name(void) {
 static Temp_map m = NULL;
 if (!m) m=Temp_empty();
 return m;
}

Temp_map newMap(TAB_table tab, Temp_map under) {
  Temp_map m = checked_malloc(sizeof(*m));
  m->tab=tab;
  m->under=under;
  return m;
}

Temp_map Temp_empty(void) {
  return newMap(TAB_empty(), NULL);
}

Temp_map Temp_layerMap(Temp_map over, Temp_map under) {
  if (over==NULL)
      return under;
  else return newMap(over->tab, Temp_layerMap(over->under, under));
}

void Temp_enter(Temp_map m, Temp_temp t, string s) {
  assert(m && m->tab);
  TAB_enter(m->tab,t,s);
}

string Temp_look(Temp_map m, Temp_temp t) {
  string s;
  assert(m && m->tab);
  s = TAB_look(m->tab, t);
  if (s) return s;
  else if (m->under) return Temp_look(m->under, t);
  else return NULL;
}

Temp_tempList Temp_TempList(Temp_temp h, Temp_tempList t) 
{Temp_tempList p = (Temp_tempList) checked_malloc(sizeof (*p));
 p->head=h; p->tail=t;
 return p;
}

Temp_labelList Temp_LabelList(Temp_label h, Temp_labelList t)
{Temp_labelList p = (Temp_labelList) checked_malloc(sizeof (*p));
 p->head=h; p->tail=t;
 return p;
}

static FILE *outfile;
void showit(Temp_temp t, string r) {
  fprintf(outfile, "t%d -> %s\n", t->num, r);
}

void Temp_dumpMap(FILE *out, Temp_map m) {
  outfile=out;
  TAB_dump(m->tab,(void (*)(void *, void*))showit);
  if (m->under) {
     fprintf(out,"---------\n");
     Temp_dumpMap(out,m->under);
  }
}



/*  My_Temp_TempList functions */

// make a empty new My_Temp_TempList
My_Temp_TempList My_Empty_Temp_TempList() {
    My_Temp_TempList list = (My_Temp_TempList)checked_malloc(sizeof *list);
    list->tail = list->head = NULL;
    list->length = 0;
    return list;
}


// TODO not tested yet
// construct My_Temp_TempList from Temp_tempList
My_Temp_TempList cloneFromTempList(Temp_tempList list) {
    My_Temp_TempList t = My_Empty_Temp_TempList();
    while(list) {
        appendMyTempList(t, list->head);
        list = list->tail;
    }
    return t;
}

// append a Temp_temp in the end of list
void appendMyTempList(My_Temp_TempList list, Temp_temp node) {
    assert(list);
    if(list->head == NULL) {
        list->head = list->tail = Temp_TempList(node, NULL);
    } else {
        list->tail->tail = Temp_TempList(node, NULL);
        list->tail = list->tail->tail;
    }
    list->length += 1;
}

// clone a My_Temp_TempList
My_Temp_TempList cloneMyTempList(My_Temp_TempList t1) {
    assert(t1);
    My_Temp_TempList list = My_Empty_Temp_TempList();
    Temp_tempList now = t1->head;
    while(now) {
        appendMyTempList(list, now->head);
        now = now->tail;
    }
    return list;
}

// find element in My_Temp_TempList, return TRUE if t exists, FALSE ow.
int findInMyTempList(My_Temp_TempList list, Temp_temp t) {
    assert(list);
    Temp_tempList now = list->head;
    while(now) {
        if(now->head == t) {
            return TRUE;
        }
        now = now->tail;
    }
    return FALSE;
}

// TODO not tested yet
// determine whether t1, t2 is equal
int isEqualMyTempList(My_Temp_TempList t1, My_Temp_TempList t2) {
    if(t1->length != t2->length) {
        return FALSE;
    }
    Temp_tempList now = t2->head;
    while(now) {
        if(findInMyTempList(t1, now->head) == FALSE) 
            return FALSE;
        now = now->tail;
    }
    return TRUE;
}

// return My_Temp_TempList equal to t1 union t2
My_Temp_TempList unionMyTempList(My_Temp_TempList t1, My_Temp_TempList t2) {
    assert(t1); assert(t2);
    My_Temp_TempList ret = cloneMyTempList(t1);
    Temp_tempList now = t2->head;
    while(now) {
        if(findInMyTempList(t1, now->head) == FALSE) {
            appendMyTempList(ret, now->head);
        }
        now = now->tail;
    }
    return ret;
}

// return My_Temp_TempList equal to t1 subtract t2
My_Temp_TempList subMyTempList(My_Temp_TempList t1, My_Temp_TempList t2) {
    assert(t1); assert(t2);
    My_Temp_TempList ret = My_Empty_Temp_TempList();
    Temp_tempList now = t1->head;
    while(now) {
        if(findInMyTempList(t2, now->head) == FALSE) {
            appendMyTempList(ret, now->head);
        }
        now = now->tail;
    }
    return ret;
}


// make a empty new Temp_labelStack
My_Temp_LabelStack My_Empty_Temp_LabelStack() {
    My_Temp_LabelStack list = (My_Temp_LabelStack)checked_malloc(sizeof *list);
    // list->tail = 
    list->head = NULL;
    list->length = 0;
    return list;  
}

// push a temp_label in front of label stack.
void pushMyTempLabelStack(My_Temp_LabelStack stack, Temp_label label) {
    assert(stack);
    if(stack->head == NULL) {
        stack->head = Temp_LabelList(label, NULL);
        // stack->head = stack->tail = Temp_LabelList(label, NULL);
    } else {
        stack->head = Temp_LabelList(label, stack->head);
    }
    stack->length += 1;
}
// pop the most front temp_label fron labelstack()
Temp_label popMyTempLabelStack(My_Temp_LabelStack stack) {
    assert(stack->length != 0);
    Temp_label label = stack->head->head;
    stack->head = stack->head->tail;
    // if(stack->head == NULL) {
        // stack->tail = NULL;
    // }
    stack->length -= 1;
    return label;
}
// get the front temp_label from label stack
Temp_label getFrontMyTempLabelStack(My_Temp_LabelStack stack) {
    return stack->head->head;
}
