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

- the lower the value, the better the worst case in terms of computational
  complexity and the larger the used stack space ;
- the greater the value, the better the best case in terms of computational
  complexity and the lower the used stack space.

.. _sort-insert:

Insertion sort
==============

.. sidebar:: Quick links

    :c:func:`silst_insertion_sort()`
    :c:func:`silst_counted_insertion_sort()`

Sort scheme based upon traditional algorithm depicted onto `Wikipedia's
insertion sort page <https://en.wikipedia.org/wiki/Insertion_sort>`_.

Noteworthy points:

- simple implementation
- very low swap (vs compare) cost for linked lists.

Which makes it the best choice (even far better than merge sort) for slightly
to mostly presorted data sets. It suffers from a *pathological* worst case
behavior when first unsorted item :

- should land at last position after sorting completion and,
- is followed by items strictly sorted in order.

The hybrid merge sort scheme tries to benefit from the very good insertion sort
efficiency while mitigating its worst case behavior by taking advantage of the
divide and conquer strategy.

.. _sort-bubble:

Bubble sort
==============

.. sidebar:: Quick links

    :c:func:`silst_bubble_sort()`

Sort scheme based upon traditional algorithm depicted onto `Wikipedia's
bubble sort page <https://en.wikipedia.org/wiki/Bubble_sort>`_.

Noteworthy points:

- most complex implementation of all algorithm mentionned here,

.. _sort-select:

Selection sort
==============

.. sidebar:: Quick links

    :c:func:`silst_selection_sort()`

Sort scheme based upon traditional algorithm depicted onto `Wikipedia's
selection sort page <https://en.wikipedia.org/wiki/Selection_sort>`_.

Noteworthy points:

- simple implementation although a bit more complex than insertion sort,
- worst time efficiency of all algorithm,
- deterministic.

Only there for reference purpose : **don't use it**.
