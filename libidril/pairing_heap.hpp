#ifndef LIBIDRIL_PAIRING_HEAP_HPP
#define LIBIDRIL_PAIRING_HEAP_HPP

#include "idril_common.hpp"
#include <memory>
#include <utility>

#include <iterator>
#include <stdexcept>

namespace idril
{
/**
 * \brief Merge mode options for PairingHeap.
 */
namespace merge_mode
{
// TODO explain
struct TwoPass
{
};

// TODO explain
struct FifoQueue
{
};

// TODO implement and explain
struct Hierarchical
{
};
} // namespace merge_mode

/**
 *  \brief PairingHeap forward declaration.
 */
template<
    class T,
    class Compare   = details::less<T>,
    class MergeMode = merge_mode::TwoPass,
    class Allocator = std::allocator<T>>
class PairingHeap;

/**
 *  \brief Node handle that is return after an insertion.
 */
class PairingHeapHandle
{
public:
    template<class T, class Compare, class MergeMode, class Allocator>
    friend class PairingHeap;

public:
    auto operator==(PairingHeapHandle const&) const -> bool = default;
    auto operator!=(PairingHeapHandle const&) const -> bool = default;

    PairingHeapHandle()                                     = default;

private:
    PairingHeapHandle(void* const node) : node_(node)
    {
    }

private:
    void* node_ {nullptr};
};

/**
 *  \brief Pairing heap represented by a binary tree.
 *  
 *  If A < B i.e., Compare()(A, B) == true then A has higher priority than B.
 *
 *  \tparam T          The type of the stored elements.
 *  \tparam Compare    A type providing a strict weak ordering.
 *  \tparam MergeMode  See the merge_mode namespace above.
 *  \tparam Allocator  Allocator for internal memory management.
 */
template<class T, class Compare, class MergeMode, class Allocator>
class PairingHeap
{
private:
    /**
     *  \brief Node of a tree that is used to represent the PairingHeap.
     */
    struct PairingNode
    {
        PairingNode(PairingNode const&) = delete;
        PairingNode(PairingNode&&)      = delete;

        template<class... Args>
        PairingNode(Args&&... args);

        T data_;
        PairingNode* parent_ {nullptr};
        PairingNode* left_ {nullptr};
        PairingNode* right_ {nullptr};
    };

public:
    /**
     *  \brief PairingHeap iterator.
     */
    template<bool IsConst>
    class PairingTreeIterator
    {
    public:
        using difference_type   = long long;
        using value_type        = details::type_if<IsConst, T const, T>;
        using pointer           = value_type*;
        using reference         = value_type&;
        using iterator_category = std::forward_iterator_tag;
        using node_t            = PairingNode;

    public:
        PairingTreeIterator() = default;
        PairingTreeIterator(node_t* const root);

        auto operator++() -> PairingTreeIterator&;
        auto operator++(int) -> PairingTreeIterator;
        auto operator*() const -> reference;
        auto operator->() const -> pointer;
        auto operator==(PairingTreeIterator const&) const -> bool = default;
        auto operator!=(PairingTreeIterator const&) const -> bool = default;

    private:
        friend class PairingHeap<T, Compare, MergeMode, Allocator>;

    private:
        node_t* current_ {nullptr};
    };

public:
    using handle_type       = PairingHeapHandle;
    using value_type        = T;
    using reference         = T&;
    using const_reference   = T const&;
    using size_type         = unsigned long long;
    using difference_type   = long long;
    using iterator          = PairingTreeIterator<false>;
    using const_iterator    = PairingTreeIterator<true>;
    using allocator_type    = Allocator;

public:
    /**
     *  \brief Default constructor
     *  \param alloc allocator
     */
    PairingHeap(Allocator const& alloc = Allocator());

    /**
     *  \brief Copy constructor
     *  \param other other heap to be copied
     */
    PairingHeap(PairingHeap const& other);

    /**
     *  \brief Move constructor
     *  \param other other heap to be moved from
     */
    PairingHeap(PairingHeap&& other) noexcept;

    /**
     *  \brief Destructor
     */
    ~PairingHeap();

