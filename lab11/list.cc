#include <mutex>

template <typename T>
class List {
public:
    class Node {
    public:
        Node(T item) : item(item), next(nullptr), front(nullptr) {}
        T item;
        Node* next;
        Node* front;

        Node* make_next(T item) {
            Node* child = new Node(item);
            child->front = this;
            child->next = this->next;
            if (child->next != nullptr) child->next->front = child; 
            this->next = child;
            return child;
        }

        Node* make_front(T item) {
            Node* child = new Node(item);
            child->next = this;
            child->front = this->front;
            if (child->front != nullptr) child->front->next = child; 
            this->front = child;
            return child;
        }

        void lock() {
            locker.lock();
        }

        void unlock() {
            locker.unlock();
        }
    private:
        mutable std::mutex locker;
    };

    List() : head(nullptr), count(0) { }

    bool insert(T item) {
        mutex.lock();
        if (head == nullptr) {
            head = new Node(item);
            mutex.unlock();
            count += 1;
            return true;
        } else {
            Node* par = nullptr;
            Node* cur = head;
            cur->lock();
            mutex.unlock();

            while (true) {
                if (cur->item > item) {
                    cur->make_front(item);
                    if (par != nullptr) par->unlock();
                    cur->unlock();
                    count += 1;
                    return true;
                } else if (cur->item == item) {
                    if (par != nullptr) par->unlock();
                    cur->unlock();
                    return false;
                } else if (cur->next == nullptr) {
                    cur->make_next(item);
                    cur->unlock();
                    if (par != nullptr) par->unlock();
                    count += 1;
                    return true;
                }
                par->unlock();
                par = cur;
                cur = cur->next;
                cur->lock();
            }
        }
    }

    bool remove(T item) {

    }

    bool contains(T item) {

    }

    size_t size() const {
        return count;
    }

private:
    int count;
    Node* head;
    mutable std::mutex mutex;
};
