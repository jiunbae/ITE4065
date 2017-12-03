/*****************************************************************************

Copyright (c) 2017, 

*****************************************************************************/

/******************************************************************//**
@file include/imp0lflst.h
Improved utilites Lock Free Linked List

Created 2/12/2017 Jiun Bae
***********************************************************************/

#ifndef imp0lflst_h
#define imp0lflst_h

#include <mtr0types.h>
#include <ut0rbt.h>
#include <ut0lst.h>

/* This module implements the lock free two-way linear list. */

/*******************************************************************//**
The two way list node.
@param TYPE the list node type name */
template <typename Type>
struct lf_list_node : public ut_list_node<Type> {
    Type*       prev;           /*!< pointer to the previous
                        node, NULL if start of list */
    Type*       next;           /*!< pointer to next node,
                        NULL if end of list */

    void reverse()
    {
        Type* tmp = prev;
        prev = next;
        next = tmp;
    }
};

/** Macro used for legacy reasons */
#define LF_LIST_NODE_T(t)       lf_list_node<t>

/*******************************************************************//**
The two-way list base node. The base node contains pointers to both ends
of the list and a count of nodes in the list (excluding the base node
from the count). We also store a pointer to the member field so that it
doesn't have to be specified when doing list operations.
@param Type the type of the list element
@param NodePtr field member pointer that points to the list node */
template <typename Type, typename NodePtr>
struct lf_list_base {
    typedef Type elem_type;
    typedef NodePtr node_ptr;
    typedef lf_list_node<Type> node_type;

    ulint       count;          /*!< count of nodes in list */
    elem_type*  start;          /*!< pointer to list start,
                                    NULL if empty */
    elem_type*  end;            /*!< pointer to list end,
                                    NULL if empty */
    node_ptr    node;           /*!< Pointer to member field
                                    that is used as a link node */
#ifdef UNIV_DEBUG
    ulint       init;           /*!< UT_LIST_INITIALISED if
                                    the list was initialised with
                                    UT_LIST_INIT() */
#endif /* UNIV_DEBUG */

    void reverse()
    {
        Type*   tmp = start;
        start = end;
        end = tmp;
    }
};


#define LF_LIST_BASE_NODE_T(t)  lf_list_base<t, lf_list_node<t> t::*>

#ifdef UNIV_DEBUG
# define LF_LIST_INITIALISED        0xCAFE
# define LF_LIST_INITIALISE(b)      (b).init = LF_LIST_INITIALISED
# define LF_LIST_IS_INITIALISED(b)  ut_a(((b).init == LF_LIST_INITIALISED))
#else
# define LF_LIST_INITIALISE(b)
# define LF_LIST_IS_INITIALISED(b)
#endif /* UNIV_DEBUG */

/*******************************************************************//**
Note: This is really the lock free list constructor. 
@param b the list base node
@param pmf point to member field that will be used as the link node */
#define LF_LIST_INIT(b, pmf)            \
{                                       \
    (b).count = 0;                      \
    (b).start = 0;                      \
    (b).end   = 0;                      \
    (b).node  = pmf;                    \
    LF_LIST_INITIALISE(b);              \
}

#endif
