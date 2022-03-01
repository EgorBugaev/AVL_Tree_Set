#include <cstddef>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <queue>

template<class T>
class Set {
private:
    class TNode;
public:
    friend class Iterator;

    Set() {
        sz_ = 0;
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
        for (T elem : elems) {
            insert(elem);
        }
    }

    Set(const Set& st) : Set() {
        copy_all_nodes(st);
        sz_ = st.sz_;
        set_boundary_iters();
    }

    Set& operator= (const Set& st) {
        if (st.begin() == begin_iter_) {
            return *this;
        }

        if (root_ != nullptr) {
            std::queue<TNode*> q;
            q.push(root_);
            while (!q.empty()) {
                TNode* cur = q.front();
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
            sz_ = 0;
        }

        copy_all_nodes(st);
        sz_ = st.sz_;
        set_boundary_iters();
        return *this;
    }

    ~Set() {
        if (root_ == nullptr) {
            return;
        }

        std::vector<TNode*> all_v;
        std::queue<TNode*> q;
        q.push(root_);
        while (!q.empty()) {
            TNode* cur = q.front();
            q.pop();
            if (cur->left != nullptr) {
                q.push(cur->left);
            }
            if (cur->right != nullptr) {
                q.push(cur->right);
            }

            delete cur;
        }
    }

    size_t size() const {
        return sz_;
    }

    bool empty() const {
        return (sz_ == 0);
    }

    void insert(const T& elem) {
        if (find(elem) != end_iter_) {
            return;
        }
        ++sz_;

        if (root_ == nullptr) {
            root_ = new TNode(elem);
            set_boundary_iters();
            return;
        }

        TNode* node = root_;
        std::vector<TNode* > way;

        while (node != nullptr) {
            way.push_back(node);
            if (elem < node->val) {
                node = node->left;
            } else {
                node = node->right;
            }
        }

        std::reverse(way.begin(), way.end());
        if (way[0]->val < elem) {
            way[0]->right = new TNode(elem);
            way[0]->right->parent = way[0];
        } else {
            way[0]->left = new TNode(elem);
            way[0]->left->parent = way[0];
        }
        for (int i = 0; i < way.size(); ++i) {
            way[i]->update_height();
            if (way[i]->left != nullptr) {
                way[i]->left = balance_node(way[i]->left);
                way[i]->left->parent = way[i];
            }
            if (way[i]->right != nullptr) {
                way[i]->right = balance_node(way[i]->right);
                way[i]->right->parent = way[i];
            }
        }
        root_ = balance_node(root_);
        root_->parent = nullptr;
        set_boundary_iters();
    }

    void erase(const T& elem) {
        if (find(elem) == end_iter_) {
            return;
        }
        --sz_;

        TNode* node = root_;
        std::vector<TNode* > way;
        TNode* x = nullptr;
        while (node != nullptr) {
            way.push_back(node);
            if (!(elem < node->val) && !(node->val < elem)) {
                x = node;
                if (node->diff() <= 0) {
                    node = node->left;
                    while (node != nullptr) {
                        way.push_back(node);
                        node = node->right;
                    }
                } else if (node->right != nullptr) {
                    node = node->right;
                    while (node != nullptr) {
                        way.push_back(node);
                        node = node->left;
                    }
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
        std::reverse(way.begin(), way.end());

        if (way[0] != x) {
            std::swap(x->val, way[0]->val);
        }
        if (way.size() > 1) {
            if (way[1]->right == way[0])
                if (way[0]->left != nullptr) {
                    way[1]->right = way[0]->left;
                    way[1]->right->parent = way[1];
                } else if (way[0]->right != nullptr){
                    way[1]->right = way[0]->right;
                    way[1]->right->parent = way[1];
                } else {
                    way[1]->right = nullptr;
                }
            else {
                if (way[0]->left != nullptr) {
                    way[1]->left = way[0]->left;
                    way[1]->left->parent = way[1];
                } else if (way[0]->right != nullptr) {
                    way[1]->left = way[0]->right;
                    way[1]->left->parent = way[1];
                } else {
                    way[1]->left = nullptr;
                }
            }
        } else {
            if (way[0]->left != nullptr) {
                root_ = way[0]->left;
                root_->parent = way[1];
            } else if (way[0]->right != nullptr){
                root_ = way[0]->right;
                root_->parent = nullptr;
            } else {
                root_ = nullptr;
            }
        }
        delete way[0];

        for (int i = 1; i < way.size(); ++i) {
            way[i]->update_height();
            if (way[i]->left != nullptr) {
                way[i]->left = balance_node(way[i]->left);
                way[i]->left->parent = way[i];
            }
            if (way[i]->right != nullptr) {
                way[i]->right = balance_node(way[i]->right);
                way[i]->right->parent = way[i];
            }
        }

        if (root_ != nullptr) {
            root_ = balance_node(root_);
            root_->parent = nullptr;
        }

        set_boundary_iters();
    }

    class iterator {
        friend class Set<T>;
    public:
        iterator() {
            node_ = nullptr;
            root_ = nullptr;
        }
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
        iterator& operator++ () {
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
        iterator& operator-- () {
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
        iterator operator++ (int) {
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
        iterator operator-- (int) {
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
        iterator(Set<T>::TNode* node, Set<T>::TNode* root): node_(node), root_(root) {}

        const Set<T>::TNode *node_;
        const Set<T>::TNode *root_;
    };

    iterator begin() const {
        return begin_iter_;
    }

    iterator end() const {
        return end_iter_;
    }

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
    class TNode {
    public:
        TNode *left, *right, *parent;
        T val;

        TNode() {
            left = nullptr;
            right = nullptr;
            parent = nullptr;
            height = 1;
            val();
        }
        explicit TNode(T value): val(value) {
            left = nullptr;
            right = nullptr;
            parent = nullptr;
            height = 1;
        }

        int32_t diff() {
            int left_height = (left == nullptr ? 0 : left->height);
            int right_height = (right == nullptr ? 0 : right->height);
            return left_height - right_height;
        }
        void update_height() {
            int left_height = (left == nullptr ? 0 : left->height);
            int right_height = (right == nullptr ? 0 : right->height);
            height = std::max(left_height + 1, right_height + 1);
        }
    private:
        int32_t height;
    };

    TNode *root_;
    size_t sz_;
    iterator begin_iter_, end_iter_;

    void copy_all_nodes(const Set& st) {
        if (st.root_ == nullptr) {
            return;
        }

        std::queue<std::pair<TNode*, TNode*> > q;
        root_ = new TNode(st.root_->val);
        q.push({st.root_, root_});
        while (!q.empty()) {
            auto p = q.front();
            q.pop();

            if (p.first->left != nullptr) {
                p.second->left = new TNode(p.first->left->val);
                p.second->left->parent = p.second;
                p.second->update_height();
                q.push({p.first->left, p.second->left});
            }
            if (p.first->right != nullptr) {
                p.second->right = new TNode(p.first->right->val);
                p.second->right->parent = p.second;
                p.second->update_height();
                q.push({p.first->right, p.second->right});
            }
        }
    }

    TNode* balance_node (TNode* a) {
        if (a == nullptr) {
            return a;
        }

        if (a->diff() == -2 && (a->right->diff() == -1 || a->right->diff() == 0)) {
            return left_rotation(a);
        } else if (a->diff() == -2 && a->right->diff() == 1) {
            a->right = right_rotation(a->right);
            a->right->parent = a;
            a->update_height();
            return left_rotation(a);
        } else if(a->diff() == 2 && (a->left->diff() == 1 || a->left->diff() == 0)) {
            return right_rotation(a);
        } else if (a->diff() == 2 && a->left->diff() == -1) {
            a->left = left_rotation(a->left);
            a->left->parent = a;
            a->update_height();
            return right_rotation(a);
        }
        return a;
    }

    TNode* left_rotation(TNode* a) {
        if (a->right == nullptr) {
            return a;
        }
        TNode* b = a->right;
        a->right = b->left;
        if (a->right != nullptr) {
            a->right->parent = a;
        }
        a->update_height();
        b->left = a;
        if (b->left != nullptr) {
            b->left->parent = b;
        }
        b->update_height();
        return b;
    }
    TNode* right_rotation(TNode* a) {
        if (a->left == nullptr) {
            return a;
        }
        TNode* b = a->left;
        a->left = b->right;
        if (a->left != nullptr) {
            a->left->parent = a;
        }
        a->update_height();
        b->right = a;
        if (b->right != nullptr) {
            b->right->parent = b;
        }
        b->update_height();
        return b;
    }

    void set_boundary_iters() {
        TNode* node = root_;
        while (node != nullptr && node->left != nullptr) {
            node = node->left;
        }
        begin_iter_ = iterator(node, root_);
        end_iter_ = iterator(nullptr, root_);
    }
};
