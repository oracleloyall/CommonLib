/*
 * list.cpp
 *
 *  Created on: 2015年7月23日
 *      Author: sw
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.hpp"

void InitList(pList_t list)
{
 list->current = NULL;
 list->head = NULL;
 list->tail = NULL;
 list->size = 0;
}

int GetListSize(pList_t list)
{
	return list->size;
}

void AddNode(pList_t list,const void* node,int len)
{
	pNode_t temp = (pNode_t)malloc(sizeof(Node_t) + len);
	if(!temp)
	{
		return;
	}
	temp->next = NULL;
	memcpy(temp->data,node,len);
	pNode_t pnode = (pNode_t)list->current;
	if(!pnode)
	{
		list->head = temp;
		list->tail = temp;
		list->size = 1;
	}else
	{
		list->tail = temp;
		list->size ++;
		pnode->next = temp;
	}
}

void ClearList(pList_t list)
{
	pNode_t node = (pNode_t)list->head;
	int i = 1;
	while(node)
	{
		pNode_t temp =  (pNode_t)node->next;;
		free((void*)node);
		node = temp;
		printf("clear:%d\n",i++);
	}
	list->current = NULL;
	list->tail = NULL;
	list->head = NULL;
	list->size = 0;
}

void* GetFirstNode(pList_t list)
{
	if(list->head)
	{
		list->tra = list->head->next;
		return list->head->data;
	}
	else
		return NULL;
}

void* GetNextNode(pList_t list)
{
	pNode_t node = list->tra;
	if(node)
	{
		list->tra = node->next;
		return node->data;
	}else
	{
		return NULL;
	}
}

void ListBuf::InitList()
{
	current = NULL;
	head = NULL;
	tail = NULL;
	size = 0;
}
int ListBuf::GetListSize()
{
	return size;
}
void ListBuf::AddNode(const void* node,int len)
{
	int listsize = sizeof(Node_t) + len + 1;
	pNode_t temp = (pNode_t)malloc(listsize);
	if(!temp)
		return;
	memset(temp,0,listsize);
	temp->next = NULL;
	memcpy(temp->data,node,len);
	pNode_t pnode = tail;
	if(!pnode)
	{
		head = temp;
		tail = temp;
		size = 1;
	}else
	{
		tail = temp;
		size ++;
		pnode->next = temp;
	}
	printf("AddNode:size=%d\n",size);
}
void ListBuf::ClearList()
{
	pNode_t node = head;
	while(node)
	{
		pNode_t temp =  (pNode_t)node->next;;
		free((void*)node);
		node = temp;
	}
	current = NULL;
	tail = NULL;
	head = NULL;
	size = 0;
}
void* ListBuf::GetFirstNode()
{
	if(head)
	{
		tra = head->next;
		return head->data;
	}
	else
		return NULL;
}
void* ListBuf::GetNextNode()
{
	pNode_t node = tra;
	if(node)
	{
		tra = node->next;
		return node->data;
	}else
	{
		return NULL;
	}
}
int ListBuf::InsertNode(int index,const void* data)
{
	return 0;
}
void* ListBuf::GetByIndex(int index)
{
	int i = 1;
	for(current = head;current;current = current->next,i++)
	{
		if(i == index)
			return current->data;
	}
	return NULL;
}

ListBuf::ListBuf()
{
	InitList();
}
ListBuf::~ListBuf()
{
	ClearList();
}
