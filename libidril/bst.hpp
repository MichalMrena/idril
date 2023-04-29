#ifndef LIBIDRIL_BST_HPP
#define LIBIDRIL_BST_HPP

#include <cstddef>
#include <memory>
#include <utility>

namespace idril
{
namespace details
{
    template<class T>
    struct less
    {
        [[nodiscard]]
        constexpr auto operator()(T const& l, T const& r) const -> bool
        {
            return l < r;
        }
    };

    template<bool B, class T, class F>
    struct type_if;

    template<class T, class F>
    struct type_if<true, T, F>
    {
        using type = T;
    };

    template<class T, class F>
    struct type_if<false, T, F>
    {
        using type = F;
    };

    template<bool B, class T, class F>
    using type_if_t = typename type_if<B, T, F>::type;
}

/**
 *  \version 1.0.0
 */
template< class Key
        , class T
        , class Compare   = details::less<Key>
        , class Allocator = std::allocator<std::pair<Key const, T>> >
class Bst
{
private:
    struct BstNode
    {
        template<class... Args>
        BstNode (Args&&...);
        BstNode (BstNode const&) = delete;
        BstNode (BstNode&&) = delete;

        std::pair<Key const, T> data_;
        BstNode* parent_ {nullptr};
        BstNode* left_ {nullptr};
        BstNode* right_ {nullptr};
    };

    static auto node_key (BstNode*) -> Key const&;
    static auto node_data (BstNode*) -> T&;
    static auto node_degree (BstNode*) -> int;

    enum class Ordering
    {
        LT, EQ, GT
    };

    struct FindSpotResult
    {
        BstNode* parent_;
        BstNode** son_;
    };

public:
    template<bool IsConst>
    class BstIterator
    {
    private:
        using pair_t = typename details::type_if_t<IsConst, const std::pair<Key const, T>, std::pair<Key const, T>>;

    public:
        BstIterator () = default;
        BstIterator (BstNode*);

        auto operator* () const -> pair_t&;
        auto operator++ () -> BstIterator&;
        auto operator== (BstIterator const&) const -> bool;
        auto operator!= (BstIterator const&) const -> bool;

    private:
        friend class Bst<Key, T, Compare, Allocator>;

    private:
        BstNode* current_ {nullptr};
    };

public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<Key const, T>;
    using iterator = BstIterator<false>;
    using const_iterator = BstIterator<true>;

public:
    auto insert (value_type const&) -> std::pair<iterator, bool>;
    auto insert (value_type&&) -> std::pair<iterator, bool>;

    template<class M>
    auto insert_or_assign (key_type const&, M&&) -> std::pair<iterator, bool>;

    template<class M>
    auto insert_or_assign (key_type&&, M&&) -> std::pair<iterator, bool>;

    template<class... Args>
    auto try_emplace (key_type const&, Args&&...) -> std::pair<iterator, bool>;

    template<class... Args>
    auto try_emplace (key_type&&, Args&&...) -> std::pair<iterator, bool>;

    template<class... Args>
    auto emplace (Args&&...) -> std::pair<iterator, bool>;

    auto find (key_type const&) -> iterator;
    auto find (key_type const&) const -> const_iterator;

    auto lookup (key_type const&) -> mapped_type*;
    auto lookup (key_type const&) const -> mapped_type const*;

    auto erase (iterator) -> iterator;
    auto erase (const_iterator) -> iterator;
    auto erase (key_type const&) -> std::size_t;

public:
    Bst ();
    Bst (Bst const&);
    Bst (Bst&&);
    auto operator= (Bst) -> Bst&;
    auto swap (Bst&) -> void;
    auto size () const -> std::size_t;
    auto ssize () const -> std::ptrdiff_t;
    auto empty() const -> bool;
    auto begin () -> iterator;
    auto end () -> iterator;
    auto begin () const -> const_iterator;
    auto end () const -> const_iterator;
    auto cbegin () const -> const_iterator;
    auto cend () const -> const_iterator;

private:
    template<class... Args>
    [[nodiscard]]
    auto new_node (Args&&...) -> BstNode*;
    auto delete_node (BstNode*) -> void;
    auto find_node (Key const&) const -> BstNode*;
    auto find_spot (Key const&) const -> FindSpotResult;
    auto compare (Key const&, BstNode*) const -> Ordering;
    template<class... Args>
    auto try_insert (Key const&, Args&&...) -> std::pair<iterator, bool>;
    template<class K, class M>
    auto insert_or_assign_impl (K&&, M&&) -> std::pair<iterator, bool>;
    auto erase_node (BstNode*) -> BstNode*;
    auto extract_node (BstNode*) -> void;

