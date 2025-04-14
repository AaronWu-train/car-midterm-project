#ifndef _STACK_H_
#define _STACK_H_

template <typename T>
class Stack {
public:
    Stack();
    ~Stack();
    void push(T value);
    T& top();
    void pop();
    bool empty() const;
    int size() const;
private:
    struct Node {
        T data;
        Node* next;
        Node(T value) : data(value), next(nullptr) {}
    };
    Node* head;
    int stack_size;
};

#include "Stack.cpp"

#endif