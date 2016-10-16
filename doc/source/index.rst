-------------------------------------------------------------------------------
              Documentation for bee-c
-------------------------------------------------------------------------------

.. image:: https://travis-ci.org/bee/bee-c.svg?branch=master
    :target: https://travis-ci.org/bee/bee-c

===========================================================
                        About
===========================================================

``bee-c`` is a client library that implements a C connector for Bee
(see https://github.com/wprayudo/bee).

The ``bee-c`` library depends on the ``msgpuck`` library
(see https://github.com/wprayudo/msgpuck).

The ``bee-c`` library consists of two parts:

  * ``beer``,
    an `IProto <http://bee.org/doc/dev_guide/box-protocol.html>`_/networking
    library
  * ``beerrpl``,
    a library for working with snapshots, xlogs and a replication client
    (this library is not ported to Bee 1.6 yet, so it isn't covered in
    this documentation set)

===========================================================
                 Compilation/Installation
===========================================================

You can clone source code from the "bee-c" repository at GitHub
and use :program:`cmake`/:program:`make` to compile and install
the ``bee-c`` library:

.. code-block:: console

    $ git clone git://github.com/wprayudo/bee-c.git ~/bee-c --recursive
    $ cd ~/bee-c
    $ cmake .
    $ make

    #### For testing against installed bee:
    $ make test

    #### For installing into system (headers+libraries):
    $ make install

    #### For building documentation using Sphinx:
    $ cd doc
    $ make sphinx-html


===========================================================
                        Contents
===========================================================

.. toctree::
   :maxdepth: 2

   connection.rst
   msgpackobject.rst
   reply.rst
   request.rst
   request_builder.rst
   schema.rst
   buffering.rst
   stream.rst

===========================================================
                         Index
===========================================================

* :ref:`genindex`