    static auto next_in_order (BstNode*) -> BstNode*;
    static auto leftmost (BstNode*) -> BstNode*;
    static auto is_left_son (BstNode*) -> bool;
    static auto is_right_son (BstNode*) -> bool;
    static auto is_root (BstNode*) -> bool;
    static auto has_right_son (BstNode*) -> bool;
    static auto has_left_son (BstNode*) -> bool;

private:
    using ttt          = std::allocator_traits<Allocator>;
    using alloc_traits = typename ttt::template rebind_traits<BstNode>;
    using allocator    = typename ttt::template rebind_alloc<BstNode>;

private:
    BstNode*    root_;
    std::size_t size_;
    [[no_unique_address]]
    allocator   alloc_;
    [[no_unique_address]]
    Compare     cmp_;
};

// bst public api:

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::insert
    (value_type const& v) -> std::pair<iterator, bool>
{
    return this->try_insert(v.first, v);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::insert
    (value_type&& v) -> std::pair<iterator, bool>
{
    return this->try_insert(v.first, std::move(v));
}

template<class Key, class T, class Compare, class Allocator>
template<class M>
auto Bst<Key, T, Compare, Allocator>::insert_or_assign
    (key_type const& k, M&& m) -> std::pair<iterator, bool>
{
    return this->insert_or_assign_impl(k, std::forward<M>(m));
}

template<class Key, class T, class Compare, class Allocator>
template<class M>
auto Bst<Key, T, Compare, Allocator>::insert_or_assign
    (key_type&& k, M&& m) -> std::pair<iterator, bool>
{
    return this->insert_or_assign_impl(std::move(k), std::forward<M>(m));
}

template<class Key, class T, class Compare, class Allocator>
template<class... Args>
auto Bst<Key, T, Compare, Allocator>::try_emplace
    (Key const& k, Args&&... as) -> std::pair<iterator, bool>
{
    return this->try_insert(
        k,
        std::piecewise_construct,
        std::forward_as_tuple(k),
        std::forward_as_tuple(std::forward<Args>(as)...)
    );
}

template<class Key, class T, class Compare, class Allocator>
template<class... Args>
auto Bst<Key, T, Compare, Allocator>::try_emplace
    (Key&& k, Args&&... as) -> std::pair<iterator, bool>
{
    return this->try_insert(
        k,
        std::piecewise_construct,
        std::forward_as_tuple(std::move(k)),
        std::forward_as_tuple(std::forward<Args>(as)...)
    );
}

template<class Key, class T, class Compare, class Allocator>
template<class... Args>
auto Bst<Key, T, Compare, Allocator>::emplace
    (Args&&... as) -> std::pair<iterator, bool>
{
    if (this->empty())
    {
        root_ = this->new_node(std::forward<Args>(as)...);
        ++size_;
        return {root_, true};
    }

    auto const node = this->new_node(std::forward<Args>(as)...);
    auto const [parent, sonp] = this->find_spot(node_key(node));
    if (not sonp)
    {
        this->delete_node(node);
        return {parent, false};
    }

    node->parent_ = parent;
    *sonp = node;
    ++size_;
    return {node, true};
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::find
    (key_type const& k) -> iterator
{
    return iterator(this->find_node(k));
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::find
    (key_type const& k) const -> const_iterator
{
    return const_iterator(this->find_node(k));
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::lookup
    (key_type const& k) -> mapped_type*
{
    auto const node = this->find_node(k);
    return node ? &node_data(node) : nullptr;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::lookup
    (key_type const& k) const -> mapped_type const*
{
    auto const node = this->find_node(k);
    return node ? &node_data(node) : nullptr;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::erase
    (iterator const it) -> iterator
{
    return this->erase_node(it.current_);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::erase
    (const_iterator const it) -> iterator
{
    return this->erase_node(it.current_);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::erase
    (Key const& k) -> std::size_t
{
    auto const node = this->find_node(k);
    if (not node)
    {
        return 0;
    }
    this->erase_node(node);
    return 1;
}

template<class Key, class T, class Compare, class Allocator>
Bst<Key, T, Compare, Allocator>::Bst
    () :
    root_  (nullptr),
    size_  (0),
    alloc_ (),
    cmp_   ()
{
}

template<class Key, class T, class Compare, class Allocator>
Bst<Key, T, Compare, Allocator>::Bst
    (Bst const&)
{
    // TODO
}

template<class Key, class T, class Compare, class Allocator>
Bst<Key, T, Compare, Allocator>::Bst
    (Bst&& o) :
    root_  (std::exchange(o.root_, nullptr)),
    size_  (std::exchange(o.size_, 0)),
    alloc_ (std::move(o.alloc_)), // TODO
    cmp_   (std::move(o.cmp_))
{
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::operator=
    (Bst o) -> Bst&
{
    this->swap(o);
    return *this;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::swap
    (Bst& o) -> void
{
    using std::swap;
    swap(root_, o.root_);
    swap(size_, o.size_);
    swap(alloc_, o.alloc_); // TODO
    swap(cmp_, o.cmp_);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::size
    () const -> std::size_t
{
    return size_;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::ssize
    () const -> std::ptrdiff_t
{
    return static_cast<std::ptrdiff_t>(size_);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::empty
    () const -> bool
{
    return 0 == this->size();
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::begin
    () -> iterator
{
    return iterator(root_);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::end
    () -> iterator
{
    return iterator();
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::begin
    () const -> const_iterator
{
    return const_iterator(root_);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::end
    () const -> const_iterator
{
    return const_iterator();
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::cbegin
    () const -> const_iterator
{
    return const_iterator(root_);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::cend
    () const -> const_iterator
{
    return const_iterator();
}


// bst private api:

template<class Key, class T, class Compare, class Allocator>
template<class... Args>
auto Bst<Key, T, Compare, Allocator>::new_node
    (Args&&... as) -> BstNode*
{
    auto p = std::allocator_traits<allocator>::allocate(alloc_, 1);
    std::allocator_traits<allocator>::construct(
        alloc_,
        p,
        std::forward<Args>(as)...
    );
    return p;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::delete_node
    (BstNode* const p) -> void
{
    std::allocator_traits<allocator>::destroy(alloc_, p);
    std::allocator_traits<allocator>::deallocate(alloc_, p, 1);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::find_node
    (Key const& key) const -> BstNode*
{
    auto pos = root_;
    while (pos != nullptr)
    {
        auto const ord = this->compare(key, pos);
        switch (ord)
        {
        case Ordering::EQ:
            return pos;

        case Ordering::LT:
            pos = pos->left;
            break;

        case Ordering::GT:
            pos = pos->right;
            break;
        }
    }
    return nullptr;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::find_spot
    (Key const& key) const -> FindSpotResult
{
    auto parent = root_;
    auto sonp = static_cast<BstNode**>(nullptr);
    do
    {
        auto const ord = this->compare(key, parent);
        switch (ord)
        {
        case Ordering::EQ:
            return {parent, nullptr};

        case Ordering::LT:
            sonp = &parent->left_;
            break;

        case Ordering::GT:
            sonp = &parent->right_;
            break;
        }

        if (*sonp != nullptr)
        {
            parent = *sonp;
        }
    }
    while (*sonp != nullptr);
    return {parent, sonp};
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::compare
    (Key const& key, BstNode* const node) const -> Ordering
{
    if (cmp_(key, node_key(node)))
    {
        return Ordering::LT;
    }
    else if (cmp_(node_key(node), key))
    {
        return Ordering::GT;
    }
    else
    {
        return Ordering::EQ;
    }
}

template<class Key, class T, class Compare, class Allocator>
template<class... Args>
auto Bst<Key, T, Compare, Allocator>::try_insert
    (Key const& k, Args&&... as) -> std::pair<iterator, bool>
{
    if (this->empty())
    {
        root_ = this->new_node(std::forward<Args>(as)...);
        ++size_;
        return {root_, true};
    }

    auto const [parent, sonp] = this->find_spot(k);
    if (not sonp)
    {
        return {parent, false};
    }

    auto const node = this->new_node(std::forward<Args>(as)...);
    node->parent_ = parent;
    *sonp = node;
    ++size_;
    return {node, true};
}

template<class Key, class T, class Compare, class Allocator>
template<class K, class M>
auto Bst<Key, T, Compare, Allocator>::insert_or_assign_impl
    (K&& k, M&& m) -> std::pair<iterator, bool>
{
    auto [it, isIn] = this->try_insert(
        k,
        std::piecewise_construct,
        std::forward_as_tuple(std::forward<M>(k)),
        std::forward_as_tuple(std::forward<M>(m))
    );

    if (isIn)
    {
        return {it, true};
    }

    auto const node = it.current_;
    node_data(node) = std::forward<M>(m);
    return {it, false};
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::erase_node
    (BstNode* const node) -> BstNode*
{
    auto const n = next_in_order(node);
    this->extract_node(node);
    this->delete_node(node);
    --size_;
    return n;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::extract_node
    (BstNode* const node) -> void
{
    auto const d = node_degree(node);
    switch (d)
    {
    case 0:
        {
            if (is_left_son(node))
            {
                node->parent_->left_ = nullptr;
            }
            else if (is_right_son(node))
            {
                node->parent_->right_ = nullptr;
            }
            else
            {
                root_ = nullptr;
            }
        }
        break;

    case 1:
        {
            auto const son = node->left_ ? node->left_ : node->right_;
            if (is_left_son(node))
            {
                node->parent_->left_ = son;
            }
            else if (is_right_son(node))
            {
                node->parent_->right_ = son;
            }
            else
            {
                root_ = son;
            }
            son->parent_ = node->parent_;
        }
        break;

    case 2:
        {
            auto const next = leftmost(node->right_);
            this->extract_node(next);
            next->parent_ = node->parent_;
            next->left_ = node->left_;
            next->right_ = node->right_;

            if (node->left_)
            {
                node->left_->parent_ = next;
            }

            if (node->right_)
            {
                node->right_->parent_ = next;
            }

            if (is_left_son(node))
            {
                node->parent_->left_ = next;
            }
            else if (is_right_son(node))
            {
                node->parent_->right_ = next;
            }
            else
            {
                root_ = next;
            }
        }
        break;
    }
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::next_in_order
    (BstNode* node) -> BstNode*
{
    if (has_right_son(node))
    {
        return leftmost(node->right_);
    }
    else
    {
        while (is_right_son(node))
        {
            node = node->parent_;
        }
        return node->parent_;
    }
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::leftmost
    (BstNode* root) -> BstNode*
{
    while (has_left_son(root))
    {
        root = root->left_;
    }
    return root;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::is_left_son
    (BstNode* const n) -> bool
{
    return n->parent_ && n == n->parent_->left_;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::is_right_son
    (BstNode* const n) -> bool
{
    return n->parent_ && n == n->parent_->right_;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::is_root
    (BstNode* const n) -> bool
{
    return n->parent_;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::has_right_son
    (BstNode* const n) -> bool
{
    return n->right_;
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::has_left_son
    (BstNode* const n) -> bool
{
    return n->left_;
}

// bst::bst_node:

template<class Key, class T, class Compare, class Allocator>
template<class... Args>
Bst<Key, T, Compare, Allocator>::BstNode::BstNode
    (Args&&... as) :
    data_ (std::forward<Args>(as)...)
{
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::node_key
    (BstNode* const node) -> Key const&
{
    return std::get<0>(node->data_);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::node_data
    (BstNode* const node) -> T&
{
    return std::get<1>(node->data_);
}

template<class Key, class T, class Compare, class Allocator>
auto Bst<Key, T, Compare, Allocator>::node_degree
    (BstNode* const node) -> int
{
    auto d = 0;
    d += static_cast<int>(static_cast<bool>(node->left_));
    d += static_cast<int>(static_cast<bool>(node->right_));
    return d;
}

// bst::bst_iterator:

template<class Key, class T, class Compare, class Allocator>
template<bool IsConst>
Bst<Key, T, Compare, Allocator>::BstIterator<IsConst>::BstIterator
    (BstNode* const root) :
    current_ (leftmost(root))
{
}

template<class Key, class T, class Compare, class Allocator>
template<bool IsConst>
auto Bst<Key, T, Compare, Allocator>::BstIterator<IsConst>::operator*
    () const -> pair_t&
{
    return current_->data_;
}

template<class Key, class T, class Compare, class Allocator>
template<bool IsConst>
auto Bst<Key, T, Compare, Allocator>::BstIterator<IsConst>::operator++
    () -> BstIterator&
{
    current_ = next_in_order(current_);
    return *this;
}

template<class Key, class T, class Compare, class Allocator>
template<bool IsConst>
auto Bst<Key, T, Compare, Allocator>::BstIterator<IsConst>::operator==
    (BstIterator const& other) const -> bool
{
    return current_ == other.current_;
}

template<class Key, class T, class Compare, class Allocator>
template<bool IsConst>
auto Bst<Key, T, Compare, Allocator>::BstIterator<IsConst>::operator!=
    (BstIterator const& other) const -> bool
{
    return not (*this == other);
}
} // namespace idril

#endif