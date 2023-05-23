#ifndef LIBIDRIL_TWO_LISTS_HPP
#define LIBIDRIL_TWO_LISTS_HPP

namespace idril
{
    template<class T, class Compare, class Allocator>
    class TwoLists
    {
    private:
        struct Item
        {
            Item(Item const&) = delete;
            Item(Item&&)      = delete;

            template<class... Args>
            Item(Args&&... args);

            T data_;
            Item* next_;
        };

    public:
        using size_type       = unsigned long long;
        using difference_type = long long;

    public:


    private:
        size_type shortListSize_;
        size_type shortListCapacity_;
        size_type longListCapacity_;
        Item** shortList_;
        Item* longList_;
    };
}

#endif