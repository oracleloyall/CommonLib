/*
 * list.hpp
 *
 *  Created on: 2015年7月23日
 *      Author: sw
 */

#ifndef SRC_LIST_HPP_
#define SRC_LIST_HPP_

typedef struct NODE
{
	struct NODE* next;
	char data[0];
}Node_t;
typedef struct NODE * pNode_t;

typedef struct LIST
{
	pNode_t head;
	pNode_t tail;
	pNode_t current;
	pNode_t tra;
	int	size;
}List_t;
typedef struct LIST* pList_t;

class ListBuf
{
private:
	pNode_t head;
	pNode_t tail;
	pNode_t tra;
	int	size;
public:
	pNode_t current;
	void* iptor;
	void InitList();
	int GetListSize();
	void AddNode(const void* data,int len);
	void ClearList();
	void* GetFirstNode();
	void* GetNextNode();
	int InsertNode(int index,const void* data);
	void* GetByIndex(int index);
	ListBuf();
	~ListBuf();
};

void InitList(pList_t list);
int GetListSize(pList_t list);
void AddNode(pList_t list,const void* data,int len);
void ClearList(pList_t list);
void* GetFirstNode(pList_t list);
void* GetNextNode(pList_t list);
int InsertNode(pList_t list,int index,const void* data);

#endif /* SRC_LIST_HPP_ */
