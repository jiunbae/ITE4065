/*****************************************************************************

MIT License

Copyright (c) 2017 Bae jiun, Maybe

@see also https://choosealicense.com/licenses/mit/

*****************************************************************************/

/******************************************************************//**
@file include/lf0lst.h
lock free list utilities

Created 3/12/2017 Jiun Bae
***********************************************************************/

#ifndef __LOCK_FREE_LIST__
#define __LOCK_FREE_LIST__

#include <atomic>
#include <limits>

namespace lockfree {

	template <typename Type>
	class Node;

	template <typename NodePtr>
	class Window;

	template <typename Type, typename NodePtr>
	class List;

	// hash_type is a structure for casting a pointer to 
	// a type that can be compared with other hash_type.
	// 
	// The following example uses reinterpret_cast to cast the pointer. 
	// An uintptr_t(unsigned long long) can hold a pointer in 64 bits.
	using hash_type = std::uintptr_t;
 

	/*
		static constexpr set or clear mark and check mark for atomic
	*/
	template <typename node_ptr>
	static constexpr
		node_ptr SET_MARKED(node_ptr p) {
		return (node_ptr)(((hash_type)(p)) | 1);
	}
	template <typename node_ptr>
	static constexpr
		node_ptr CLEAR_MARKED(node_ptr p) {
		return (node_ptr)(((hash_type)(p)) & ~1);
	}
	template <typename node_ptr>
	static constexpr
		hash_type IS_MARKED(node_ptr p) {
		return ((hash_type)(p)) & 1;
	}

	// Cast elem pointer to hash_type
	// using reinterpret_cast<>()
	template <typename elem_type>
	static constexpr
		hash_type hash_from_ptr(const elem_type* p) {
		return reinterpret_cast<hash_type>(p);
	}

	/**
	* The simple node for list
	* Using std::atomic for support lock-free list
	*
	* @param Type the type of node element
	*/
	template <typename Type>
	class Node {
		using node_ptr = Node<Type>*;

	public:
		Node(const Type& elem, hash_type key)
			: key(key), elem(elem) {
			std::atomic_init(&next, nullptr);
		}

	public:
		hash_type key;
		const Type& elem;
		// @see also http://en.cppreference.com/w/cpp/atomic/atomic
		std::atomic<node_ptr> next;
	};

	/**
	* The Window class for atomic snapshot in lock-free list
	*
	* @param NodePtr the type of the node pointer, point to pred and current.
	*/
	template <typename NodePtr>
	class Window {
		using node_ptr = NodePtr;

	public:
		Window(node_ptr pred, node_ptr curr)
			: pred(pred), curr(curr) {
		}

		// Atomic stnapshot
		static Window<node_ptr>* find(node_ptr head, hash_type value) {
			node_ptr pred = nullptr;
			node_ptr curr = nullptr;
			node_ptr succ = nullptr;

			bool snip = false;

			while (true) {
				pred = head;
				curr = pred->next.load();
				while (true) {
					succ = curr->next.load();
					while (IS_MARKED(succ)) {
						snip = pred->next.compare_exchange_strong(curr, succ);
						if (!snip) goto retry;
						curr = succ;
						succ = curr->next.load();
					}

					if (curr->key >= value) return new Window<node_ptr>(pred, curr);
					pred = curr;
					curr = succ;
				}
			retry:;
			}
		}

	public:
		node_ptr pred;
		node_ptr curr;
	};

	/**
	* The single-way lock-free linked list
	* I have tried to maintain compatibility with ut_list_base(in innobase).
	* However, for full compatibility, you need to implement some additional macros.
	* 
	* @param Type the type of the list element
	* @param NodePtr field member pointer that points to the list node, (but not implemented yet)
	*/
	template <typename Type, typename NodePtr>
	class List {
		using elem_type = Type;
		using node_ptr = NodePtr;
		using node_type = Node<Type>;
		using window_ptr = Window<node_ptr>*;

	public:
		// head and tail are set to the minimum and maximum values of hash_type.
		// Because all pointers are greater than 0 and less than hash_type(-1), insert them in order.
		List()
			: head(new Node<elem_type>(Type(0), hash_type(0)))
			, tail(new Node<elem_type>(Type(0), hash_type(-1))) {
			head->next.store(tail);
			tail->next.store(nullptr);
			std::atomic_init(&count, size_t(0));
		}

		~List() {}

		bool insert(const elem_type* elem) {
			hash_type key = hash_from_ptr(elem);

			while (true) {
				window_ptr window = Window<node_ptr>::find(head, key);

				node_ptr pred = window->pred;
				node_ptr curr = window->curr;

				if (curr->key == key) return false;
				else {
					node_ptr node = new Node<elem_type>(*elem, key);
					node->next = curr;

					if (pred->next.compare_exchange_strong(curr, node)) {
						increase_count();
						return true;
					}
				}
			}
		}

		bool remove(const elem_type* elem) {
			hash_type key = hash_from_ptr(elem);
			bool snip;

			while (true) {
				window_ptr window = Window<node_ptr>::find(head, key);
				node_ptr pred = window->pred;
				node_ptr curr = window->curr;

				if (curr->key != key) return false;
				else {
					node_ptr succ = curr->next;
					node_ptr next_marked = SET_MARKED(succ);
					snip = curr->next.compare_exchange_strong(succ, next_marked);

					if (!snip) continue;

					pred->next.compare_exchange_strong(curr, succ);
					decrease_count();
					return true;
				}
			}
		}

		bool contains(const elem_type* elem) const {
			hash_type key = hash_from_ptr(elem);
			bool marked = false;
			node_ptr curr = head;

			while (curr->key > key) {
				curr = curr->next;
				node_ptr succ = curr->next;
				marked = IS_MARKED(succ);
			}
			return (curr->key = key && !marked);
		}

		// TODO: implement utils
		//         - easy loop util
		//         - get last or first elem
		//         - support for-each style
		//         - support iterator (if possible)

		size_t size() const {
			return get_count();
		}

	private:
		void increase_count() {
			count.fetch_add(1);
		}

		void decrease_count() {
			count.fetch_sub(1);
		}

		size_t get_count() const {
			return count.load();
		}

	private:
		std::atomic<size_t> count;
		node_ptr head;
		node_ptr tail;
	};
}

#endif