#ifndef _STACK_CPP_
#define _STACK_CPP_

#include "Stack.h"

template <typename T>
Stack<T>::Stack() : head(nullptr), stack_size(0) {}

template <typename T>
Stack<T>::~Stack() {
    Node* temp;
    while (head) {
        temp = head, head = head->next;
        delete temp;
    }
}

template <typename T>
void Stack<T>::push(T value) {
    Node* new_node = new Node(value);
    new_node->next = head, head = new_node, ++stack_size;
}

template <typename T>
T& Stack<T>::top() {
    return head->data;
}

template <typename T>
void Stack<T>::pop() {
    if (head) {
        Node* temp = head;
        head = head->next, --stack_size;
        delete temp;
    }
}

template <typename T>
bool Stack<T>::empty() const {
    return head == nullptr;
}

template <typename T>
int Stack<T>::size() const {
    return stack_size;
}

template <typename T>
Stack<T>::Node::Node(T value) : data(value), next(nullptr) {}

#endif