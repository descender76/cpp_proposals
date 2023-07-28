<style type="text/css">
ins { background-color: #CCFFCC }
s { background-color: #FFCACA }
blockquote { color: inherit !important }
</style>

<table>
<tr>
<td>Document number</td>
<td>P2955R0</td>
</tr>
<tr>
<td>Date</td>
<td>2023-7-28</td>
</tr>
<tr>
<td>Reply-to</td>
<td>

Jarrad J. Waterloo &lt;descender76 at gmail dot com&gt;

</td>
</tr>
<tr>
<td>Audience</td>
<td>SG23 Safety and Security</td>
</tr>
</table>

# Safer Range Access

<style>
.inline-link
{
    font-size: small;
    margin-top: -2.8em;
    margin-right: 4px;
    text-align: right;
    font-weight: bold;
}

code
{
    font-family: "Fira Code", monospace !important;
    font-size: 0.87em;
}

.sourceCode
{
    font-size: 0.95em;
}

a code
{
    color: #0645ad;
}
</style>

## Table of contents

- [Safer Range Access](#Safer-Range-Access)
  - [Abstract](#Abstract)
  - [Technical Details](#Technical-Details)
    - [The common challenges](#The-common-challenges)
    - [The common safe member functions](#The-common-safe-member-functions)
    - [The common safe free functions](#The-common-safe-free-functions)
  - [Example](#Example)
  - [Resolution](#Resolution)
  - [Summary](#Summary)
  - [Frequently Asked Questions](#Frequently-Asked-Questions)
  - [References](#References)

## Abstract

Currently the element access of `std::array`, `std::deque`, `std::span`, `std::string`, `std::string_view`, `std::vector` is not as safe as it could be. While the `at` member functions are checked for range access errors, the `operator[]`, `front` and `back` are not checked. That means the majority of the element access is unchecked. Further, even though the rarely used `at` is checked, like the others, it returns a reference which, besides exposing implementation details, can lead to dangling and reference invalidation errors.

This paper proposes that we provide programmers with functions that have value semantics in order to not only mitigate range access errors but also to reduce the dangling and reference invalidation errors.

## Technical Details

Feel free to jump to the end for the [example](#Example) code.

These technical details will focus on `vector` but everything can be easily applied to the other containers and views with range based access. The first method to be added is a `test` function since only a given implementation can say whether an index is valid.

```cpp
[[nodiscard]]
[[safe]]
constexpr bool test( std::vector<T>::size_type pos ) const;
```

The next two methods requested for addition are actually `unsafe`. Though `unsafe`, these will be the foundation for which all of the `safe` functions will be built upon.

```cpp
[[nodiscard]]
[[unsafe(reason=["range", "dangling_reference", "reference_invalidation"])]]
constexpr reference get_reference( size_type pos ) noexcept;
[[nodiscard]]
[[unsafe(reason=["range", "dangling_reference", "reference_invalidation"])]]
constexpr const_reference get_reference( size_type pos ) const noexcept;
```

It should be noted that these functions are just as `unsafe` as the current subscript operators. The difference is while the subscript operators has undefined unsafety, the proposed `get_reference` deliberately perform no checks as the checks will be performed either by the programmer or by `safe` functions that call these `unsafe` ones. This way when `contracts` arrive and when the undefined behavior is changed to defined behavior, the `safe` functions won't be performing an additional superfluous test.

```cpp
[[nodiscard]]
[[unsafe(reason=["range", "dangling_reference", "reference_invalidation"])]]
constexpr reference operator[]( size_type pos );
[[nodiscard]]
[[unsafe(reason=["range", "dangling_reference", "reference_invalidation"])]]
constexpr const_reference operator[]( size_type pos ) const;
// ...
[[nodiscard]]
[[unsafe(reason=["dangling_reference", "reference_invalidation"])]]
constexpr reference at( size_type pos );
[[nodiscard]]
[[unsafe(reason=["dangling_reference", "reference_invalidation"])]]
constexpr const_reference at( size_type pos ) const;
```

### The common challenges

1. `C++`'s subscript operators tend to have reference semantics instead of value semantics because our language does not have seperate operators for getting and setting values.
    1.  `C++`'s subscript operators does not support additional parameters such as `std::source_location` which are not part of the indexer.
1. `std::optional`, `std::variant` and `std::expected` does not support references

Consequently, this proposal just use functions instead of any operators and transient proxy classes. While I prefer `deducing this` [^p0847r7] style member functions since these won't be overloaded, as most compilers currently do not support such feature, the examples will be regular member functions.

### The common safe member functions

Following are the function names of all the requested member functions. They are represented in table fashion to better illustrate the relationships between all of these variants of getters and setters.

1. `operator[]`/`at`, `front` and `back` get seperate getter and setter functions that take and return values.
1. The getter and setter variants gets multiplied by how programmers handle errors.
    - throw exception
    - return default
    - return optional
    - terminate
    - do nothing i.e. void i.e. crop
    - grow

<table>
<tr>
<th></th>
<th></th>
<th>front</th>
<th>back</th>
</tr>
<tr>
<th rowspan="3">get</th>
<td>get_value</td>
<td>get_front_value</td>
<td>get_back_value</td>
</tr>
<tr>
<td>get_optional</td>
<td>get_front_optional</td>
<td>get_back_optional</td>
</tr>
<tr>
<td>get_or_terminate</td>
<td>get_front_or_terminate</td>
<td>get_back_or_terminate</td>
</tr>
<tr>
<th rowspan="4">set</th>
<td>set_value</td>
<td>set_front_value</td>
<td>set_back_value</td>
</tr>
<tr>
<td>set_or_terminate</td>
<td>set_front_or_terminate</td>
<td>set_back_or_terminate</td>
</tr>
<tr>
<td>set_and_crop</td>
<td>set_front_and_crop</td>
<td>set_back_and_crop</td>
</tr>
<tr>
<td>set_and_grow</td>
<td>set_front_and_grow</td>
<td>set_back_and_grow</td>
</tr>
</table>

Returning references don't just allow the programmer to get and set the whole value but also modify a part of the indexed value or call some member function. In order to preserve that capability, the previous get/set table gets duplicated with a transform/visit versions. Like the value based get's above, the transform member functions returns a subset of the index value as a value using a transformer object. Like the value based set's above, the visit member functions are safe because they deal with values and always return void instead of a reference. Its provided copier objects performs a programmer decided action to the indexed value.

<table>
<tr>
<th></th>
<th></th>
<th>front</th>
<th>back</th>
</tr>
<tr>
<th rowspan="3">transform</th>
<td>transform_value</td>
<td>transform_front_value</td>
<td>transform_back_value</td>
</tr>
<tr>
<td>transform_optional</td>
<td>transform_front_optional</td>
<td>transform_back_optional</td>
</tr>
<tr>
<td>transform_or_terminate</td>
<td>transform_front_or_terminate</td>
<td>transform_back_or_terminate</td>
</tr>
<tr>
<th rowspan="4">visit</th>
<td>visit_value</td>
<td>visit_front_value</td>
<td>visit_back_value</td>
</tr>
<tr>
<td>visit_or_terminate</td>
<td>visit_front_or_terminate</td>
<td>visit_back_or_terminate</td>
</tr>
<tr>
<td>visit_and_crop</td>
<td>visit_front_and_crop</td>
<td>visit_back_and_crop</td>
</tr>
<tr>
<td>visit_and_grow</td>
<td>visit_front_and_grow</td>
<td>visit_back_and_grow</td>
</tr>
</table>

It would be preferable that all of these member functions in these two tables were actually free functions, if and only if we get the `pipe redirect operator` [^p2011r1] [^p2672r0]. This would save on have to specify all of the functions repeatedly on the `std::array`, `std::deque`, `std::span`, `std::string`, `std::string_view`, `std::vector` classes. The complete list of these member functions from the perspective of `std::vector` follows.

```cpp
// at and operator[] replacements

[[nodiscard]]
[[safe]]
constexpr value_type get_value( size_type pos, const std::source_location location = std::source_location::current() ) const;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_value( size_type pos, const F& transformer, const std::source_location location = std::source_location::current() ) const;

[[nodiscard]]
[[safe]]
constexpr value_type get_value( size_type pos, const_reference default_value ) const noexcept;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_value( size_type pos, const F& transformer, const auto& default_value ) const noexcept;

[[nodiscard]]
[[safe]]
constexpr std::optional<value_type> get_optional( size_type pos ) const noexcept;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_optional( size_type pos, const F& transformer ) const noexcept;

[[nodiscard]]
[[safe]]
constexpr value_type get_or_terminate( size_type pos, const std::source_location location = std::source_location::current() ) const noexcept;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_or_terminate( size_type pos, const F& transformer, const std::source_location location = std::source_location::current() ) const noexcept;

[[safe]]
constexpr void set_value( size_type pos, const_reference value, const std::source_location location = std::source_location::current() );

template<class F>
[[safe]]
constexpr void visit_value( size_type pos, const F& copier, const std::source_location location = std::source_location::current() );

[[safe]]
constexpr void set_or_terminate( size_type pos, const_reference value, const std::source_location location = std::source_location::current() ) noexcept;

template<class F>
[[safe]]
constexpr void visit_or_terminate( size_type pos, const F& copier, const std::source_location location = std::source_location::current() ) noexcept;

[[safe]]
constexpr void set_and_crop( size_type pos, const_reference value ) noexcept;

template<class F>
[[safe]]
constexpr void visit_and_crop( size_type pos, const F& copier ) noexcept;

[[safe]]
constexpr void set_and_grow( size_type pos, const_reference value );

template<class F>
[[safe]]
constexpr void visit_and_grow( size_type pos, const F& copier );

[[safe]]
constexpr void set_and_grow( size_type pos, const_reference value, const_reference default_value );

template<class F>
[[safe]]
constexpr void visit_and_grow( size_type pos, const F& copier, const_reference default_value );

// front replacements

[[nodiscard]]
[[safe]]
constexpr value_type get_front_value( const std::source_location location = std::source_location::current() ) const;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_front_value( const F& transformer, const std::source_location location = std::source_location::current() ) const;

[[nodiscard]]
[[safe]]
constexpr value_type get_front_value( const_reference default_value ) const noexcept;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_front_value( const F& transformer, const auto& default_value ) const noexcept;

[[nodiscard]]
[[safe]]
constexpr std::optional<value_type> get_front_optional() const noexcept;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_front_optional( const F& transformer ) const noexcept;

[[nodiscard]]
[[safe]]
constexpr value_type get_front_or_terminate( const std::source_location location = std::source_location::current() ) const noexcept;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_front_or_terminate( const F& transformer, const std::source_location location = std::source_location::current() ) const noexcept;

[[safe]]
constexpr void set_front_value( const_reference value, const std::source_location location = std::source_location::current() );

template<class F>
[[safe]]
constexpr void visit_front_value( const F& copier, const std::source_location location = std::source_location::current() );

[[safe]]
constexpr void set_front_or_terminate( const_reference value, const std::source_location location = std::source_location::current() ) noexcept;

template<class F>
[[safe]]
constexpr void visit_front_or_terminate( const F& copier, const std::source_location location = std::source_location::current() ) noexcept;

[[safe]]
constexpr void set_front_and_crop( const_reference value ) noexcept;

template<class F>
[[safe]]
constexpr void visit_front_and_crop( const F& copier ) noexcept;

[[safe]]
constexpr void set_front_and_grow( const_reference value );

template<class F>
[[safe]]
constexpr void visit_front_and_grow( const F& copier );

[[safe]]
constexpr void set_front_and_grow( const_reference value, const_reference default_value );

template<class F>
[[safe]]
constexpr void visit_front_and_grow( const F& copier, const_reference default_value );

// back replacements
[[nodiscard]]
[[safe]]
constexpr value_type get_back_value( const std::source_location location = std::source_location::current() ) const;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_back_value( const F& transformer, const std::source_location location = std::source_location::current() ) const;

[[nodiscard]]
[[safe]]
constexpr value_type get_back_value( const_reference default_value ) const noexcept;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_back_value( const F& transformer, const auto& default_value ) const noexcept;

[[nodiscard]]
[[safe]]
constexpr std::optional<value_type> get_back_optional() const noexcept;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_back_optional( const F& transformer ) const noexcept;

[[nodiscard]]
[[safe]]
constexpr value_type get_back_or_terminate( const std::source_location location = std::source_location::current() ) const noexcept;

template<class F>
[[nodiscard]]
[[safe]]
constexpr auto transform_back_or_terminate( const F& transformer, const std::source_location location = std::source_location::current() ) const noexcept;

[[safe]]
constexpr void set_back_value( const_reference value, const std::source_location location = std::source_location::current() );

template<class F>
[[safe]]
constexpr void visit_back_value( const F& copier, const std::source_location location = std::source_location::current() );

[[safe]]
constexpr void set_back_or_terminate( const_reference value, const std::source_location location = std::source_location::current() ) noexcept;

template<class F>
[[safe]]
constexpr void visit_back_or_terminate( const F& copier, const std::source_location location = std::source_location::current() ) noexcept;

[[safe]]
constexpr void set_back_and_crop( const_reference value ) noexcept;

template<class F>
[[safe]]
constexpr void visit_back_and_crop( const F& copier ) noexcept;

[[safe]]
constexpr void set_back_and_grow( const_reference value );

template<class F>
[[safe]]
constexpr void visit_back_and_grow( const F& copier );

[[safe]]
constexpr void set_back_and_grow( const_reference value, const_reference default_value );

template<class F>
[[safe]]
constexpr void visit_back_and_grow( const F& copier, const_reference default_value );
```

### The common safe free functions

Besides these member functions are a couple of safe free functions.

```cpp
template<class T>
constexpr void copy_and_crop( std::span<T> destination, std::size_t pos, std::span<const T> source ) noexcept;

template<class T>
constexpr void copy_and_grow( std::vector<T>& destination, std::size_t pos, std::span<const T> source ) noexcept;
```

Besides being safe for not returning a reference, these two functions perform batch copies, reducing the number of individual checks that have to be performed. Both batch copy a source span to a destination range based collection or view. In the case of `copy_and_crop`, any elements that are outside the range of the destination do not get copied. In the case of `copy_and_grow`, the destination range based collection grows to accommodate the elements of the source span that exceeds the destinations current bounds. These two functions are analogous to combining two dimensional arrays such as images.

## Example

```cpp
#include <string>
#include <vector>
#include <iostream>
#include <source_location>
#include <optional>
#include <algorithm>
#include <span>
#include <cassert>
#include <sstream>

using namespace std::literals::string_literals;

// currently 12 unsafe element access methods
// proposed  14 unsafe element access methods, 56 safe element access methods/functions
template<class T>
class my_vector : public std::vector<T> {
private:
    static std::string to_string(const std::vector<T>::size_type pos, const std::source_location location)
    {
        std::stringstream ss;
        ss << "file: "
            << location.file_name() << '('
            << location.line() << ':'
            << location.column() << ") `"
            << location.function_name() << "`: "
            << "invalid index: " << pos << '\n';
        return ss.str();
    }

    static void log(const std::vector<T>::size_type pos, const std::source_location location)
    {
        std::clog << "file: "
            << location.file_name() << '('
            << location.line() << ':'
            << location.column() << ") `"
            << location.function_name() << "`: "
            << "invalid index: " << pos << '\n';
    }
public:
    using std::vector<T>::vector;
    typedef std::vector<T>::value_type value_type;
    typedef std::vector<T>::size_type size_type;
    typedef std::vector<T>::reference reference;
    typedef std::vector<T>::const_reference const_reference;

    [[nodiscard]]
    //[[safe]]
    constexpr bool test( std::vector<T>::size_type pos ) const
    {
        return pos < this->size();
    }

    [[nodiscard]]
    //[[unsafe]]
    constexpr reference get_reference( size_type pos ) noexcept
    {
        return (*this)[pos];
    }

    [[nodiscard]]
    //[[unsafe]]
    constexpr const_reference get_reference( size_type pos ) const noexcept
    {
        return (*this)[pos];
    }
    // at and operator[] replacements
    [[nodiscard]]
    //[[safe]]
    constexpr value_type get_value( size_type pos, const std::source_location location = std::source_location::current() ) const
    {
        if(test(pos))
        {
            return get_reference(pos);
        }
        else
        {
            throw std::out_of_range(to_string(pos, location));
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_value( size_type pos, const F& transformer, const std::source_location location = std::source_location::current() ) const
    {
        if(test(pos))
        {
            return transformer(get_reference(pos));
        }
        else
        {
            throw std::out_of_range(to_string(pos, location));
        }
    }

    [[nodiscard]]
    //[[safe]]
    constexpr value_type get_value( size_type pos, const_reference default_value ) const noexcept
    {
        if(test(pos))
        {
            return get_reference(pos);
        }
        else
        {
            return default_value;
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_value( size_type pos, const F& transformer, const auto& default_value ) const noexcept
    {
        if(test(pos))
        {
            return transformer(get_reference(pos));
        }
        else
        {
            return default_value;
        }
    }

    [[nodiscard]]
    //[[safe]]
    constexpr std::optional<value_type> get_optional( size_type pos ) const noexcept
    {
        if(test(pos))
        {
            return std::optional<value_type>{get_reference(pos)};
        }
        else
        {
            return std::nullopt;
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_optional( size_type pos, const F& transformer ) const noexcept
    {
        if(test(pos))
        {
            return std::optional<decltype(transformer(get_reference(pos)))>{transformer(get_reference(pos))};
        }
        else
        {
            return std::optional<decltype(transformer(get_reference(pos)))>{};
        }
    }

    [[nodiscard]]
    //[[safe]]
    constexpr value_type get_or_terminate( size_type pos, const std::source_location location = std::source_location::current() ) const noexcept
    {
        if(test(pos))
        {
            return get_reference(pos);
        }
        else
        {
            log(pos, location);
            std::terminate();
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_or_terminate( size_type pos, const F& transformer, const std::source_location location = std::source_location::current() ) const noexcept
    {
        if(test(pos))
        {
            return transformer(get_reference(pos));
        }
        else
        {
            log(pos, location);
            std::terminate();
        }
    }

    //[[safe]]
    constexpr void set_value( size_type pos, const_reference value, const std::source_location location = std::source_location::current() )
    {
        if(test(pos))
        {
            get_reference(pos) = value;
        }
        else
        {
            throw std::out_of_range(to_string(pos, location));
        }
    }

    template<class F>
    //[[safe]]
    constexpr void visit_value( size_type pos, const F& copier, const std::source_location location = std::source_location::current() )
    {
        if(test(pos))
        {
            copier(get_reference(pos));
        }
        else
        {
            throw std::out_of_range(to_string(pos, location));
        }
    }

    //[[safe]]
    constexpr void set_or_terminate( size_type pos, const_reference value, const std::source_location location = std::source_location::current() ) noexcept
    {
        if(test(pos))
        {
            get_reference(pos) = value;
        }
        else
        {
            log(pos, location);
            std::terminate();
        }
    }

    template<class F>
    //[[safe]]
    constexpr void visit_or_terminate( size_type pos, const F& copier, const std::source_location location = std::source_location::current() ) noexcept
    {
        if(test(pos))
        {
            copier(get_reference(pos));
        }
        else
        {
            log(pos, location);
            std::terminate();
        }
    }

    // https://en.wikipedia.org/wiki/Cropping_(image)
    //[[safe]]
    constexpr void set_and_crop( size_type pos, const_reference value ) noexcept
    {
        if(test(pos))
        {
            get_reference(pos) = value;
        }
    }

    template<class F>
    constexpr void visit_and_crop( size_type pos, const F& copier ) noexcept
    {
        if(test(pos))
        {
            copier(get_reference(pos));
        }
    }

    //[[safe]]
    constexpr void set_and_grow( size_type pos, const_reference value )
    {
        this->resize(std::max(this->size(), pos + 1));
        get_reference(pos) = value;
    }

    template<class F>
    //[[safe]]
    constexpr void visit_and_grow( size_type pos, const F& copier )
    {
        this->resize(std::max(this->size(), pos + 1));
        copier(get_reference(pos));
    }

    //[[safe]]
    constexpr void set_and_grow( size_type pos, const_reference value, const_reference default_value )
    {
        this->resize(std::max(this->size(), pos + 1), default_value);
        get_reference(pos) = value;
    }

    template<class F>
    //[[safe]]
    constexpr void visit_and_grow( size_type pos, const F& copier, const_reference default_value )
    {
        this->resize(std::max(this->size(), pos + 1), default_value);
        copier(get_reference(pos));
    }
    // front replacements
    [[nodiscard]]
    //[[safe]]
    constexpr value_type get_front_value( const std::source_location location = std::source_location::current() ) const
    {
        if(this->empty())
        {
            throw std::out_of_range(to_string(0, location));
        }
        else
        {
            return get_reference(0);
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_front_value( const F& transformer, const std::source_location location = std::source_location::current() ) const
    {
        if(this->empty())
        {
            throw std::out_of_range(to_string(0, location));
        }
        else
        {
            return transformer(get_reference(0));
        }
    }

    [[nodiscard]]
    //[[safe]]
    constexpr value_type get_front_value( const_reference default_value ) const noexcept
    {
        if(this->empty())
        {
            return default_value;
        }
        else
        {
            return get_reference(0);
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_front_value( const F& transformer, const auto& default_value ) const noexcept
    {
        if(this->empty())
        {
            return default_value;
        }
        else
        {
            return transformer(get_reference(0));
        }
    }

    [[nodiscard]]
    //[[safe]]
    constexpr std::optional<value_type> get_front_optional() const noexcept
    {
        if(this->empty())
        {
            return std::nullopt;
        }
        else
        {
            return std::optional<value_type>{get_reference(0)};
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_front_optional( const F& transformer ) const noexcept
    {
        if(this->empty())
        {
            return std::optional<decltype(transformer(get_reference(0)))>{};
        }
        else
        {
            return std::optional<decltype(transformer(get_reference(0)))>{transformer(get_reference(0))};
        }
    }

    [[nodiscard]]
    //[[safe]]
    constexpr value_type get_front_or_terminate( const std::source_location location = std::source_location::current() ) const noexcept
    {
        if(this->empty())
        {
            log(0, location);
            std::terminate();
        }
        else
        {
            return get_reference(0);
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_front_or_terminate( const F& transformer, const std::source_location location = std::source_location::current() ) const noexcept
    {
        if(this->empty())
        {
            log(0, location);
            std::terminate();
        }
        else
        {
            return transformer(get_reference(0));
        }
    }

    //[[safe]]
    constexpr void set_front_value( const_reference value, const std::source_location location = std::source_location::current() )
    {
        if(this->empty())
        {
            throw std::out_of_range(to_string(0, location));
        }
        else
        {
            get_reference(0) = value;
        }
    }

    template<class F>
    //[[safe]]
    constexpr void visit_front_value( const F& copier, const std::source_location location = std::source_location::current() )
    {
        if(this->empty())
        {
            throw std::out_of_range(to_string(0, location));
        }
        else
        {
            copier(get_reference(0));
        }
    }

    //[[safe]]
    constexpr void set_front_or_terminate( const_reference value, const std::source_location location = std::source_location::current() ) noexcept
    {
        if(this->empty())
        {
            log(0, location);
            std::terminate();
        }
        else
        {
            get_reference(0) = value;
        }
    }

    template<class F>
    //[[safe]]
    constexpr void visit_front_or_terminate( const F& copier, const std::source_location location = std::source_location::current() ) noexcept
    {
        if(this->empty())
        {
            log(0, location);
            std::terminate();
        }
        else
        {
            copier(get_reference(0));
        }
    }

    // https://en.wikipedia.org/wiki/Cropping_(image)
    //[[safe]]
    constexpr void set_front_and_crop( const_reference value ) noexcept
    {
        if(!this->empty())
        {
            get_reference(0) = value;
        }
    }

    template<class F>
    constexpr void visit_front_and_crop( const F& copier ) noexcept
    {
        if(!this->empty())
        {
            copier(get_reference(0));
        }
    }

    //[[safe]]
    constexpr void set_front_and_grow( const_reference value )
    {
        this->resize(std::max(this->size(), 1ul));
        get_reference(0) = value;
    }

    template<class F>
    //[[safe]]
    constexpr void visit_front_and_grow( const F& copier )
    {
        this->resize(std::max(this->size(), 1ul));
        copier(get_reference(0));
    }

    //[[safe]]
    constexpr void set_front_and_grow( const_reference value, const_reference default_value )
    {
        this->resize(std::max(this->size(), 1ul), default_value);
        get_reference(0) = value;
    }

    template<class F>
    //[[safe]]
    constexpr void visit_front_and_grow( const F& copier, const_reference default_value )
    {
        this->resize(std::max(this->size(), 1ul), default_value);
        copier(get_reference(0));
    }
    // back replacements
    [[nodiscard]]
    //[[safe]]
    constexpr value_type get_back_value( const std::source_location location = std::source_location::current() ) const
    {
        if(this->empty())
        {
            throw std::out_of_range(to_string(0, location));
        }
        else
        {
            return get_reference(this->size() - 1);
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_back_value( const F& transformer, const std::source_location location = std::source_location::current() ) const
    {
        if(this->empty())
        {
            throw std::out_of_range(to_string(0, location));
        }
        else
        {
            return transformer(get_reference(this->size() - 1));
        }
    }

    [[nodiscard]]
    //[[safe]]
    constexpr value_type get_back_value( const_reference default_value ) const noexcept
    {
        if(this->empty())
        {
            return default_value;
        }
        else
        {
            return get_reference(this->size() - 1);
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_back_value( const F& transformer, const auto& default_value ) const noexcept
    {
        if(this->empty())
        {
            return default_value;
        }
        else
        {
            return transformer(get_reference(this->size() - 1));
        }
    }

    [[nodiscard]]
    //[[safe]]
    constexpr std::optional<value_type> get_back_optional() const noexcept
    {
        if(this->empty())
        {
            return std::nullopt;
        }
        else
        {
            return std::optional<value_type>{get_reference(this->size() - 1)};
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_back_optional( const F& transformer ) const noexcept
    {
        if(this->empty())
        {
            return std::optional<decltype(transformer(get_reference(0)))>{};
        }
        else
        {
            return std::optional<decltype(transformer(get_reference(0)))>{transformer(get_reference(this->size() - 1))};
        }
    }

    [[nodiscard]]
    //[[safe]]
    constexpr value_type get_back_or_terminate( const std::source_location location = std::source_location::current() ) const noexcept
    {
        if(this->empty())
        {
            log(0, location);
            std::terminate();
        }
        else
        {
            return get_reference(this->size() - 1);
        }
    }

    template<class F>
    [[nodiscard]]
    //[[safe]]
    constexpr auto transform_back_or_terminate( const F& transformer, const std::source_location location = std::source_location::current() ) const noexcept
    {
        if(this->empty())
        {
            log(0, location);
            std::terminate();
        }
        else
        {
            return transformer(get_reference(this->size() - 1));
        }
    }

    //[[safe]]
    constexpr void set_back_value( const_reference value, const std::source_location location = std::source_location::current() )
    {
        if(this->empty())
        {
            throw std::out_of_range(to_string(0, location));
        }
        else
        {
            get_reference(this->size() - 1) = value;
        }
    }

    template<class F>
    //[[safe]]
    constexpr void visit_back_value( const F& copier, const std::source_location location = std::source_location::current() )
    {
        if(this->empty())
        {
            throw std::out_of_range(to_string(0, location));
        }
        else
        {
            copier(get_reference(this->size() - 1));
        }
    }

    //[[safe]]
    constexpr void set_back_or_terminate( const_reference value, const std::source_location location = std::source_location::current() ) noexcept
    {
        if(this->empty())
        {
            log(0, location);
            std::terminate();
        }
        else
        {
            get_reference(this->size() - 1) = value;
        }
    }

    template<class F>
    //[[safe]]
    constexpr void visit_back_or_terminate( const F& copier, const std::source_location location = std::source_location::current() ) noexcept
    {
        if(this->empty())
        {
            log(0, location);
            std::terminate();
        }
        else
        {
            copier(get_reference(this->size() - 1));
        }
    }

    // https://en.wikipedia.org/wiki/Cropping_(image)
    //[[safe]]
    constexpr void set_back_and_crop( const_reference value ) noexcept
    {
        if(!this->empty())
        {
            get_reference(this->size() - 1) = value;
        }
    }

    template<class F>
    constexpr void visit_back_and_crop( const F& copier ) noexcept
    {
        if(!this->empty())
        {
            copier(get_reference(this->size() - 1));
        }
    }

    //[[safe]]
    constexpr void set_back_and_grow( const_reference value )
    {
        this->resize(std::max(this->size(), 1ul));
        get_reference(this->size() - 1) = value;
    }

    template<class F>
    //[[safe]]
    constexpr void visit_back_and_grow( const F& copier )
    {
        this->resize(std::max(this->size(), 1ul));
        copier(get_reference(this->size() - 1));
    }

    //[[safe]]
    constexpr void set_back_and_grow( const_reference value, const_reference default_value )
    {
        this->resize(std::max(this->size(), 1ul), default_value);
        get_reference(this->size() - 1) = value;
    }

    template<class F>
    //[[safe]]
    constexpr void visit_back_and_grow( const F& copier, const_reference default_value )
    {
        this->resize(std::max(this->size(), 1ul), default_value);
        copier(get_reference(this->size() - 1));
    }
};

template<class T>
constexpr void copy_and_crop( std::span<T> destination, std::size_t pos, std::span<const T> source ) noexcept
{
    std::size_t dsize = destination.size();
    if(pos < dsize)
    {
        std::size_t length = std::min(dsize - pos, source.size());
        std::copy_n(source.begin(), length, destination.begin() + pos);
    }
}

template<class T>
constexpr void copy_and_grow( std::vector<T>& destination, std::size_t pos, std::span<const T> source ) noexcept
{
    std::size_t ssize = source.size();
    destination.resize(std::max(destination.size(), pos + ssize));
    std::copy_n(source.begin(), ssize, destination.begin() + pos);
}

// gcc, clang: -std=c++2b -O3
// msvc:       /std:c++20
// shadowing to safety
// shadowing enhances safety
int main()
{
    my_vector<int> myints{1};
    bool t = myints.test(0);
    t = myints.test(1);
    // get whole
    int pv = myints.get_value( 0 );//throws
    pv = myints.get_value( 0, 42 );//default value
    std::optional<int> opv = myints.get_optional( 0 );
    pv = myints.get_or_terminate( 0 );

    pv = myints.get_front_value();//throws
    pv = myints.get_front_value( 42 );//default value
    opv = myints.get_front_optional();
    pv = myints.get_front_or_terminate();

    pv = myints.get_back_value();//throws
    pv = myints.get_back_value( 42 );//default value
    opv = myints.get_back_optional();
    pv = myints.get_back_or_terminate();
    // set whole
    myints.set_value( 0, 42 );//throws
    myints.set_or_terminate( 0, 42 );
    myints.set_and_crop( 0, 42 );
    myints.set_and_grow( 1, 42 );
    myints.set_and_grow( 3, 42, 0 );

    myints.set_front_value( 42 );//throws
    myints.set_front_or_terminate( 42 );
    myints.set_front_and_crop( 42 );
    myints.set_front_and_grow( 42 );
    myints.set_front_and_grow( 42, 0 );

    myints.set_back_value( 42 );//throws
    myints.set_back_or_terminate( 42 );
    myints.set_back_and_crop( 42 );
    myints.set_back_and_grow( 42 );
    myints.set_back_and_grow( 42, 0 );

    my_vector<std::string> mystrings{"1"};
    bool ts = mystrings.test(0);
    ts = mystrings.test(1);

    auto s42 = "42"s;
    // get/transform whole or part
    size_t pvi = mystrings.transform_value( 0, [](const std::string& item){return item.size();} );
    pvi = mystrings.transform_value( 0, [](const std::string& item){return item.size();}, 42ul );
    std::optional<size_t> opvs = mystrings.transform_optional( 0, [](const std::string& item){return item.size();} );
    pvi = mystrings.transform_or_terminate( 0, [](const std::string& item){return item.size();} );

    pvi = mystrings.transform_front_value( [](const std::string& item){return item.size();} );
    pvi = mystrings.transform_front_value( [](const std::string& item){return item.size();}, 42ul );
    opvs = mystrings.transform_front_optional( [](const std::string& item){return item.size();} );
    pvi = mystrings.transform_front_or_terminate( [](const std::string& item){return item.size();} );

    pvi = mystrings.transform_back_value( [](const std::string& item){return item.size();} );
    pvi = mystrings.transform_back_value( [](const std::string& item){return item.size();}, 42ul );
    opvs = mystrings.transform_back_optional( [](const std::string& item){return item.size();} );
    pvi = mystrings.transform_back_or_terminate( [](const std::string& item){return item.size();} );
    // set/visit whole or part
    mystrings.visit_value( 0, [&s42](std::string& item){item = s42;} );
    mystrings.visit_or_terminate( 0, [&s42](std::string& item){item = s42;} );
    mystrings.visit_and_crop( 0, [&s42](std::string& item){item = s42;} );
    mystrings.visit_and_grow( 1, [&s42](std::string& item){item = s42;} );
    mystrings.visit_and_grow( 3, [&s42](std::string& item){item = s42;}, "0" );

    mystrings.visit_front_value( [&s42](std::string& item){item = s42;} );
    mystrings.visit_front_or_terminate( [&s42](std::string& item){item = s42;} );
    mystrings.visit_front_and_crop( [&s42](std::string& item){item = s42;} );
    mystrings.visit_front_and_grow( [&s42](std::string& item){item = s42;} );
    mystrings.visit_front_and_grow( [&s42](std::string& item){item = s42;}, "0" );

    mystrings.visit_back_value( [&s42](std::string& item){item = s42;} );
    mystrings.visit_back_or_terminate( [&s42](std::string& item){item = s42;} );
    mystrings.visit_back_and_crop( [&s42](std::string& item){item = s42;} );
    mystrings.visit_back_and_grow( [&s42](std::string& item){item = s42;} );
    mystrings.visit_back_and_grow( [&s42](std::string& item){item = s42;}, "0" );

    std::vector<std::string> destination{"1", "2", "3"};
    /*const*/ std::vector<std::string> source1{"3", "4", "5"};
    /*const*/ std::vector<std::string> source2{"6", "7", "8"};

    copy_and_crop(std::span<std::string>{destination}, 1, std::span<const std::string>{source1});// 1, 3, 4
    copy_and_grow(destination, 2, std::span<const std::string>{source2});// 1, 3, 6, 7, 8
    //static_assert( destination[0] == "1"s );
    //static_assert( destination[1] == "3"s );
    //static_assert( destination[2] == "6"s );
    //static_assert( destination[3] == "7"s );
    //static_assert( destination[4] == "8"s );
    assert( destination.size() == 5 );
    assert( destination[0] == "1"s );
    assert( destination[1] == "3"s );
    assert( destination[2] == "6"s );
    assert( destination[3] == "7"s );
    assert( destination[4] == "8"s );
    return 0;
}
```

## Resolution

How effective is the safety proposed? Let's look at the *[exponential]* safety scale [^cdchoc].


![Hierarchy of Controls](https://upload.wikimedia.org/wikipedia/commons/3/36/NIOSH%E2%80%99s_%E2%80%9CHierarchy_of_Controls_infographic%E2%80%9D_as_SVG.svg)

While this proposal does greatly increase safety by increasing the number of safe element access functions from 0 to 56, based on this scale, the effectiveness is limited because it is at best `PPE`, personal protective equipment that only works if used. While **NOT** a part of the proposal, throughout this proposal you may have noticed the following description attributes.

```cpp
[[unsafe(reason=["range", "dangling_reference", "reference_invalidation"])]]
[[unsafe(reason=["dangling_reference", "reference_invalidation"])]]
[[safe]]
```

Armed with this type of documentation, a tool could be provided that outputs a JSON/YAML statistical summary report representing an audit of the safety of the code. This report could be checked in with the source code so that a automated code reviewer could reject code submission if the patch would render the code base less safe or proportionally so. Further, those code who can't have any unsafety, such as new code, would benefit from static analysis, preferably built into the compiler, that would reject any deprecated and documented unsafe code especially when a substitute has been provided. These are high level tools used by owners, architects, managers, team leads and auditors on the project/module level.

## Summary

The advantages of adopting said proposal are as follows:

1. Reduces range based access errors
1. Reduces dangling references at run time
1. Reduces reference invalidation errors at run time

<!--
## Frequently Asked Questions

### Why not ...?
-->
## References

<!--Hierarchy of Controls-->
[^cdchoc]: <https://www.cdc.gov/niosh/topics/hierarchy/default.html>
<!--Hierarchy of hazard controls-->
[^wikihohc]: <https://en.wikipedia.org/wiki/Hierarchy_of_hazard_controls>
<!--Hierarchy of hazard controls-->
[^wikihohclicense]: <https://en.wikipedia.org/wiki/File:NIOSH%E2%80%99s_%E2%80%9CHierarchy_of_Controls_infographic%E2%80%9D_as_SVG.svg>
<!--Hierarchy of hazard controls-->
[^wikihohclicenseimage]: <https://upload.wikimedia.org/wikipedia/commons/3/36/NIOSH%E2%80%99s_%E2%80%9CHierarchy_of_Controls_infographic%E2%80%9D_as_SVG.svg>
<!--A pipeline-rewrite operator-->
[^p2011r1]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2011r1.html>
<!--Exploring the Design Space for a Pipeline Operator-->
[^p2672r0]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2672r0.html>
<!--Deducing this-->
[^p0847r7]: <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0847r7.html>
<!---->
<!--
[^]: <>
-->
