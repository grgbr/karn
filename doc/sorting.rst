*******
Sorting
*******

==================
Singly linked list
==================

4 available flavours:

* :ref:`sort-bubble`,
* :ref:`sort-select`,
* :ref:`sort-insert`,
* :ref:`sort-hybrid_merge`.

For linked lists note that :

* the relative cost of item swapping vs comparison is much smaller than for
  arrays,
* most of the time, much smaller number of item swaps needed than for arrays
  (depends on algorithm however),
* at the cost of lower cache locality.

.. table:: Sorting algorithm properties

    +------------------+-----------------------------------------------------------------------+
    | Property         | Algorithm                                                             |
    |                  +--------------------+----------------+----------------+----------------+
    |                  | Hybrid merge       | Insertion      | Selection      | Bubble         |
    +==================+====================+================+================+================+
    | :term:`Adaptive` | sort of            | yes            | no             | yes            |
    +------------------+--------------------+----------------+----------------+----------------+
    | :term:`Online`   | no                 | yes            | no             | no             |
    +------------------+--------------------+----------------+----------------+----------------+
    | :term:`Stable`   | yes                | yes            | yes            | yes            |
    +-------+----------+--------------------+----------------+----------------+----------------+
    | Time  | worst    | :math:`O(nlog(n))` | :math:`O(n^2)` | :math:`O(n^2)` | :math:`O(n^2)` |
    |       +----------+--------------------+----------------+----------------+----------------+
    |       | average  | :math:`O(nlog(n))` | :math:`O(n^2)` | :math:`O(n^2)` | :math:`O(n^2)` |
    |       +----------+--------------------+----------------+----------------+----------------+
    |       | best     | :math:`O(n)`       | :math:`O(n)`   | :math:`O(n^2)` | :math:`O(n)`   |
    +-------+----------+--------------------+----------------+----------------+----------------+
    | Space | worst    | 27 slists          | :math:`O(1)`   | :math:`O(1)`   | :math:`O(1)`   |
    |       |          | (54 words)         |                |                |                |
    |       +----------+--------------------+----------------+----------------+----------------+
    |       | average  | :math:`O(log(n))`  | :math:`O(1)`   | :math:`O(1)`   | :math:`O(1)`   |
    |       +----------+--------------------+----------------+----------------+----------------+
    |       | best     | :math:`O(1)`       | :math:`O(1)`   | :math:`O(1)`   | :math:`O(1)`   |
    +-------+----------+--------------------+----------------+----------------+----------------+
    | :term:`In-place` | no                 | yes            | yes            | yes            |
    +------------------+--------------------+----------------+----------------+----------------+
    | Allocation       | on stack           | none           | none           | none           |
    +------------------+--------------------+----------------+----------------+----------------+
    | Recursive        | no                 | no             | no             | no             |
    +------------------+--------------------+----------------+----------------+----------------+

.. _sort-bubble:

Bubble sort
===========

.. sidebar:: Quick links

    :c:func:`slist_bubble_sort()`

Sort scheme based upon traditional algorithm depicted onto `Wikipedia's
bubble sort page <https://en.wikipedia.org/wiki/Bubble_sort>`_.

Algorithm Outline :

#. iterate over unsorted list untill an out of order item is found,
#. swap it with its adjacent predecessor,
#. loop back to step 1 till the end of (partially) unsorted list,
#. keep looping back to step 1 as long as swaps are needed.

Noteworthy points:

* most complex implementation of all algorithm mentioned here,
* high number of items swaps,
* decent efficiency with presorted data sets,
* poor to extremely low efficiency even over small data sets.

Implemented for reference only : **do not use** it.

.. _sort-select:

Selection sort
==============

.. sidebar:: Quick links

    :c:func:`slist_selection_sort()`

Sort scheme based upon traditional algorithm depicted onto `Wikipedia's
selection sort page <https://en.wikipedia.org/wiki/Selection_sort>`_.

Algorithm Outline :

#. setup 2 sublists, one empty for future sorted items, the other for unsorted
   ones,
#. iterate over whole unsorted sublist to locate next in order item,
#. insert it at tail of sorted sublist,
#. loop back to step 2 untill all unsorted list items are consumed.

Noteworthy points:

* simple implementation although a bit more complex than insertion sort,
* worst time efficiency of all algorithm,
* deterministic.

Only there for reference purpose : **don't use** it.

.. _sort-insert:

Insertion sort
==============

.. sidebar:: Quick links

    :c:func:`slist_insertion_sort()`
    :c:func:`slist_counted_insertion_sort()`

Sort scheme based upon traditional algorithm depicted onto `Wikipedia's
insertion sort page <https://en.wikipedia.org/wiki/Insertion_sort>`_.

Algorithm Outline :

#. setup 2 sublists, one empty for future sorted items, the other for unsorted
   ones,
#. iterate over unsorted sublist untill an unsorted item is found,
#. insert it in order into sorted sublist,
#. loop back to step 2 untill all unsorted list items are consumed.

Noteworthy points :

* simple implementation,
* limited number of items swaps,
* very efficient on small and presorted data sets,
* poor efficiency over large data sets.

Which makes it the best choice (even far better than merge sort) for slightly
to mostly presorted data sets. It suffers from a *pathological* worst case
behavior when first unsorted item :

* must land at last position after sorting completion and,
* is followed by items strictly sorted in order.

The hybrid merge sort scheme tries to benefit from the very good insertion sort
efficiency while mitigating its worst case behavior by taking advantage of the
divide and conquer strategy.

.. _sort-hybrid_merge:

Hybrid merge sort
=================

.. sidebar:: Quick links

    :c:func:`slist_merge_sort()`
    :c:func:`slist_hybrid_merge_sort()`
    :c:func:`slist_merge_presort()`

Merge sorting implementation is based on an original idea attributed to
Jon McCarthy and described
`here <http://richardhartersworld.com/cri/2007/schoen.html>`_.

The whole point is : avoid excessive scanning of the list during sublist
splitting phases by using an additional bounded auxiliary space to store
sublist heads.
The whole process is performed in an iterative manner.

Hybrid algorithm will start using the merge sort's divide and conquer
strategy only when number of keys to be sorted is greater than a calculated
threshold. It will rely upon insertion sorting otherwise.

This threshold is automatically computed according to an heuristic implemented
in :c:func:`slist_merge_sort()` . Tradeoffs:

* the lower the value, the better the worst case in terms of computational
  complexity and the larger the used stack space ;
* the greater the value, the better the best case in terms of computational
  complexity and the lower the used stack space.

====================
Algorithm properties
====================

.. glossary::

    Adaptive
      speeds up to :math:`O(n)` when data is nearly sorted
     
    Online
      can sort a list as it receives it, i.e. on-the-fly
      
    In-place
      only requires a constant amount :math:`O(1)` of additional memory space
    
    Stable
      does not change the relative order of elements with equal keys
