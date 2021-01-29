#pragma once
#include <utility>

/**
 * Class for C++ property that helps not to write 
 * tons of code for every class variable
 */
template<class T>
class property {
protected:
    T m_value; /**< Value of a property */
public:
    using value_type = T; /**< define for other classes to use */

    /**
     * Default constructor, does nothing special
     */
    property() {}

    /**
     * Args forward constructor \n
     * Allows multiple arguments to be passed into property's value.
     * @param args arguments to be passed to value's constructor
     */
    template<class... Args>
    property(Args... args)
        : m_value(args...) {}

    /**
     * Copy/move constructor \n
     * Constructor that executes both when property is copied or moved.
     * Works with "perfect forwarding" technique.
     * @param rhs property on the rhs that will be copied or moved into property's value
     */
    template<class Arg>
    property(Arg&& rhs)
        : m_value(std::forward<Arg>(rhs)) {}

    /**
     * Call operator, returns reference to value \n
     * @return value's reference
     */
    auto& operator()() { return m_value; }
    /**
     * Const call operator, returns const reference to value \n
     * @return value's const reference
     */
    auto& operator()() const { return m_value; }
    /**
     * Pointer operator, returns value's call operator result reference \n
     * @return reference to value's call operator
     */
    auto& operator->() { return m_value(); }
    /**
     * Const pointer operator, returns value's call operator result reference \n
     * @return const reference to value's call operator
     */
    auto& operator->() const { return m_value(); }
    /**
     * Setter, copies rhs into the value \n
     * @param rhs value to copy
     * @return reference to new value
     */
    auto& set(const T& rhs) { return m_value = rhs; }
    /**
     * Setter, moves rhs into the value \n
     * @param rhs value to move
     * @return reference to new value
     */
    auto& set(T&& rhs) { return m_value = std::move(rhs); }
    /**
     * Getter, returns value's reference \n
     * @return value's reference
     */
    auto& get() { return m_value; }
    /**
     * Const getter, returns const value's reference \n
     * @return const value's reference
     */
    auto& get() const { return m_value; }

    /**
     * Conversion operator \n
     * @return value
     */
    operator T() const { return m_value; }
    /**
     * Assignment move operator \n
     * @param rhs rhs to move into the property's value
     * @return reverence to new value
     */
    template<class Arg>
    auto& operator=(Arg&& rhs) { return m_value = std::forward<Arg>(rhs); }
    //template<class Arg> auto& operator =  (Arg&& rhs) { return m_value  = std::forward<Arg>(rhs); }

    /**
     * Comparison operator.
     * @param rhs right parameter to compare. 
     * @param lhs left parameter to compare. 
     * @returns true if parameters are equal, false of not.
     * */
    friend bool operator==(const property<T>& lhs, const property<T>& rhs) { return lhs() == rhs(); }
};
