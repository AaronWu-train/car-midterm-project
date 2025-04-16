template<typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
    } *m_front, m_rear
    size_t m_size;
public:
    Queue(void) : m_front(nullptr), m_rear(nullptr), m_size(0) {}
    ~Queue(void) {
        while (m_front != nullptr) {
            Node* temp = m_front;
            m_front = m_front->next;
            delete temp;
        }
    }
    void push(const T& data) {
        Node* newNode = new Node{data, nullptr};
        if (m_rear != nullptr) {
            m_rear->next = newNode;
        } else {
            m_front = newNode;
        }
        m_rear = newNode;
        ++m_size;
    }
    void pop(void) {
        if (m_front != nullptr) {
            Node* temp = m_front;
            m_front = m_front->next;
            delete temp;
            --m_size;
            if (m_front == nullptr) {
                m_rear = nullptr;
            }
        }
    }
    T& front(void) {
        if (m_front != nullptr) {
            return m_front->data;
        } else {
            throw std::out_of_range("Queue is empty");
        }
    }
    bool empty(void) const {
        return m_front == nullptr;
    }
    size_t size(void) const {
        return m_size;
    }
    void clear(void) {
        while (m_front != nullptr) {
            Node* temp = m_front;
            m_front = m_front->next;
            delete temp;
        }
        m_rear = nullptr;
        m_size = 0;
    }
};

void setup() {

}
void loop() {

}