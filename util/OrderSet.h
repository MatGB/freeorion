#ifndef _OrderSet_h_
#define _OrderSet_h_


#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "Export.h"

#include <map>
#include <memory>
#include <vector>


class Order;

/** The pointer type used to store Orders in OrderSets. */
typedef std::shared_ptr<Order> OrderPtr;

/** a collection of orders that may be searched using arbitrary predicate functions and functors*/
class FO_COMMON_API OrderSet {
private:
    typedef std::map<int, OrderPtr> OrderMap;

public:
    typedef OrderMap::const_iterator    const_iterator;    ///< defines a public const_iterator type for OrderSet
    typedef std::vector<OrderPtr>       OrderVec;          ///< the return type of FindOrders()

    /** \name Structors */ //@{
    OrderSet();
    //@}

    /** \name Accessors */ //@{
    const OrderPtr ExamineOrder(int order) const;       ///< returns a pointer to any order, so that it can be examined through its accessors; returns 0 if no such order exists

    /** returns all the orders that match \a pred.  Predicates used with this function must take a single OrderPtr 
        parameter and must return bool or a type for which there is a conversion to bool.*/
    template <class Pred>
    OrderVec FindOrders(Pred pred) const
    {
        OrderVec retval;
        for (const OrderMap::value_type& order : m_orders) {
            auto &o = order.second;
            if (pred(o))
                retval.push_back(o);
        }
        return retval;
    }

    const_iterator begin() const {return m_orders.begin();} ///< returns the begin const_iterator for the OrderSet
    const_iterator end() const   {return m_orders.end();}   ///< returns the end const_iterator for the OrderSet
    //@}

    /** \name Mutators */ //@{
    /** If \p suppress_immediate_execution is false, then execute the \p order immediately on the client.
        Always store the \p order in the OrderSet to be executed later on the server.
        Return an index that can be used to reference the order. */
    int            IssueOrder(const OrderPtr& order, bool suppress_immediate_execution = false);
    int            IssueOrder(OrderPtr&& order, bool suppress_immediate_execution = false);

    /** Applies all Orders in the OrderSet.  As of this writing, this is needed only after deserializing an OrderSet
        client-side during game loading. */
    void           ApplyOrders();

    bool           RescindOrder(int order);    ///< removes the order from the OrderSet; returns true on success, false if there was no such order or the order is non-rescindable
    void           Reset(); ///< clears all orders; should be called at the beginning of a new turn
    //@}

private:
    OrderMap m_orders;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// Template Implementations
template <class Archive>
void OrderSet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_orders);
}

#endif // _OrderSet_h_