    /**
     *  \brief Assignment operator
     *
     *  Serves both as copy and move assignment operator.
     *  The argument can be either copy-constructed or move-constructed.
     *
     *  \param other heap to assign into this one
     *  \return reference to this heap
     */
    auto operator=(PairingHeap other) noexcept -> PairingHeap&;

    /**
     *  \brief Inserts new element constructing it in-place from \p args
     *  \param args arguments from which the element will be constructed
     *  \return handle to the inserted element
     */
    template<class... Args>
    auto emplace(Args&&... args) -> handle_type;

    /**
     *  \brief Inserts new element copy-constructing it from \p value
     *  \param args element to be inserted
     *  \return handle to the inserted element
     */
    auto insert(value_type const& value) -> handle_type;

    /**
     *  \brief Inserts new element move-constructing it from \p value
     *  \param args element to be inserted
     *  \return handle to the inserted element
     */
    auto insert(value_type&& value) -> handle_type;

    /**
     *  \brief Removes the element with the highest priority
     */
    auto delete_min() -> void;

    /**
     *  \brief Accesses the element with the highest priority
     *  \return reference to the element with highest priority
     */
    auto find_min() -> reference;

    /**
     *  \brief Accesses the element with the highest priority
     *  \return reference to the element with highest priority
     */
    auto find_min() const -> const_reference;

    /**
     *  \brief Adjusts position of the element whose priority has increased
     *
     *  Behavior is undefined if the priority decreased!
     *
     *  \param handle handle pointing to the element with updated priority
     */
    auto decrease_key(handle_type handle) -> void;

    /**
     *  \brief Adjusts position of the element whose priority has increased
     *
     *  Behavior is undefined if the priority decreased!
     *
     *  \param pos iterator pointing to the element with updated priority
     */
    auto decrease_key(iterator pos) -> void;

    /**
     *  \brief Adjusts position of the element whose priority has increased
     *
     *  Behavior is undefined if the priority decreased!
     *
     *  \param pos iterator pointing to the element with updated priority
     */
    auto decrease_key(const_iterator pos) -> void;

    // TODO increase_key

    /**
     *  \brief Melds the other heap into this one
     *  \param other other heap to be melded into this one
     *  \return reference to this heap
     */
    auto meld(PairingHeap other) -> PairingHeap&;

    /**
     *  \brief Removes the element from the heap
     *  \param handle handle pointing to the element to be removed
     */
    auto erase(handle_type handle) -> void;

    /**
     *  \brief Removes the element from the heap
     *  \param pos iterator pointing to the element to be removed
     */
    auto erase(iterator pos) -> void;

    /**
     *  \brief Removes the element from the heap
     *  \param pos iterator pointing to the element to be removed
     */
    auto erase(const_iterator pos) -> void;

    /**
     *  \brief Swap this heap with the \p other
     *  \param other other heap to be swapped with this one
     */
    auto swap(PairingHeap& other) noexcept -> void;

    /**
     *  \brief Checks if the heap is empty
     *  \return bool value indication whether this heap is empty
     */
    auto empty() const -> bool;

    /**
     *  \brief Returns the number of elements in the heap
     *  \return the number of elements in the heap
     */
    auto size() const -> size_type;

    /**
     *  \brief Returns the number of elements in the heap
     *  \return the number of elements in the heap
     */
    auto ssize() const -> difference_type;

    /**
     *  \brief Removes all elements from the heap leaving it empty
     */
    auto clear() -> void;

    /**
     *  \brief Returns begin iterator
     *  \return begin iterator
     */
    auto begin() -> iterator;

    /**
     *  \brief Returns end iterator
     *  \return end iterator
     */
    auto end() -> iterator;

    /**
     *  \brief Returns begin iterator
     *  \return begin iterator
     */
    auto begin() const -> const_iterator;

    /**
     *  \brief Returns end iterator
     *  \return end iterator
     */
    auto end() const -> const_iterator;

    /**
     *  \brief Returns begin iterator
     *  \return begin iterator
     */
    auto cbegin() const -> const_iterator;

    /**
     *  \brief Returns end iterator
     *  \return end iterator
     */
    auto cend() const -> const_iterator;

