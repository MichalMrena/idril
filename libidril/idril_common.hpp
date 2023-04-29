
namespace idril::details
{
/**
 *  \brief Function object implementing operator<
 *  Implementation of \c std::less
 */
template<class T>
struct less
{
    [[nodiscard]]
    constexpr auto operator()
        (T const& l, T const& r) const noexcept -> bool
    {
        return l < r;
    }
};

/**
 *  \brief Provides member typedef based on the value of \p B
 *  Implementation of \c std::conditional
 */
template<bool B, class T, class F>
struct type_if;

/**
 *  \brief Specialization for B = true
 */
template<class T, class F>
struct type_if<true, T, F>
{
    using type = T;
};

/**
 *  \brief Specialization for B = false
 */
template<class T, class F>
struct type_if<false, T, F>
{
    using type = F;
};

/**
 *  \brief \p T of \p B = true, \p F otherwise
 *  Implementation of \c std::conditional_t
 */
template<bool B, class T, class F>
using type_if_t = typename type_if<B, T, F>::type;

/**
 *  \brief Tag for in-place construction
 *  Implementation of \c std::piecewise_construct
 */
struct PiecewiseConstructTag
{
};

// TODO move
// TODO forward
}