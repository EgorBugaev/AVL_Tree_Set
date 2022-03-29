#include <algorithm>
#include <assert.h>
#include <cstddef>
#include <iostream>
#include <stack>

/* Class for balanced binary search tree based on AVL tree.
 * Balanced depth is achieved through keeping difference between heights of left and right children less than 2.
 * Allows inserting/extracting elements with logarithmic complexity, linear memory usage.
 */
template<class T>
class Set {
  private:
    class TNode;
  public:
    constexpr static size_t EMPTY_SET_SIZE = 0;

    Set() {
        size_ = EMPTY_SET_SIZE;
        root_ = nullptr;
        end_iter_ = iterator(nullptr, nullptr);
        begin_iter_ = end_iter_;
    }

    template<typename Iterator>
    Set(Iterator first, Iterator last) : Set() {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    Set(std::initializer_list<T> elems) : Set() {
        for (const T& elem : elems) {
            insert(elem);
        }
    }

    Set(const Set& st) : Set() {
        copy_all_nodes(st);
        size_ = st.size_;
        set_boundary_iters();
    }

    // Assignment constructor, linear time.
    Set& operator=(const Set& st) {
        if (st.begin() == begin_iter_) {
            return *this;
        }
        delete_all_nodes();

        copy_all_nodes(st);
        size_ = st.size_;
        set_boundary_iters();
        return *this;
    }

    // Destructor, linear time.
    ~Set() {
        delete_all_nodes();
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return (size_ == EMPTY_SET_SIZE);
    }

    // Inserts element in logarithmic time, simultaneously balances depth of the tree.
    void insert(const T& elem) {
        if (find(elem) != end_iter_) {
            return;
        }
        ++size_;

        if (root_ == nullptr) {
            root_ = new TNode(elem);
            set_boundary_iters();
            return;
        }

        TNode* node = root_;
        std::vector<TNode*> path;

        while (node != nullptr) {
            path.push_back(node);
            if (elem < node->val) {
                node = node->left;
            } else {
                node = node->right;
            }
        }

        std::reverse(path.begin(), path.end());
        if (path[0]->val < elem) {
            path[0]->set_right(new TNode(elem));
        } else {
            path[0]->set_left(new TNode(elem));
        }

        balance_path(path);
        set_boundary_iters();
    }

    /* Finds node with the closest (less or equal) value and one child, swaps values with original node and deletes node with one child.
     * The only child of deleted node is reassigned to its parent in place of the deleted. Logarithmic time.
     */
    void erase(const T& elem) {
        if (find(elem) == end_iter_) {
            return;
        }
        --size_;

        TNode* node = root_;
        std::vector<TNode*> path;
        TNode* x = nullptr;

        while (node != nullptr) {
            path.push_back(node);
            if (are_equal_values(elem, node->val)) {
                x = node;
                node = node->left;
                while (node != nullptr) {
                    path.push_back(node);
                    node = node->right;
                }
                break;
            }

            if (elem < node->val) {
                node = node->left;
            } else {
                node = node->right;
            }
        }

        if (x == nullptr) {
            assert(0);
        }
        std::reverse(path.begin(), path.end());

        if (path[0] != x) {
            std::swap(x->val, path[0]->val);
        }

        delete_TNode(path[0]);
        balance_path(path);
        set_boundary_iters();
    }

    // Allows external access to the values stored in tree and finds next/prev element.
    class iterator {
      public:
        iterator() {
            node_ = nullptr;
            root_ = nullptr;
        }

        iterator(Set<T>::TNode* node, Set<T>::TNode* root): node_(node), root_(root) {}

        const T &operator*() {
            if (node_ == nullptr) {
                assert(0);
            }
            return node_->val;
        }

        const T *operator->() {
            return &(node_->val);
        }

        bool operator==(const iterator& b) const {
            return (node_ == b.node_) && (root_ == b.root_);
        }

        bool operator!=(const iterator& b) const {
            return (node_ != b.node_) || (root_ != b.root_);
        }

        // Finds next element in the tree, amortized time O(1), real time O(log size).
        iterator& operator++() {
            if (node_ == nullptr) {
                return *this;
            }
            if (node_->right != nullptr) {
                node_ = node_->right;
                while (node_->left != nullptr) {
                    node_ = node_->left;
                }
                return *this;
            }

            const TNode *prev = node_;
            node_ = node_->parent;
            while (node_ != nullptr && node_->right == prev) {
                prev = node_;
                node_ = node_->parent;
            }
            return *this;
        }

        // Finds previous element in the tree, amortized time O(1), real O(log size).
        iterator& operator--() {
            if (node_ == nullptr) {
                node_ = root_;
                while (node_->right != nullptr) {
                    node_ = node_->right;
                }
                return *this;
            }
            if (node_->left != nullptr) {
                node_ = node_->left;
                while (node_->right != nullptr) {
                    node_ = node_->right;
                }
                return *this;
            }

            const TNode *prev = node_;
            node_ = node_->parent;
            while (node_ != nullptr && node_->left == prev) {
                prev = node_;
                node_ = node_->parent;
            }
            if (node_ == nullptr) {
                assert(0);
            }
            return *this;
        }

        // Finds next element in the tree, amortized time O(1), real time O(log size).
        const iterator operator++(int) {
            iterator old_copy = *this;

            if (node_ == nullptr) {
                return *this;
            }
            if (node_->right != nullptr) {
                node_ = node_->right;
                while (node_->left != nullptr) {
                    node_ = node_->left;
                }
                return *this;
            }

            const TNode *prev = node_;
            node_ = node_->parent;
            while (node_ != nullptr && node_->right == prev) {
                prev = node_;
                node_ = node_->parent;
            }
            return old_copy;
        }

        // Finds previous element in the tree, amortized time O(1), real O(log size).
        const iterator operator--(int) {
            iterator old_copy = *this;

            if (node_ == nullptr) {
                node_ = root_;
                while (node_->right != nullptr) {
                    node_ = node_->right;
                }
                return *this;
            }
            if (node_->left != nullptr) {
                node_ = node_->left;
                while (node_->right != nullptr) {
                    node_ = node_->right;
                }
                return *this;
            }

            const TNode *prev = node_;
            node_ = node_->parent;
            while (node_ != nullptr && node_->left == prev) {
                prev = node_;
                node_ = node_->parent;
            }
            if (node_ == nullptr) {
                assert(0);
            }
            return old_copy;
        }

      private:
        const Set<T>::TNode *node_;
        const Set<T>::TNode *root_;
    };

    iterator begin() const {
        return begin_iter_;
    }

    iterator end() const {
        return end_iter_;
    }

    // Finds element with given value or returns end_iter_ if it doesn't exist. Logarithmic time.
    iterator find(const T& elem) const {
        TNode* node = root_;

        while (node != nullptr) {
            if (!(node->val < elem) && !(elem < node->val)) {
                return iterator(node, root_);
            }
            if (elem < node->val) {
                node = node->left;
            } else {
                node = node->right;
            }
        }
        return end();
    }

    // Finds the leftmost element with value greater or equal to given value. Logarithmic time.
    iterator lower_bound(const T& elem) const {
        if (find(elem) != end()) {
            return find(elem);
        }
        TNode* node = root_;

        TNode* last_successful = nullptr;
        while (node != nullptr) {
            if (node->val < elem) {
                node = node->right;
            } else {
                last_successful = node;
                node = node->left;
            }
        }
        return iterator(last_successful, root_);
    }

  private:
    // Element in AVL tree storing actual value and three connected elements.
    class TNode {
    public:
        // Constants showing at which difference children's heights are considered imbalance.
        constexpr static int IMBALANCE_TO_LEFT = 2;
        constexpr static int IMBALANCE_TO_RIGHT = -2;

        // Constant showing at which difference children's heights are considered tilted so that they would become imbalanced after improper shifting.
        constexpr static int TILTED_LEFT = 1;
        constexpr static int TILTED_RIGHT = -1;

        TNode *left, *right, *parent;
        T val;

        TNode() {
            left = nullptr;
            right = nullptr;
            parent = nullptr;
            val();
        }

        explicit TNode(T value): val(value) {
            left = nullptr;
            right = nullptr;
            parent = nullptr;
        }

        // Returns difference between left son's height and right son's height.
        int32_t diff() {
            int left_height = (left == nullptr ? 0 : left->height);
            int right_height = (right == nullptr ? 0 : right->height);
            return left_height - right_height;
        }

        // Calculates this node's height based on children's height.
        void update_height() {
            int left_height = (left == nullptr ? 0 : left->height);
            int right_height = (right == nullptr ? 0 : right->height);
            height = std::max(left_height + 1, right_height + 1);
        }

        void set_right(TNode* new_right) {
            right = new_right;
            if (new_right != nullptr) {
                new_right->parent = this;
            }
            update_height();
        }

        void set_left(TNode* new_left) {
            left = new_left;
            if (new_left != nullptr) {
                new_left->parent = this;
            }
            update_height();
        }
    private:
        int32_t height;

    };

    TNode *root_;
    size_t size_;
    iterator begin_iter_, end_iter_;

    // Compare for equivalence without requiring == operator.
    bool are_equal_values(T val_a, T val_b) {
        return !(val_a < val_b) && !(val_b < val_a);
    }

    // Deletes node from tree and correctly reassigns parents and children. Node must have no more than one child.
    void delete_TNode(TNode* node) {
        if (node->parent == nullptr) {
            root_ = node->left;
            if (node->left == nullptr) {
                root_ = node->right;
            }

            if (root_ != nullptr) {
                root_->parent = nullptr;
            }

            delete node;
            return;
        }

        TNode* parent = node->parent;
        if (parent->right == node) {
            parent->set_right(node->left);
            if (node->left == nullptr) {
                parent->set_right(node->right);
            }
        }
        else {
            parent->set_left(node->left);
            if (node->left == nullptr) {
                parent->set_left(node->right);
            }
        }
        delete node;
    }

    // Balances all nodes on a vertical path in a tree starting from the lowest;
    void balance_path(std::vector<TNode*>& path) {
        for (int i = 1; i < path.size(); ++i) {
            path[i]->update_height();
            if (path[i]->left != nullptr) {
                path[i]->set_left(balance_node(path[i]->left));
            }
            if (path[i]->right != nullptr) {
                path[i]->set_right(balance_node(path[i]->right));
            }
        }

        root_ = balance_node(root_);
    }

    // Makes this set a deep copy of another in linear time.
    void copy_all_nodes(const Set& st) {
        if (st.root_ == nullptr) {
            return;
        }

        std::stack<std::pair<TNode*, TNode*>> q;
        root_ = new TNode(st.root_->val);
        q.push({st.root_, root_});
        while (!q.empty()) {
            auto p = q.top();
            q.pop();

            if (p.first->left != nullptr) {
                p.second->set_left(new TNode(p.first->left->val));
                q.push({p.first->left, p.second->left});
            }
            if (p.first->right != nullptr) {
                p.second->set_right(new TNode(p.first->right->val));
                q.push({p.first->right, p.second->right});
            }
        }
    }

    // Deletes each node while traversing tree, linear time.
    void delete_all_nodes() {
        if (root_ == nullptr) {
            return;
        }

        std::stack<TNode*> q;
        q.push(root_);
        while (!q.empty()) {
            TNode* cur = q.top();
            q.pop();
            if (cur->left != nullptr) {
                q.push(cur->left);
            }
            if (cur->right != nullptr) {
                q.push(cur->right);
            }

            delete cur;
        }

        root_ = nullptr;
        begin_iter_ = iterator(nullptr, nullptr);
        end_iter_ = iterator(nullptr, nullptr);
        size_ = 0;
    }

    /* Balances nodes' depth by swapping this node and its children so that children's heights differ for no more than 1.
     * Before balancing we adjust children's heights so that rotating does not unbalance children.
     */
    TNode* balance_node(TNode* old_root) {
        if (old_root == nullptr) {
            return old_root;
        }

        if (old_root->diff() == TNode::IMBALANCE_TO_RIGHT && old_root->right->diff() == TNode::TILTED_LEFT) {
            old_root->set_right(increase_right_height(old_root->right));
            return increase_left_height(old_root);
        } else if (old_root->diff() == TNode::IMBALANCE_TO_RIGHT) {
            return increase_left_height(old_root);
        } else if (old_root->diff() == TNode::IMBALANCE_TO_LEFT && old_root->left->diff() == TNode::TILTED_RIGHT) {
            old_root->set_left(increase_left_height(old_root->left));
            return increase_right_height(old_root);
        } else if(old_root->diff() == TNode::IMBALANCE_TO_LEFT) {
            return increase_right_height(old_root);
        }
        return old_root;
    }

    // Makes right child new root in this subtree, thus shifting old root to be its left child and increasing left child's height.
    TNode* increase_left_height(TNode* old_root) {
        if (old_root->right == nullptr) {
            return old_root;
        }
        TNode* root_parent = old_root->parent;

        TNode* new_root = old_root->right;
        old_root->set_right(new_root->left);

        new_root->set_left(old_root);
        new_root->parent = root_parent;
        return new_root;
    }

    // Makes left child new root in this subtree, thus shifting old root to be its right child and increasing right child's height.
    TNode* increase_right_height(TNode* old_root) {
        if (old_root->left == nullptr) {
            return old_root;
        }
        TNode* root_parent = old_root->parent;

        TNode* new_root = old_root->left;
        old_root->set_left(new_root->right);

        new_root->set_right(old_root);
        new_root->parent = root_parent;
        return new_root;
    }

    // Finds iterators for the first element and the element after the last.
    void set_boundary_iters() {
        TNode* node = root_;
        while (node != nullptr && node->left != nullptr) {
            node = node->left;
        }
        begin_iter_ = iterator(node, root_);
        end_iter_ = iterator(nullptr, root_);
    }
};