    /**
     *  \brief Returns element that is pointed to by \p handle
     *  \return reference to the element associated with the handle
     */
    auto get_handle_data(handle_type handle) -> reference;

    /**
     *  \brief Returns element that is pointed to by \p handle
     *  \return reference to the element associated with the handle
     */
    auto get_handle_data(handle_type handle) const -> const_reference;

private:
    using node_t            = PairingNode;
    using type_alloc_traits = std::allocator_traits<Allocator>;
    using node_alloc_traits =
        typename type_alloc_traits::template rebind_traits<node_t>;
    using node_allocator =
        typename type_alloc_traits::template rebind_alloc<node_t>;

private:
    template<class... Args>
    auto new_node(Args&&... args) -> node_t*;
    auto copy_node(node_t* node) -> node_t*;
    auto delete_node(node_t* node) -> void;
    auto insert_impl(node_t* node) -> handle_type;
    auto empty_check() const -> void;
    auto deep_copy(node_t* root) -> node_t*;
    auto erase_impl(node_t* node) -> void;

    template<class Cmp = Compare>
    auto dec_key_impl(node_t* node) -> void;

    template<class... Args>
    auto new_node_impl(Args&&... args) -> node_t*;

    static auto handle_to_node(handle_type handle) -> node_t*;

    template<class Cmp = Compare>
    static auto pair(node_t* lhs, node_t* rhs) -> node_t*;
    static auto merge(node_t* first) -> node_t*;
    static auto merge(node_t* first, merge_mode::TwoPass) -> node_t*;
    static auto merge(node_t* first, merge_mode::FifoQueue) -> node_t*;
    static auto first_pass(node_t* first) -> node_t*;
    static auto second_pass(node_t* last) -> node_t*;

private:
    [[no_unique_address]]
    node_allocator alloc_;
    node_t* root_;
    size_type size_;
};

template<class T, class Compare, class MergeMode, class Allocator>
auto meld(
    PairingHeap<T, Compare, MergeMode, Allocator> lhs,
    PairingHeap<T, Compare, MergeMode, Allocator> rhs
) noexcept -> PairingHeap<T, Compare, MergeMode, Allocator>;

template<class T, class Compare, class MergeMode, class Allocator>
auto swap(
    PairingHeap<T, Compare, MergeMode, Allocator>& lhs,
    PairingHeap<T, Compare, MergeMode, Allocator>& rhs
) noexcept -> void;

/// definitions:

namespace details
{
struct AlwaysTrueCmp
{
    template<class T>
    [[nodiscard]] constexpr auto operator()(T const&, T const&) const noexcept
        -> bool
    {
        return true;
    }
};
} // namespace details

// PairingNode definition:

template<class T, class Compare, class MergeMode, class Allocator>
template<class... Args>
PairingHeap<T, Compare, MergeMode, Allocator>::PairingNode::PairingNode(
    Args&&... args
)
    : data_(std::forward<Args>(args)...), parent_(nullptr), left_(nullptr),
      right_(nullptr)
{
}

// PairingTreeIterator definition:

template<class T, class Compare, class MergeMode, class Allocator>
template<bool IsConst>
PairingHeap<T, Compare, MergeMode, Allocator>::PairingTreeIterator<
    IsConst>::PairingTreeIterator(node_t* const root)
    : current_(root)
{
    if (current_)
    {
        while (current_->left_)
        {
            current_ = current_->left_;
        }
    }
}

template<class T, class Compare, class MergeMode, class Allocator>
template<bool IsConst>
auto PairingHeap<T, Compare, MergeMode, Allocator>::PairingTreeIterator<
    IsConst>::operator++() -> PairingTreeIterator&
{
    if (current_->right_)
    {
        current_ = current_->right_;
        while (current_->left_)
        {
            current_ = current_->left_;
        }
    }
    else
    {
        while (current_->parent_ && current_->parent_->right_ == current_)
        {
            current_ = current_->parent_;
        }
        current_ = current_->parent_;
    }
    return *this;
}

template<class T, class Compare, class MergeMode, class Allocator>
template<bool IsConst>
auto PairingHeap<T, Compare, MergeMode, Allocator>::PairingTreeIterator<
    IsConst>::operator++(int) -> PairingTreeIterator
{
    auto const ret = *this;
    ++(*this);
    return ret;
}

template<class T, class Compare, class MergeMode, class Allocator>
template<bool IsConst>
auto PairingHeap<T, Compare, MergeMode, Allocator>::PairingTreeIterator<
    IsConst>::operator*() const -> reference
{
    return current_->data_;
}

template<class T, class Compare, class MergeMode, class Allocator>
template<bool IsConst>
auto PairingHeap<T, Compare, MergeMode, Allocator>::PairingTreeIterator<
    IsConst>::operator->() const -> pointer
{
    // TODO
    return std::addressof(current_->data_);
}

// pairing_heap definition:

template<class T, class Compare, class MergeMode, class Allocator>
PairingHeap<T, Compare, MergeMode, Allocator>::PairingHeap(
    Allocator const& alloc
)
    : alloc_(alloc), root_(nullptr), size_(0)
{
}

template<class T, class Compare, class MergeMode, class Allocator>
PairingHeap<T, Compare, MergeMode, Allocator>::PairingHeap(
    PairingHeap const& other
)
    : alloc_(other.alloc_), // TODO
      root_(this->deep_copy(other)), size_(other.size_)
{
}

template<class T, class Compare, class MergeMode, class Allocator>
PairingHeap<T, Compare, MergeMode, Allocator>::PairingHeap(PairingHeap&& other
) noexcept
    : alloc_(std::move(other.alloc_)), // TODO
      root_(std::exchange(other.root_, nullptr)),
      size_(std::exchange(other.size_, 0))
{
}

template<class T, class Compare, class MergeMode, class Allocator>
PairingHeap<T, Compare, MergeMode, Allocator>::~PairingHeap()
{
    auto* node = root_;
    if (node)
    {
        while (node->left_ || node->right_)
        {
            node = node->left_ ? node->left_ : node->right_;
        }
    }

    while (node)
    {
        auto* next = node->parent_ && node->parent_->right_ != node
                         ? node->parent_->right_
                         : nullptr;

        if (next != nullptr)
        {
            while (node->left_ || node->right_)
            {
                node = node->left_ ? node->left_ : node->right_;
            }
        }
        else
        {
            next = node->parent_;
        }
        this->delete_node(node);
        node = next;
    }

    root_ = nullptr;
    size_ = 0;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::operator=(PairingHeap other
) noexcept -> PairingHeap&
{
    swap(*this, other);
    return *this;
}

template<class T, class Compare, class MergeMode, class Allocator>
template<class... Args>
auto PairingHeap<T, Compare, MergeMode, Allocator>::emplace(Args&&... args)
    -> handle_type
{
    return this->insert_impl(this->new_node(std::forward<Args>(args)...));
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::insert(
    value_type const& value
) -> handle_type
{
    return this->insert_impl(this->new_node(value));
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::insert(value_type&& value)
    -> handle_type
{
    return this->insert_impl(this->new_node(std::move(value)));
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::delete_min() -> void
{
    this->empty_check();
    auto* const oldRoot = root_;

    if (1 == this->size())
    {
        root_ = nullptr;
    }
    else
    {
        root_->left_->parent_ = nullptr;
        root_                 = merge(root_->left_);
    }

    --size_;
    this->delete_node(oldRoot);
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::find_min() -> reference
{
    this->empty_check();
    return root_->data_;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::find_min() const
    -> const_reference
{
    this->empty_check();
    return root_->data_;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::decrease_key(
    handle_type const handle
) -> void
{
    this->dec_key_impl(handle_to_node(handle));
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::decrease_key(iterator pos)
    -> void
{
    this->dec_key_impl(pos.current_);
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::decrease_key(
    const_iterator pos
) -> void
{
    this->dec_key_impl(pos.current_);
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::meld(PairingHeap other)
    -> PairingHeap&
{
    if (not root_ && not other.root_)
    {
        return *this;
    }

    auto* const otherRoot = std::exchange(other.root_, nullptr);

    if (root_ && otherRoot)
    {
        root_ = pair(root_, otherRoot);
    }
    else if (not root_)
    {
        root_ = otherRoot;
    }

    size_ += std::exchange(other.size_, 0);
    return *this;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::erase(
    handle_type const handle
) -> void
{
    this->erase_impl(handle_to_node(handle));
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::erase(iterator pos) -> void
{
    this->erase_impl(pos.current());
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::erase(const_iterator pos)
    -> void
{
    this->erase_impl(pos.current());
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::swap(PairingHeap& other
) noexcept -> void
{
    using std::swap;
    swap(root_, other.root_);
    swap(size_, other.size_);

    if constexpr (node_alloc_traits::propagate_on_container_swap::value)
    {
        swap(alloc_, other.alloc_);
    }
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::empty() const -> bool
{
    return 0 == this->size();
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::size() const -> size_type
{
    return size_;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::ssize() const
    -> difference_type
{
    return static_cast<difference_type>(this->size());
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::clear() -> void
{
    *this = PairingHeap();
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::begin() -> iterator
{
    return iterator(root_);
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::end() -> iterator
{
    return iterator();
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::begin() const
    -> const_iterator
{
    return this->cbegin();
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::end() const
    -> const_iterator
{
    return this->cend();
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::cbegin() const
    -> const_iterator
{
    return const_iterator(root_);
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::cend() const
    -> const_iterator
{
    return const_iterator();
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::get_handle_data(
    handle_type handle
) -> reference
{
    return handle_to_node(handle)->data_;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::get_handle_data(
    handle_type handle
) const -> const_reference
{
    return handle_to_node(handle)->data_;
}

template<class T, class Compare, class MergeMode, class Allocator>
template<class... Args>
auto PairingHeap<T, Compare, MergeMode, Allocator>::new_node(Args&&... args)
    -> node_t*
{
    return this->new_node_impl(std::forward<Args>(args)...);
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::copy_node(node_t* const node
) -> node_t*
{
    return this->new_node_impl(node->data_);
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::delete_node(
    node_t* const node
) -> void
{
    if (node)
    {
        node_alloc_traits::destroy(alloc_, node);
        node_alloc_traits::deallocate(alloc_, node, 1);
    }
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::insert_impl(
    node_t* const node
) -> handle_type
{
    root_ = this->empty() ? node : pair(root_, node);
    ++size_;
    return handle_type(node);
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::empty_check() const -> void
{
    if (this->empty())
    {
        throw std::out_of_range("Heap is empty!");
    }
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::deep_copy(node_t* srcNode)
    -> node_t*
{
    if (not srcNode)
    {
        return nullptr;
    }

    auto* const newRoot = this->copy_node(srcNode);
    auto* newNode       = newRoot;

    while (srcNode)
    {
        if (srcNode->left_ && not newNode->left_)
        {
            newNode->left_ = this->copy_node(srcNode->left_);
            srcNode        = srcNode->left_;
            newNode        = newNode->left_;
        }
        else if (srcNode->right_ && not newNode->right_)
        {
            newNode->right_ = this->copy_node(srcNode->right_);
            srcNode         = srcNode->right_;
            newNode         = newNode->right_;
        }
        else
        {
            srcNode = srcNode->parent_;
            newNode = newNode->parent_;
        }
    }

    return newRoot;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::erase_impl(
    node_t* const node
) -> void
{
    this->dec_key_impl<details::AlwaysTrueCmp>(node);
    this->delete_min();
}

template<class T, class Compare, class MergeMode, class Allocator>
template<class Cmp>
auto PairingHeap<T, Compare, MergeMode, Allocator>::dec_key_impl(
    node_t* const node
) -> void
{
    if (node == root_)
    {
        return;
    }

    if (node->parent_ && node == node->parent_->left_)
    {
        node->parent_->left_ = node->right_;
    }
    else
    {
        node->parent_->right_ = node->right_;
    }

    if (node->right_)
    {
        node->right_->parent_ = node->parent_;
    }

    node->parent_ = nullptr;
    node->right_  = nullptr;

    root_         = pair<Cmp>(node, root_);
}

template<class T, class Compare, class MergeMode, class Allocator>
template<class... Args>
auto PairingHeap<T, Compare, MergeMode, Allocator>::new_node_impl(Args&&... args
) -> node_t*
{
    auto const p = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, p, std::forward<Args>(args)...);
    return p;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::handle_to_node(
    handle_type const handle
) -> node_t*
{
    return static_cast<node_t*>(handle.node_);
}

template<class T, class Compare, class MergeMode, class Allocator>
template<class Cmp>
auto PairingHeap<T, Compare, MergeMode, Allocator>::pair(
    node_t* const lhs, node_t* const rhs
) -> node_t*
{
    auto const areOrdered  = Cmp()(lhs->data_, rhs->data_);
    auto* const parent     = areOrdered ? lhs : rhs;
    auto* const son        = areOrdered ? rhs : lhs;
    auto* const oldLeftSon = parent->left_;

    son->parent_           = parent;
    son->right_            = oldLeftSon;
    parent->left_          = son;

    if (oldLeftSon)
    {
        oldLeftSon->parent_ = son;
    }

    return parent;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::merge(node_t* const first)
    -> node_t*
{
    return merge(first, MergeMode());
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::merge(
    node_t* const first, merge_mode::TwoPass
) -> node_t*
{
    return second_pass(first_pass(first));
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::merge(
    node_t* first, merge_mode::FifoQueue
) -> node_t*
{
    // Use parent pointers to make the queue
    auto* head    = first;
    auto* last    = first;
    first         = first->right_;
    head->parent_ = nullptr;
    head->right_  = nullptr;

    auto* next    = static_cast<node_t*>(nullptr);
    while (first)
    {
        next           = first->right_;
        first->right_  = nullptr;
        first->parent_ = head;
        head           = first;
        first          = next;
    }

    // Perform fifo merge
    while (head && head->parent_)
    {
        auto* const lhs = head;
        auto* const rhs = head->parent_;
        head            = head->parent_->parent_;
        lhs->parent_    = nullptr;
        rhs->parent_    = nullptr;
        last->parent_   = pair(lhs, rhs);
        last            = last->parent_;
    }

    return last;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::first_pass(node_t* first)
    -> node_t*
{
    auto* prev   = static_cast<node_t*>(nullptr);
    auto* paired = static_cast<node_t*>(nullptr);
    auto* next   = first;
    auto* second = first->right_;

    for (;;)
    {
        first = next;
        if (not first)
        {
            return prev;
        }

        second = first->right_;
        if (second)
        {
            next            = second->right_;
            first->right_   = nullptr;
            first->parent_  = nullptr;
            second->right_  = nullptr;
            second->parent_ = nullptr;
            paired          = pair(first, second);
        }
        else
        {
            paired = first;
            next   = nullptr;
        }

        if (prev)
        {
            prev->right_    = paired;
            paired->parent_ = prev;
        }
        prev = paired;

        if (next)
        {
            next->parent_ = nullptr;
        }
        else
        {
            return prev;
        }
    }

    return prev; // TODO toto môže byť null, je to v poriadku?
}

template<class T, class Compare, class MergeMode, class Allocator>
auto PairingHeap<T, Compare, MergeMode, Allocator>::second_pass(node_t* last)
    -> node_t*
{
    auto* const parent = last->parent_;
    last->parent_      = nullptr;

    while (parent)
    {
        auto const next = parent->parent_;
        parent->right_  = nullptr;
        parent->parent_ = nullptr;
        last            = pair(last, parent);
        parent          = next;
    }

    return last;
}

template<class T, class Compare, class MergeMode, class Allocator>
auto meld(
    PairingHeap<T, Compare, MergeMode, Allocator> lhs,
    PairingHeap<T, Compare, MergeMode, Allocator> rhs
) noexcept -> PairingHeap<T, Compare, MergeMode, Allocator>
{
    lhs.meld(std::move(rhs));
    return PairingHeap<T, Compare, MergeMode, Allocator>(std::move(lhs));
}

template<class T, class Compare, class MergeMode, class Allocator>
auto swap(
    PairingHeap<T, Compare, MergeMode, Allocator>& lhs,
    PairingHeap<T, Compare, MergeMode, Allocator>& rhs
) noexcept -> void
{
    lhs.swap(rhs);
}
} // namespace idril

#endif