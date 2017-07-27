###############
Getting started
###############

Karn is a self instructional / training project aiming to provide practical
implementations of well known data structures such as the ones found onto
Wikipedia's `List of data structures page
<https://en.wikipedia.org/wiki/List_of_data_structures>`_.

Frustration each time a suitable data structure must be selected within a
particular applicative context because of multiple reasons :

* no time to perform comparative study of available candidates,
* confusion as to theoretical time complexity vs measured efficiency,
* many unqualified implementation in the open source jungle,
* lack of testing coverage 
* ...
  
Despite the abundant literature, always facing uncertainties with respect to
selection criteria and trade offs. Real need for a rational and systematic
approach with measurable results.

Something like this `Big-O Cheat Sheet Poster
<http://bigocheatsheet.com/img/big-o-cheat-sheet-poster.png>`_ with additional
criteria such as:

* merging
* allocation strategy (stack vs heap, caller vs in-structure alloc)
* cache / memory hierarchies consciousness 
* data sets properties
* ...

Basically, Karn's ambition is to provide a data structures toolbox combining
theoretical bounds understanding and experimental validation.

====================
Distinctive features
====================

Design to run onto :

* GNU platforms
* PC based system
* hopefully on embedded targets with "reasonable" constraints in the futur
* for 32 bits and 64 bits CPU architecture.

*Data structures* :

* :ref:`dstruct-slist`
* :ref:`dstruct-bheap_fixed`

*Sorting* :

* :ref:`sort-bubble`
* :ref:`sort-select`
* :ref:`sort-insert`
* :ref:`sort-hybrid_merge`

Karn's library *build configuration* allows to enable / disable individual
features at compile time, which makes it runnable onto constrained targets.

Karn comes with numerous *functional unit tests*.

Finally, *performance assessment tools* hopefully allow users to select data
structures / algorithms matching their applicative requirements.

=====================
API design rationales
=====================

Generally speaking, exposed APIs implement *no arguments validation* as data
structures are meant to be embedded within applicative intermediate layers.
Trade offs:

* prevent from code duplication (better performance and ease of maintenance) ;
* applicative / upper layers need some additional checks anyway ;
* make them suitable for constrained embedded environments ;
* at the cost of reduced robustness, debuggability and ease of integration.

To mitigate the latter 2 points, all APIs are instrumented with abundant
assertions allowing to prevent from users programmatic errors. This also
provides them with the ability to quickly identify error root causes.

As to robustness, users will have to carefully design their applicative state
machines and implement proper arguments checking on their own. Choice is
subjective... agreed !

=========
Licensing
=========

Available under GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version. For more information, see http://www.gnu.org/licenses.
