/*
 * temp.h 
 *
 */

#ifndef TEMP_H
#define TEMP_H

typedef struct Temp_temp_ *Temp_temp;
Temp_temp Temp_newtemp(void);
int getTempNum(Temp_temp);

typedef struct Temp_tempList_ *Temp_tempList;
struct Temp_tempList_ { Temp_temp head; Temp_tempList tail;};
Temp_tempList Temp_TempList(Temp_temp h, Temp_tempList t);

typedef struct My_Temp_TempList_ *My_Temp_TempList;
struct My_Temp_TempList_ {
    int length;
    Temp_tempList head;
    Temp_tempList tail;
};
// make a empty new My_Temp_TempList
My_Temp_TempList My_Empty_Temp_TempList();
// construct My_Temp_TempList from Temp_tempList
My_Temp_TempList cloneFromTempList(Temp_tempList list);
// append a Temp_temp in the end of list
void appendMyTempList(My_Temp_TempList list, Temp_temp node);
// clone a My_Temp_TempList
My_Temp_TempList cloneMyTempList(My_Temp_TempList t1);
// find element in My_Temp_TempList, return TRUE if t exists, FALSE ow.
int findInMyTempList(My_Temp_TempList list, Temp_temp t);
// determine whether t1, t2 is equal
int isEqualMyTempList(My_Temp_TempList t1, My_Temp_TempList t2);
// return My_Temp_TempList equal to t1 union t2
My_Temp_TempList unionMyTempList(My_Temp_TempList t1, My_Temp_TempList t2);
// return My_Temp_TempList equal to t1 subtract t2
My_Temp_TempList subMyTempList(My_Temp_TempList t1, My_Temp_TempList t2);


typedef S_symbol Temp_label;
Temp_label Temp_newlabel(void);
Temp_label Temp_namedlabel(string name);
string Temp_labelstring(Temp_label s);

typedef struct Temp_labelList_ *Temp_labelList;
struct Temp_labelList_ { Temp_label head; Temp_labelList tail;};
Temp_labelList Temp_LabelList(Temp_label h, Temp_labelList t);

typedef struct Temp_map_ *Temp_map;
Temp_map Temp_empty(void);
Temp_map Temp_layerMap(Temp_map over, Temp_map under);
void Temp_enter(Temp_map m, Temp_temp t, string s);
string Temp_look(Temp_map m, Temp_temp t);
void Temp_dumpMap(FILE *out, Temp_map m);

Temp_map Temp_name(void);


typedef struct My_Temp_LabelStack_ *My_Temp_LabelStack;
struct My_Temp_LabelStack_ {
    int length;
    Temp_labelList head;
    // Temp_labelList tail;
};

// make a empty new Temp_labelStack
My_Temp_LabelStack My_Empty_Temp_LabelStack();
// push a temp_label in front of label stack.
void pushMyTempLabelStack(My_Temp_LabelStack, Temp_label);
// pop the most front temp_label fron labelstack()
Temp_label popMyTempLabelStack(My_Temp_LabelStack);
// get the front temp_label from label stack
Temp_label getFrontMyTempLabelStack(My_Temp_LabelStack);



#endif
