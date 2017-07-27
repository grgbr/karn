***************
Data structures
***************

.. _dstruct-slist:

==================
Singly linked list
==================

.. sidebar:: Quick links

    :ref:`API <api-slist>`

:c:type:`slist` is a simple singly linked list allowing to link together
arbitrary sequences of :c:type:`slist_node` nodes. Iteration is unidirectional
from head (start) to tail (end).

:c:type:`slist_node` nodes are meant to be embedded into user structures
allowing to implement usual list manipulation primitives in a generic and
reusable way.

:c:type:`slist` is NULL terminated. It contains a head node and an additional
tail pointer allowing to quickly jump to the end of list. The former head node
allows to save an extra pointer comparison when enqueuing a node (to tail) or
joining (sub)lists.

===========
Binary Heap
===========

A binary heap is a binary tree with 2 additional constraints:

*shape property*
    All levels of the tree are fully filled except the last one (deepest) where
    nodes are filled from left to right.

*heap property*
    The key stored in each node is either greater than or equal
    to (max-heap), or less than or equal to (min-heap) the keys in the node's
    children.

Typical operations:

- insert element
- extract smallest (min-heap) or largest (max-heap) element
- find smallest or largest element
- decrease
- merge

Common usage:

- priority queues,
- (heap)sort.

.. _dstruct-bheap_fixed:

Fixed length array based binary heap
====================================

.. sidebar:: Quick links

    :ref:`API <api-bheap_fixed>`

*Underlying structure* is a Fixed length array based binary search tree
where each key is located into a traditional C array slot.

2 *Heap and keys instantiation* alternatives:

* delegate underlying keys array preallocation to user,
* or allocate both heap and underlying keys array in a single block of memory.

*Insertion* and *extraction* operations by *copy*.

*Peeking* operation by *reference*.

*Key comparison* implementation delegated to user.
