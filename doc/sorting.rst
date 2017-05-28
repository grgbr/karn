*******
Sorting
*******

==================
Singly linked list
==================

3 available flavours:

- :c:func:`slist_bubble_sort()` implements bubble sort,
- :c:func:`slist_selection_sort()` implements selection sort,
- :c:func:`slist_insertion_sort()` implements insertion sort,
- :c:func:`slist_merge_sort()` and :c:func:`slist_hybrid_merge_sort()` implement
  an hybridization between merge and insertion sort.

.. table:: Sorting algorithm properties

    +-----------------+-----------------------------------------------------------------------+
    | Property        | Algorithm                                                             |
    |                 +--------------------+----------------+----------------+----------------+
    |                 | Hybrid merge       | Insertion      | Selection      | Bubble         |
    +=================+====================+================+================+================+
    | Stable          | yes                | yes            | yes            | yes            |
    +-------+---------+--------------------+----------------+----------------+----------------+
    | Time  | worst   | :math:`O(nlog(n))` | :math:`O(n^2)` | :math:`O(n^2)` | :math:`O(n^2)` |
    |       +---------+--------------------+----------------+----------------+----------------+
    |       | average | :math:`O(nlog(n))` | :math:`O(n^2)` | :math:`O(n^2)` | :math:`O(n^2)` |
    |       +---------+--------------------+----------------+----------------+----------------+
    |       | best    | :math:`O(n)`       | :math:`O(n)`   | :math:`O(n^2)` | :math:`O(n)`   |
    +-------+---------+--------------------+----------------+----------------+----------------+
    | Space | worst   | 27 slists          | :math:`O(1)`   | :math:`O(1)`   | :math:`O(1)`   |
    |       |         | (54 words)         |                |                |                |
    |       +---------+--------------------+----------------+----------------+----------------+
    |       | average | :math:`O(log(n))`  | :math:`O(1)`   | :math:`O(1)`   | :math:`O(1)`   |
    |       +---------+--------------------+----------------+----------------+----------------+
    |       | best    | :math:`O(1)`       | :math:`O(1)`   | :math:`O(1)`   | :math:`O(1)`   |
    +-------+---------+--------------------+----------------+----------------+----------------+
    | In-place        | no                 | yes            | yes            | yes            |
    +-----------------+--------------------+----------------+----------------+----------------+
    | Allocation      | on stack           | none           | none           | none           |
    +-----------------+--------------------+----------------+----------------+----------------+
    | Recursive       | no                 | no             | no             | no             |
    +-----------------+--------------------+----------------+----------------+----------------+

.. _sort-hybrid_merge:

Hybrid merge sort
=================

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

- the lower the value, the better the worst case in terms of computational
  complexity and the larger the used stack space ;
- the greater the value, the better the best case in terms of computational
  complexity and the lower the used stack space.

.. _sort-insert:

Insertion sort
==============

.. todo:: complete me

.. _sort-bubble:

Bubble sort
==============

.. todo:: complete me

.. _sort-select:

Selection sort
==============

.. todo:: complete me
