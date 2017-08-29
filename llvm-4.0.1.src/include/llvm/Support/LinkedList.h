
#include <iostream>
#include "DynamicAnalysis.h"



template <class T>
struct Node
{
	T val;
	Node<T> *next;
};

template <class T>
class LinkedList
{
public:
	LinkedList();
	~LinkedList();
	int findElement(T element);
	void insertAtBack(T valueToInsert);
	void print();
	bool empty();
	unsigned size();
	void clear();
	void removeFromFront();
	void removeElement(T element);
	T getFront();
	T elementAt(unsigned position);

private:
	Node<T> *first;
	Node<T> *last;
	unsigned nElements;
};


template <class T>
LinkedList<T>::LinkedList()
{
	first = NULL;
	last = NULL;
	nElements = 0;
}

template <class T>
LinkedList<T>::~LinkedList()
{
	Node<T>* temp = first;
	while(temp != NULL)
	{
		temp = temp->next;
		delete(first);
		first = temp;
	}
}


template <class T>
void LinkedList<T>::print()
{
	std::cout << "nElemets " << nElements << "\n";
}


template <class T>
T LinkedList<T>::elementAt(unsigned position){

	unsigned counter = 0;
	bool elementFound = false;
	T returnValue;
	Node<T>* temp = first;

	if (first == NULL && last == NULL)
		llvm::report_fatal_error("Cannot get an element from an empty list");

	while(elementFound == false && temp != NULL){
		if (counter == position){
			elementFound = true;
			returnValue = temp->val;
		}
		counter++;
		temp = temp->next;
	}

	return returnValue;
}


template <class T> T LinkedList<T>::getFront()
{
	return first->val;
}

template <class T>
void LinkedList<T>::insertAtBack(T valueToInsert)
{

	Node<T>* newNode = new Node<T>();

	newNode->val = valueToInsert;
	newNode->next = NULL;

	Node<T>* temp = first;

	if (temp != NULL)
	{
		while (temp->next != NULL)
		{
			temp = temp->next;
		}

		temp->next = newNode;
		last = newNode;
	}
	else
	{
		first = newNode;
	}

	nElements++;
}



template <class T>
bool LinkedList<T>::empty()
{
	if (first == NULL && last == NULL) {return true;}
	else {return false;}
}

template <class T>
unsigned LinkedList<T>::size()
{
	return nElements;
}

template <class T>
void LinkedList<T>::clear()
{
	Node<T>* temp = first;
	while(temp != NULL)
	{
		temp = temp->next;
		first = temp;
		delete(temp);
	}

	first = NULL;
	last = NULL;
	nElements = 0;

}


template <class T>
void LinkedList<T>::removeFromFront()
{

	if (first == NULL && last == NULL) {
		llvm::report_fatal_error("Cannot remove an element from an empty list");
	}
	else
	{
		Node<T>* temp;

		temp = first;
		first = first->next;

		delete(temp);
		nElements--;
	}
}



template <class T>
void LinkedList<T>::removeElement(T element)
{
	bool elementFound = false;
	Node<T>* temp = first;
	Node<T>* prevTemp = NULL;

	if(empty())
		llvm::report_fatal_error("Cannot remove an element from an empty list");

	while(elementFound == false && temp != NULL){
		if(temp->val == element){
			elementFound = true;
			if(temp == first){
				first = temp->next;
				delete(temp);
				nElements--;

			}else{
				if(temp == last){
					//std::cout << "Element is last\n";
					prevTemp->next = NULL;
					last = prevTemp;
					delete(temp);
					nElements--;

				}else{
					prevTemp->next = temp->next;
				//	std::cout << "Deleting element\n";
					delete(temp);
					nElements--;
				}
			}
		}else{
			prevTemp = temp;
			temp = temp->next;
		}
	}
}



// Return the position of the element, or -1 if not found
template <class T>
int LinkedList<T>::findElement(T element)
{
	bool elementFound = false;
	int position = 0;
	Node<T>* temp = first;

	while(elementFound == false && temp != NULL){

		if(temp->val == element){
			elementFound = true;
		//	std::cout<< "Element found\n";
			//std::cout<< "Stack before:\n";
			//print();
			T tempFound = temp->val;

			removeElement(temp->val);
			//std::cout<< "Stack after:\n";
						//print();
						//std::cout << "insertAtBack\n";
						//std::cout << "Stack before:\n";
						//		print();
			insertAtBack(tempFound);
			//std::cout << "Stack after:\n";
										//	print();
			// Break because we do not want position to be increased.
			break;
		}
		position++;
		temp = temp->next;
	}
	if(elementFound == false)
		position = -1;

	return position;
}

