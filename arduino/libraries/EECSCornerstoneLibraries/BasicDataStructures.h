#ifndef _BASIC_DATA_STRUCTURES_H_
#define _BASIC_DATA_STRUCTURES_H_

template <typename TypeA, typename TypeB>
class pair { 
    TypeA first; 
    TypeB second; 
    bool operator< (pair<TypeA, TypeB> other);
};

template <typename Type>
class vector {
    private:
        int m_size;
        Type *m_begin, *m_end;
    public:
        vector(void);
        Type& operator[] (int index);
        Type* begin(void);
        Type* end(void);
        int size(void);
        bool empty(void);
        void resize(int size);
        void resize(int size, Type initial_value);
        void reserve(int size);
        void push_back(Type value);
};

template<typename Type, bool (*CompareGreater) (const Type&, const Type&)>
class priority_queue {
    private:
        int m_size;    
        vector<Type> m_heap;
        void heapify(void);
    public:
        priority_queue(void);
        priority_queue(Type* initial_array_begin, Type* initial_array_end);
        priority_queue(vector<Type> initial_array);
        void push(Type value);
        Type& top(void);
        void pop(void);
        bool empty(void);
        int size(void);
};

#endif